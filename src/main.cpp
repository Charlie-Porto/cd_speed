//#include "hf.hpp"
#include <unistd.h> // For getlogin_r()
#include <limits.h> // For LOGIN_NAME_MAX

#include "hf.hpp"

#include <iostream>


void printManual()
{
    const std::string manual = "\
Usage: cds <directory_alii...>\n\
       cds <command> [<args...>]\n\
Commands:\n\
 -l|--list\t\t List directory alii options from config\n\
 -a|--add <alias> <dir>\t Add a directory alias to config\n\
";

    std::cout << manual << '\n';
}


int main(int argc, char* argv[])
{
    using namespace std;
    using namespace std::filesystem;

    //! Get config file path
#ifdef __linux__
    char username[LOGIN_NAME_MAX];
    if (!getlogin_r(username, sizeof(username)) == 0)
    {
        cerr << "Unable to get username.\n";
        return EXIT_FAILURE;
    }
    const string user{ username };
    const path config_file{ string("/home/") + user + "/.config/cd_speed/cds_config.json" };
#elif defined(__APPLE__)
    const string user{ getlogin() };
    const path config_file{ string("/Users/") + user + "/.config/cd_speed/cds_config.json" };
#elif defined(_WIN32)
    cout << "Sorry, cd-speed is not yet ported to Windows. Exiting.\n";
    return EXIT_SUCCESS;
#endif
    if (!is_regular_file(config_file))
    {
        cerr << "Config file does not exist: " << config_file << '\n';
        cout << '\n';
        printManual();
        return EXIT_FAILURE;
    }
    //!< DEVONLY
    //cout << "config file path: " << config_file << '\n';

    //! Parse input
    vector<string> args{};
    for (int i=1; i<argc; ++i)
        args.push_back(string(argv[i]));

    //////////////////// CHECKS THAT ARE USUALLY AT THE BEGINNING ////////////////////
    //! Check if manual should be printed
    if (argc == 1)
    {
        //printManual();
        cout << "Config\n-------------------------------------------\n";
        showConfig(config_file, user, args);
        return EXIT_SUCCESS;
    }
    else if (string(argv[1]) == string("-h") || string(argv[1]) == string("--help"))
    {
        printManual();
        return EXIT_SUCCESS;
    }
    //////////////////////////////////////////////////////////////////////////////////

    const ActionType action = actionTypeFromArgs(args);
    switch (action)
    {
        case CDS:
        {
            const string cd_path = cdPathFromDirKwdSequence(args, config_file);
            if (cd_path == "")
            {
                cerr << "ERROR: directory not found for alias sequence.\n";
                return EXIT_FAILURE;
            }
            cout << "cds:" << cd_path;
            break;
        }
        case LIST:
            for (size_t i=0; i!=args.size(); ++i)
            {
                if (args[i] == "list" || args[i] == "l")
                {
                    args.erase(args.begin()+i);
                    break;
                }
            }
            cout << "Config\n-------------------------------------------\n";
            showConfig(config_file, user, args);
            break;
        case ADD:
            if (!addDirAliasToConfig(config_file, user, args))
            {
                cerr << "ERROR: unable to add alias to config file.\n";
                return EXIT_FAILURE;
            }
            else
            {
                cout << "Success: added kwd to config file.\n";
            }
            break;
        default:
            break;
    }

    return EXIT_SUCCESS;
}
