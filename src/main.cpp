//#include "hf.hpp"
#include <unistd.h> // For getlogin_r()
#include <limits.h> // For LOGIN_NAME_MAX
                    //
                    //
int main(int argc, char* argv[])
{
    using namespace std;
    using namespace std::filesystem;

    //! Check if manual should be printed
    if (argc == 1 || argv[1] == "-h" || argv[1] == "--help")
    {
        cout << "usage: cds [ARGS...] [FLAGS]\n";
    }

    //! Get config file path
    char username[LOGIN_NAME_MAX];
    if (!getlogin_r(username, sizeof(username)) == 0)
    {
        cerr << "Unable to get username.\n";
        return EXIT_FAILURE;
    }
    const string user{ username };
    //! TODO: make OS agnostic
    const path config_file{ string("/Users/") + user + "/.config/cd_speed/cds_config.json" };
    if (!is_regular_file(config_file))
    {
        cerr << "Config file does not exist: " << config_file << '\n';
        return EXIT FAILURE;
    }
}
