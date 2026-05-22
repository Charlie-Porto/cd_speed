#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>

#include "nlohmann/json.hpp"


enum ActionType
{
    CDS
  , ADD
  , UPDATE
  , REMOVE
  , LIST
  , ERROR
};


ActionType actionTypeFromArgs( const std::vector<std::string>& args );

bool parseConfigToJsonObjAndValidate( const std::string& config_file_abs_path
                                    , nlohmann::json& config );

void showConfig( const std::string config_file_abs_path
               , const std::string& user
               , const std::vector<std::string>& kwds={});

std::string cdPathFromDirKwdSequence( const std::vector<std::string>& dir_kwd_seq
                                    , const std::string& config_file_abs_path );
