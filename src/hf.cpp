#include "hf.hpp"

#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace std;
using json = nlohmann::json;

constexpr char GROUP_ID_CHAR = '$';

const std::unordered_map<std::string, ActionType> cmd_str_to_enum = {
    { "add",    ADD    },
    { "update", UPDATE },
    { "remove", REMOVE },
    { "list",   LIST   },
};
const std::unordered_map<std::string, ActionType> cmd_abbrv_str_to_enum = {
    { "a", ADD    },
    { "u", UPDATE },
    { "r", REMOVE },
    { "l", LIST   }
};


/////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Private //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

bool parseFileToJsonObj( const std::string& config_file_abs_path
                       , json& jsn )
{
    std::ifstream config_file(config_file_abs_path);
    if (!config_file.is_open())
    {
        std::cout << "Failed to open config file: " << config_file_abs_path << '\n';
        return false;
    }
    jsn = json::parse(config_file);
    return true;
}


void swapSubstringIfExists( string& str
                          , const string& to
                          , const string& from )
{
    const size_t start_pos = str.find(from);
    if(start_pos == string::npos)
        return;
    str.replace(start_pos, from.length(), to);
}


void showPathKwdConfig(json::const_iterator& path_kwd_config, const size_t start_tabs=0)
{
    if (!path_kwd_config.value().contains("dir"))
        return;
    const string dirname = string(path_kwd_config.value().at("dir"));
    string tab="";
    for (size_t i=0; i!= start_tabs; ++i)
        tab += "\t";
    cout << tab << path_kwd_config.key() << "\t" << dirname << '\n';

    if (path_kwd_config.value().contains("children") && !path_kwd_config.value().at("children").empty())
    {
        auto children_config = path_kwd_config.value().at("children");
        for (json::const_iterator it=children_config.begin(); it!=children_config.end(); ++it)
            showPathKwdConfig(it, start_tabs+1);
    }
}

/////////////////////////////////////////////////////////////////////////////
////////////////////////////// END Private //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


ActionType actionTypeFromArgs(const std::vector<std::string>& args)
{
    size_t option_flag_count = 0;
    ActionType action = CDS;
    for (const auto& arg : args)
    {
        const size_t initial_ofc = option_flag_count;
        const auto it_abbrv = cmd_abbrv_str_to_enum.find(arg);
        const auto it_full = cmd_str_to_enum.find(arg);
        if (it_abbrv != cmd_abbrv_str_to_enum.end())
        {
            ++option_flag_count;
            action = it_abbrv->second;
        }
        else if (it_full != cmd_str_to_enum.end())
        {
            ++option_flag_count;
            action = it_full->second;
        }
    }
    if (option_flag_count > 1)
        return ERROR;
    return action;
}


bool parseConfigToJsonObjAndValidate( const std::string& config_file_abs_path
                                    , json& config )
{
    if (!parseFileToJsonObj(config_file_abs_path, config))
    {
        cerr << "ERROR: failed to parse config file; exiting." << '\n';
        return false;
    }
    if (!config.contains("groups"))
    {
        cerr << "ERROR: config file does not contain a 'groups' node." << '\n';
        return false;
    }
    if (!config.contains("paths"))
    {
        cerr << "ERROR: config file does not contain a 'paths' node." << '\n';
        return false;
    }
    return true;
}


void showConfig( const std::string config_file_abs_path
               , const std::string& user
               , const std::vector<std::string>& kwds)
{
    json config;
    if (!parseConfigToJsonObjAndValidate(config_file_abs_path, config))
    {
        cerr << "ERROR: failed to parse and/or validate config file; exiting.\n";
        return;
    }
    json config_path_kwds = config.at("paths");
    for (json::const_iterator it=config_path_kwds.begin(); it!=config_path_kwds.end(); ++it)
        showPathKwdConfig(it);
}


std::string cdPathFromDirKwdSequence( const std::vector<std::string>& dir_kwd_seq
                                    , const std::string& config_file_abs_path )
{
    json config; 
    if (!parseConfigToJsonObjAndValidate(config_file_abs_path, config))
    {
        cerr << "ERROR: Failed to validate config file.\n";
        return ".";
    }
    //json groups = config.at("groups");
    json current_config = config.at("paths");
    const string cd_root = config.at("root");
    string cd_path = "";
    for (const auto& kwd : dir_kwd_seq)
    {
        if (!current_config.contains(kwd))
        {
            // TODO: do group check
        }
        if (!current_config.at(kwd).contains("dir"))
        {
            cerr << "ERROR: directory keyword exists but does not have an associated dir.\n";
            return ".";
        }
        const string npe = current_config.at(kwd).at("dir");
        cd_path += npe + "/";
        if (!current_config.contains("children"))
            return cd_root + "/" + cd_path;
        current_config = current_config.at(kwd).at("children");
    }
    return cd_root + "/" + cd_path;
}
