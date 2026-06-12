#include "hf.hpp"

#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace std;
using json = nlohmann::json;

constexpr char GROUP_ID_CHAR = '$';

const unordered_map<string, ActionType> cmd_str_to_enum = {
    { "--add",    ADD    },
    { "--update", UPDATE },
    { "--remove", REMOVE },
    { "--list",   LIST   },
};
const unordered_map<string, ActionType> cmd_abbrv_str_to_enum = {
    { "-a", ADD    },
    { "-u", UPDATE },
    { "-r", REMOVE },
    { "-l", LIST   }
};


/////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Private //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

bool parseFileToJsonObj( const std::string& config_file_abs_path
                       , json& jsn )
{
    ifstream config_file(config_file_abs_path);
    if (!config_file.is_open())
    {
        cout << "Failed to open config file: " << config_file_abs_path << '\n';
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
    const string dirname { path_kwd_config.value().at("dir") };
    string tab="";
    for (size_t i=0; i!= start_tabs; ++i)
        tab += "\t";
    cout << tab << path_kwd_config.key() << "\t" << dirname << '\n';

    if (path_kwd_config.value().contains("children")
    && !path_kwd_config.value().at("children").empty())
    {
        const auto children_config = path_kwd_config.value().at("children");
        for (json::const_iterator it=children_config.begin(); it!=children_config.end(); ++it)
            showPathKwdConfig(it, start_tabs+1);
    }
}


json* ptrToConfigPathAliasNodeInJson(const vector<string>& alias_sequence
                                   , json& paths_config)
{
    json* current_config_node = &paths_config;
    for (size_t i=0; i!=alias_sequence.size(); ++i)
    {
        const auto& alias = alias_sequence.at(i);
        if (!current_config_node->contains(alias))
        {
            cerr << "ERROR: alias not in config\n";
            return nullptr;
        }
        current_config_node = &current_config_node->at(alias);
        if (i == alias_sequence.size()-1)
        {
            break;
        }
        if (current_config_node->contains("children"))
        {
            cout << "node has children: " << alias << '\n';
            current_config_node = &current_config_node->at("children");
        }
    }
    return current_config_node;
}


bool addDirAliasToConfigCore(
    const std::vector<std::string>& preceding_alii
  , const std::string& alias
  , const std::string& directory_path //! relative to dirs corresponding to preceding alii, if they exist
  , const std::string& config_file_abs_path
){
    json config;
    if (!parseConfigToJsonObjAndValidate(config_file_abs_path, config))
    {
        cerr << "ERROR: Failed to validate config file.\n";
        return false;
    }
    json& paths_config = config.at("paths");
    json* ptr_to_preceding_alii_config_node = &paths_config;
    if (!preceding_alii.empty())
    {
        ptr_to_preceding_alii_config_node = ptrToConfigPathAliasNodeInJson(preceding_alii, paths_config);
        cout << "ptr_to_preceding_alii_config_node: " << ptr_to_preceding_alii_config_node->at("dir") << '\n';
        if (!ptr_to_preceding_alii_config_node->contains("children"))
            ptr_to_preceding_alii_config_node->push_back({"children", {}});
        ptr_to_preceding_alii_config_node = &ptr_to_preceding_alii_config_node->at("children");
    }
    json& config_node = *ptr_to_preceding_alii_config_node;
    config_node[alias] = {
        { "dir", directory_path }
    };
    ofstream new_config_file(config_file_abs_path);
    new_config_file << std::setw(4) << config << std::endl;
    new_config_file.close();

    return true;
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


void showConfig( const std::string& config_file_abs_path
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


bool addDirAliasToConfig( const std::string& config_file_abs_path
                        , const std::string& user
                        , const std::vector<std::string>& args)
{
    const size_t argc = args.size();
    if (argc < 3)
    {
        cerr << "ERROR: Not enough args passed to add an alias to the config.\n";
        return false;
    }
    string directory = args.at(argc-1);
    string alias = args.at(argc-2);
    vector<string> preceding_alii = args;
    for (size_t i=0; i!=3 && !preceding_alii.empty(); ++i)
        preceding_alii.pop_back();

    //!< DEVONLY
    string alii_str = "";
    for (const auto& alias : preceding_alii)
        alii_str += alias + " ";
    cout << "Adding Directory Alias\n";
    cout << " preceeding alii: " << alii_str << '\n';
    cout << " alias: " << alias << '\n';
    cout << " directory: " << directory << '\n';
    //!< END DEVONLY

    return addDirAliasToConfigCore(preceding_alii,
                                   alias,
                                   directory,
                                   config_file_abs_path);
}
