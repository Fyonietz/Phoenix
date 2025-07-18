#include "engine.hpp"

//Global Variable

std::string Global::dll_name = "core/msys-routes.dll";

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
    Config::routes = config["Routes_Type"].c_str();
}

void static_routes(){
    std::cout << "Web++[Info]: Routes Type: Static" << std::endl;

    std::wstring w_dll_name = CharToWChar(Global::dll_name.c_str());
    HMODULE hDLL = LoadLibraryW(w_dll_name.c_str()); 
    if(!hDLL){
        std::cerr << "failed to load .dll" << std::endl;
        return;
    }
    RouteFunc test = (RouteFunc)GetProcAddress(hDLL,"update");
    if(!test){
        std::cerr << "Failed to find function" << std::endl;
    }
    test();

    FreeLibrary(hDLL);
    return;
}
HINSTANCE dllHandle = nullptr;
RouteFunc routefunc = nullptr;
std::filesystem::file_time_type lastWriteTime;
bool dynamic_routes(){    
    std::cout << "Web++[Info]: Routes Type: Dynamic" << std::endl;
    std::wstring w_dll_name = CharToWChar(Global::dll_name.c_str());
    if(dllHandle){
        FreeLibrary(dllHandle);
        dllHandle=nullptr;
    }

    std::string temp_dll = "core/msys-routes-temp.dll";
    std::wstring w_temp_dll = CharToWChar(temp_dll.c_str());
    
    // Before copying, ensure the temp DLL is not locked and can be deleted
    if (dllHandle) {
        FreeLibrary(dllHandle);
        dllHandle = nullptr;
    }
    if (std::filesystem::exists(temp_dll)) {
        std::filesystem::remove(temp_dll);
    }
    std::filesystem::copy_file(
        Global::dll_name,
        temp_dll,
        std::filesystem::copy_options::overwrite_existing
    );

    dllHandle = LoadLibrary(temp_dll.c_str());
    if(!dllHandle){
        std::cerr << "Failed To Load DLL .\n";
        return false;
    }
    // FIX: assign to the global routefunc, not a local variable!
    routefunc = (RouteFunc)GetProcAddress(dllHandle,"update");
    if(!routefunc){
        std::cerr << "Failed To Get Function\n";
        return false;
    }
    return true;
}

void dynamic_routes_starter(){
    lastWriteTime = std::filesystem::last_write_time(Global::dll_name);
    while(true){
        if(std::filesystem::last_write_time(Global::dll_name) != lastWriteTime){
            lastWriteTime = std::filesystem::last_write_time(Global::dll_name);
            std::cout << "Web++[Info]: Routes Updated" << std::endl;
            dynamic_routes();
        }
        if(routefunc)routefunc();
    
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
};


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