#include "MainWindow.hpp"
#include <QtWidgets>
#include <obs-module.h>
#include "plugin-support.h"
#include "Smashgg.hpp"
#include <QObject>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <future>
#include <pthread.h>
#include <qtconcurrentrun.h>


using namespace std;

MainWidget::MainWidget(QWidget *parent) : QDockWidget("MonkeyBenQ's SmashGG Plugin", parent)
{
    this->isRecording = false;
    this->tourneyId = "";
    this->eventName = "";
    this->tourneyName = "";
    this->currentMatch;
    //this->outputpath = obs_frontend_get_current_record_output_path();

    this->parent = parent;

    QWidget *widget = new QWidget();

    QVBoxLayout *layout = new QVBoxLayout();
        
    this->label1->setText("");
    //https://www.start.gg/tournament/api-testing-3/event/dev-testing/brackets/1534774/2307767
    this->lineedit->setText("TOURNEY");
    this->submit_button->setText("Submit");
    this->start_button->setText("Press to Start");
    this->eventoptions->setVisible(false);
    this->select_eventid_button->setVisible(false);
    this->select_eventid_button->setText("Select Event");
    this->hideEveything = false;

    //hideHandler(); //HIDE EVERYTHIMNG

    layout->addWidget(this->label1);
    layout->addWidget(this->lineedit);
    layout->addWidget(this->eventoptions);
    layout->addWidget(this->submit_button);
    layout->addWidget(this->start_button);
    layout->addWidget(this->select_eventid_button);


    widget->setLayout(layout);
    
    setWidget(widget);
    
    setVisible(false);
    setFloating(true);
    resize(300,300);

    QObject::connect(this->submit_button, SIGNAL(clicked()), SLOT(actualButtonClicked()));
    QObject::connect(this->start_button, SIGNAL(clicked()), SLOT(startButtonClicked()));
    QObject::connect(this->select_eventid_button, SIGNAL(clicked()), SLOT(selectEventButtonClicked()));

    
}

void MainWidget::printToLog(std::string message)
{

    obs_log(LOG_INFO, message.c_str());
}

void MainWidget::actualButtonClicked()
{

    /*time_t now = time(0); UNLOCKED VERSION

    tm *ltm = localtime(&now);

    printToLog("current month " + to_string(ltm->tm_mon));
    printToLog("current day " + to_string(ltm->tm_mday));

    if (ltm->tm_mon > 2){
        printToLog("locked");
        return;
    }*/


    this->label1->setText(this->lineedit->text());
    printToLog("Text Changed");
    
    printToLog(getTournamentName(this->lineedit->text().toStdString()));

    string text = this->lineedit->text().toStdString();

    if(text.size() > 10){ //MULTIPLE EVENTS IN TEH TOURNAMENT
        printToLog("IDs: " + text);
        string id = getIdFromTournamentLink(text);
        
        if(id.size() > 10){
            printToLog(id);
            vector<string> idsVec = splitByspace(id); //GET ALL EVENT IDS FROM A TOURNAMENT LINK
            id = idsVec[0];

            for(int i =0; i != idsVec.size(); i++) // ADD IDS TO COMBOBOX
            {
                if(idsVec[i] != " ")
                {
                    string eventid = idsVec[i];
                    string eventname = getEventName(eventid);
                    this->eventoptions->addItem(QString::fromStdString(eventid + " | " + eventname)); 
                }
            }
            this->hideEveything = true;
            hideHandler(); // BRINGS UP THE MENU TO SELECT THE EVENT ID
        }

        this->tourneyName = QString::fromStdString(getTournamentName(id));
        this->tourneyId = QString::fromStdString(id);
    }
    else{ // ONLY 1 EVENT IN TOURNAMENT
        this->eventName = QString::fromStdString(getEventName(this->lineedit->text().toStdString()));
        this->tourneyName = QString::fromStdString(getTournamentName(this->lineedit->text().toStdString()));
        this->tourneyId = QString::fromStdString(this->lineedit->text().toStdString());
    }
    MatchHandler();
    updateText();
}

void MainWidget::hideHandler()
{
    if(this->hideEveything == true) {
        this->label1->setVisible(false);
        this->lineedit->setVisible(false);
        this->submit_button->setVisible(false);
        this->start_button->setVisible(false);

        this->eventoptions->setVisible(true);
        this->select_eventid_button->setVisible(true);
    }
    else{
        this->label1->setVisible(true);
        this->lineedit->setVisible(true);
        this->submit_button->setVisible(true);
        this->start_button->setVisible(true);

        this->eventoptions->setVisible(false);
        this->select_eventid_button->setVisible(false);
    }
}

