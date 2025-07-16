#include "app.hpp"

std::unordered_map<std::string,std::string> loadConfig(){
    std::unordered_map<std::string,std::string> config;
    std::ifstream file("config/server.wpp");

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


void initConfig() {
    auto config = loadConfig();
    Config::root = config["Document_Root"].c_str();
    Config::port = config["Server_Port"].c_str();
    Config::threads = config["Number_Threads"].c_str();
    Config::keep_alive = config["Keep_Alive"].c_str();
}


void start(const char *root,const char *port,const char *threads,const char *alive){
  
    const char *config[]={
    "document_root",root,
    "listening_ports",port,
    "num_threads",threads,
    "enable_keep_alive",alive,
    0
    };

    struct mg_callbacks callbacks={};
    struct mg_context *context = mg_start(&callbacks,0,config);
    if(!context){
        std::cerr << "Failed To Start Server" << std::endl;
        return;
    }else{
        std::cout << "Server Start On http://localhost:" << Config::port << std::endl;
    }
    getchar();

    mg_stop(context);
};