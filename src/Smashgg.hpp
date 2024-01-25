#ifndef HEADER_H
#define HEADER_H

#include <vector>
#include <nlohmann/json.hpp>
#include <string>

size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string* data);

std::string makeRequest(const char *query);

std::string getIdFromTournamentLink(std::string link);

std::string getTournamentName(std::string ID);

std::vector<std::string> getSetData(std::string ID);

std::vector<std::string> sortSetData(std::string text);

void printVector(std::vector<std::string> vec);

nlohmann::json trimmer(nlohmann::json js);

std::string getEventName(std::string ID);

std::vector<std::string> splitByspace(std::string IDs);

#endif