void MainWidget::startButtonClicked()
{
    if(this->isRecording == true)
    {
        this->isRecording = false;
        this->start_button->setText("Press To Start");
    }
    else{
        this->isRecording = true;
        this->start_button->setText("Press to Stop");
        QFuture<void> future = QtConcurrent::run(&MainWidget::mainLoop, this);
    }
    updateText();
}

void MainWidget::selectEventButtonClicked(){
    
    string option = this->eventoptions->currentText().toStdString();
    string id;
    for(int i = 0; option[i] != ' '; i++)
    {
        id.push_back(option[i]);
    }
    printToLog("selected id " + id);

    this->tourneyName = QString::fromStdString(getTournamentName(id));
    this->tourneyId = QString::fromStdString(id);
    this->eventName = QString::fromStdString(getEventName(id));
    this->eventoptions->clear();
    MatchHandler();
    updateText();
    this->hideEveything = false;
    hideHandler();
}

void MainWidget::updateText()
{   
    QString isrectext;

    if(this->isRecording){isrectext = "Recording\n";}
    else{isrectext = "Not Recording\n";}
    
    QString currmatch;

    if(this->currentMatch.size() <= 1){
        currmatch = "No Match";
    }
    else if (this->currentMatch.size() < 4)
    {
        currmatch = QString::fromStdString(this->currentMatch[0]) + " vs " + QString::fromStdString(this->currentMatch[1]);
    }
    

    this->label1->setText(isrectext + "Tourney Name: " + this->tourneyName + "\n" + "Event Name: " + this->eventName + "\n" + "Event ID: " + this->tourneyId + "\n" + "Curent Match: " + currmatch + "\n");
}

void MainWidget::MatchHandler()
{
    vector<string> players = getSetData(this->tourneyId.toStdString());
    this->currentMatch = players;
}

void MainWidget::mainLoop()
{
    bool inprogressrec = false;
    if(this->currentMatch.size() >= 2)
    {
        obs_frontend_recording_start();
        inprogressrec = true;
    }
    this->isRecording = true;
    printToLog("starting loop");
    updateText();

    while(this->isRecording == true)
    {       
        vector<string> parcedMatch = getSetData(this->tourneyId.toStdString());
        obs_log(LOG_INFO, parcedMatch[0].c_str());
        if(parcedMatch != this->currentMatch)
        {
            if(inprogressrec == true) {
                stopRecording(this->currentMatch);
                obs_log(LOG_INFO, "STOP RECORDING");
                inprogressrec = false;
            }
            this->currentMatch = parcedMatch;
            updateText();
            obs_frontend_recording_stop();

            if(this->currentMatch.size() >= 2)
            {
                this_thread::sleep_for(chrono::milliseconds(3000));
                obs_log(LOG_INFO, "START RECORDING");
                inprogressrec = true;
                obs_frontend_recording_start();
            }

        }
        this_thread::sleep_for(chrono::milliseconds(10000));
    }

    printToLog("end loop");
}

void MainWidget::stopRecording(vector<string> outputFileName)
{
    obs_frontend_recording_stop();
    string s;
    if(outputFileName.size() == 2)
    {
        s.append(cleanName(outputFileName[0]));
        s.append(" vs ");
        s.append(cleanName(outputFileName[1]));
    }
    s.append(".mp4");

    string path = obs_frontend_get_last_recording();
    //C:/Users/jacob/Videos/for will/2024-01-19 04-20-32.mp4
    int lastpos;


    for(int i = static_cast<int>(path.size())-1; path.at(i) != '/'; i--)
    {
        lastpos = i;
    }
    string newpath = path.substr(0,lastpos).append(s);

    QFuture<void> future = QtConcurrent::run(&MainWidget::renameFile, this, path, newpath);


    
}
void MainWidget::renameFile(string oldfile, string newfile)
{
    this_thread::sleep_for(chrono::milliseconds(10000));
    

    obs_log(LOG_INFO, "RENAME");
    int i = rename(oldfile.c_str(), newfile.c_str());
    
    printToLog(to_string(i));
    printToLog(oldfile);
    printToLog(newfile);
}

std::string MainWidget::cleanName(std::string name)
{
    string illegalChars = "\\/:?\"<>|";
    string newname;
    for(int i = 0; i != name.size(); i++)
    {
        if(illegalChars.find(name.at(i)) == string::npos)
        {
            newname.push_back(name.at(i));
        }

    }
    return newname;

}

