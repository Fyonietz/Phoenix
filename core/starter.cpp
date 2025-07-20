#include "starter.hpp"


std::string Global::dll_name = "core/msys-routes.dll";
std::string Global::info = " > Phoenix[Info]: ";
std::unordered_map<std::string,std::string> loadConfig(){
    std::unordered_map<std::string,std::string> config;
    std::ifstream file("config/server.wpc");

    if(!file){
        std::cerr << "Failed To Open Config File: " <<std::endl;
        return config;
    }

    std::string line;
    while (std::getline(file,line))
    {
        if(line.empty() || line[0] == '#')continue;

        std::istringstream input_string(line);
        std::string key, value;
        if(std::getline(input_string,key,'=') && std::getline(input_string,value)){
            config[key]=value.c_str();
        }
    }
    return config;
}
void updateConfig(){
    Config::server_config[1] = Config::root.c_str();
    Config::server_config[3] = Config::port.c_str();
    Config::server_config[5] = Config::threads.c_str();
    Config::server_config[7] = Config::keep_alive.c_str();
    Config::callbacks = {};
    Config::context = mg_start(&Config::callbacks, 0, Config::server_config);

    if (!Config::context) {
    std::cerr <<Global::info<< "Failed To Start Server From Starter" << std::endl;
} else {
    std::cout <<Global::info<< "Server Started On http://localhost:" << Config::port << std::endl;
}
}

void initConfig() {
    auto config = loadConfig();
    Config::root = config["Document_Root"];
    Config::port = config["Server_Port"];
    Config::threads = config["Number_Threads"];
    Config::keep_alive = config["Keep_Alive"];
    Config::routes = config["Routes_Type"];
}
const char *Config::server_config[] = {
    "document_root",        nullptr,
    "listening_ports",      nullptr,
    "num_threads",          nullptr,
    "enable_keep_alive",    nullptr,
    "enable_directory_listing", "no",
    "index_files",          "layout.html",
    0
};
