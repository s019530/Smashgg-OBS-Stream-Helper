
#include <iostream>


#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <obs-module.h>

#include "plugin-support.h"
#include "MainWindow.hpp"
#include "Smashgg.hpp"

using namespace std;

size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string* data) {
    data->append((char*) ptr, size * nmemb);
    return size * nmemb;
}

std::string getIdFromTournamentLink(std::string link)
{
    try {
        if(link.at(0) == 'h')
        {
            link = link.substr(32);
        }
        else if(link.at(0) == 's')
        {
            link = link.substr(20);
        }
        else if(link.at(0) == 'w')
        {
            link = link.substr(24);
        }
        else
        {
            return "-1";
        }
        string finalstring;
        for(int i = 0; i != size(link); i++)
        {
            if(link.at(i) == '/'){break;}
            finalstring.push_back(link.at(i));
        }
    
    
        string query = "{\r\n    \"query\":\"query EventStandings{tournament(slug:\\\""+ finalstring + "\\\"){events{id}}}\"\r\n\r\n\r\n}";
        const char *data = query.c_str();
        string response = makeRequest(data);

        obs_log(LOG_INFO, response.c_str());
    
        auto j3 = nlohmann::json::parse(response);
        //cout << query << endl;

        if(j3["data"]["tournament"]["events"].size() == 1)
        {
            return trimmer(j3["data"]["tournament"]["events"])["id"].dump();
        }
        else{
            string returnstring;
            for(int i = 0; i != j3["data"]["tournament"]["events"].size(); i++)
            {   
                string temp = j3["data"]["tournament"]["events"][i]["id"].dump();
                returnstring.append(temp + " ");
            }
            return returnstring;
        }

        //obs_log(LOG_INFO, trimmer(j3["data"]["tournament"]["events"])["id"].dump().c_str());

        return trimmer(j3["data"]["tournament"]["events"])["id"].dump();
    } catch (...) {
        return "-1";
    }
    
}

std::string getTournamentName(std::string ID)
{

    string data2 = "{\r\n    \"query\":\"query EventEntrants{event(id:\\\"" + ID + "\\\"){id name tournament{name}}}\"\r\n\r\n}";

    const char *temp2 = data2.c_str();

    
    //char *data = "{\r\n    \"query\":\"query EventEntrants{event(id:\\\"990600\\\"){id name tournament{name}}}\"\r\n\r\n}";
    
    try {
        std::string temp = makeRequest(temp2);
        auto j3 = nlohmann::json::parse(temp);
    
        temp = j3["data"]["event"]["tournament"]["name"]; 


        return temp;
    } catch (...) {
        return "-1";
    }


}

std::string makeRequest(const char *query)
{
    auto curl = curl_easy_init();
    if (curl) {

        cout << "start req" << endl;
        struct curl_slist *chunk = NULL;

        chunk = curl_slist_append(chunk, "Authorization: Bearer 5b9b8a56ce95de3cfe8409e563237117");
        chunk = curl_slist_append(chunk, "Content-Type: application/json");

        std::string s;
        //char *data = "{\r\n    \"query\":\"query EventStandings {event(id:\\\"990600\\\"){id name sets{nodes{id}}}}\"\r\n}";

        curl_easy_setopt(curl, CURLOPT_URL, "http://api.start.gg/gql/alpha");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, query);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);



        CURLcode res = curl_easy_perform(curl);

        if(res != CURLE_OK)
        {
            std::cout << curl_easy_strerror(res);
            std::cout << "error" << std::endl;
        }

        std::cout << std::endl;

        curl_easy_cleanup(curl);

        curl_slist_free_all(chunk);

        return s;

    }
    
    return "";
}

std::vector<std::string> getSetData(std::string ID){

    vector<string> Names;

    string query = "{\r\n    \"query\":\"query EventEntrants{event(id:\\\""+ID+"\\\"){id name tournament{name streamQueue{stream{streamName}sets{slots{entrant{name}}state}}}}}\"\r\n\r\n\r\n}";
    const char *data = query.c_str();


    try {
        std::string temp = makeRequest(data);
        string forprinting;
        forprinting.append("response: ").append(temp);
        obs_log(LOG_INFO, forprinting.c_str());

        Names = sortSetData(temp);    

        return Names;
    } catch (...) {
        Names.push_back("-1");
        return Names;
    }


} 

std::vector<std::string> sortSetData(std::string text)
{
    
    vector<string> tempvec;

    try {
        string arr[] = {"hello"};
    
        auto js = nlohmann::json::parse(text);

        
        js = js["data"]["event"]["tournament"]["streamQueue"];
        js= trimmer(js);//get sets
        js = js["sets"]; //gets slots

        for(int i = 0; i != size(js); i++)
        {
            //cout << js[i]["state"] << endl;;
            if(js[i]["state"] == 2)
            {

                //std::vector<string> tempvec;
                //js =  js[i]["slots"][0]["entrant"]["name"];
                auto temp =  js[i]["slots"];
                for(int a = 0; a != size(temp); a++)
                {
                    tempvec.push_back(js[i]["slots"][a]["entrant"]["name"]);
                }
                

                return tempvec;
            }
        }
        



    } catch (...) {
    }

    tempvec.push_back("-1");

    return tempvec;

}

nlohmann::json trimmer(nlohmann::json js)
{
    std::string temp = js.dump();
    temp = temp.substr(1,temp.length()-2);


    return nlohmann::json::parse(temp);

}

void printVector(std::vector<std::string> vec)
{
    cout << endl;
    cout << "Printing Players";

    string temp = "Printing Players";

    //obs_log(LOG_INFO, temp.c_str());

    for(int i = 0; i != size(vec); i++)
    {
        obs_log(LOG_INFO, vec[i].c_str());
        cout << vec[i] << endl;
    }
}

std::string getEventName(std::string ID){


    string data = "{\r\n    \"query\":\"query EventStandings{event(id: "+ ID +"){name}}\"\r\n\r\n\r\n}";
    const char *query = data.c_str();

    try {
        std::string temp = makeRequest(query);
        auto j3 = nlohmann::json::parse(temp);


        return  j3["data"]["event"]["name"].dump();
    } catch (...) {
        return "-1";
    }

}

std::vector<std::string> splitByspace(std::string IDs){
    vector<string> result;
    int lastpos = 0;

    for(int i =0; i != IDs.size(); i++)
    {
        if(IDs.at(i) == ' ')
        {
            result.push_back(IDs.substr(lastpos, i -lastpos));
            lastpos = i + 1;
        }
    }
    result.push_back(IDs.substr(lastpos, IDs.size() - lastpos));

    return result;

}