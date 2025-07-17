#include "engine.hpp"
std::wstring CharToWChar(const char* str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str, -1, &wstr[0], size_needed);
    // Remove the extra null terminator for std::wstring
    if (!wstr.empty() && wstr.back() == L'\0') wstr.pop_back();
    return wstr;
}
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


void initConfig() {
    auto config = loadConfig();
    Config::root = config["Document_Root"].c_str();
    Config::port = config["Server_Port"].c_str();
    Config::threads = config["Number_Threads"].c_str();
    Config::keep_alive = config["Keep_Alive"].c_str();
}

void routes(){
    std::string dll_name = "core/msys-routes.dll";
    std::wstring w_dll_name = CharToWChar(dll_name.c_str());
    HMODULE hDLL = LoadLibraryW(w_dll_name.c_str()); 
    if(!hDLL){
        std::cerr << "failed to load .dll" << std::endl;
        return;
    }
    Route test = (Route)GetProcAddress(hDLL,"update");
    if(!test){
        std::cerr << "Failed to find function" << std::endl;
    }
    test();

    FreeLibrary(hDLL);
    return;
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