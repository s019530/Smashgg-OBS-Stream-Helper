#ifndef MainWindow_H
#define MainWindow_H

#include <QtWidgets>
#include <QDockWidget>
#include <qlabel.h>
#include <QLineEdit>
#include <QPushButton>
#include <string>
#include <qcombobox.h>
#include <QComboBox>
#include "obs-frontend-api.h"

class MainWidget : public QDockWidget{
    Q_OBJECT

    public:
        MainWidget(QWidget *parent = nullptr);

    private:
        QWidget *parent = nullptr;
        QLabel *label1 = new QLabel();
        QLineEdit *lineedit = new QLineEdit();
        QPushButton *submit_button = new QPushButton();
        QPushButton *start_button = new QPushButton();
        QPushButton *select_eventid_button = new QPushButton();
        bool isRecording;
        QString tourneyId;
        QString tourneyName;
        QString eventName;
        std::vector<std::string> currentMatch;
        QComboBox *eventoptions = new QComboBox();
        bool hideEveything;
        std::string outputpath;

        void printToLog(std::string message);

        void hideHandler();

        void mainLoop();

        void stopRecording(std::vector<std::string> outputFileName);

        void renameFile(std::string oldfile, std::string newfile);

        std::string cleanName(std::string name);
        
    private slots:
        void actualButtonClicked();
        void startButtonClicked();
        void selectEventButtonClicked();
        void updateText();
        void MatchHandler();
};



#endif