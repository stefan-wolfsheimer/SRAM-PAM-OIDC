#pragma once
#include <string>
#include <vector>
#include <map>
#include "nlohmann/json.hpp"

std::string url_encode(const std::string & base_url,
                       const nlohmann::json & payload);

std::string url_encode(const std::string & base_url,
                       const std::map<std::string, std::string> & args);

std::map<std::string, std::string> url_parse_query(const std::string & url);

std::string url_request(const std::string & url,
                        const std::string & method,
                        const std::map<std::string, std::string> & headers,
                        const std::string & data);