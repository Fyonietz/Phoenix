#include "engine.hpp"


struct mg_context* server_context = nullptr;
bool server_running = false;
HINSTANCE dllHandle = nullptr;
RouteFunc routefunc = nullptr;
std::filesystem::file_time_type lastWriteTime;
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
    RouteFunc test = (RouteFunc)GetProcAddress(hDLL,"Routes_LinkUp");
    if(!test){
        std::cerr << "Failed to find function" << std::endl;
    }
    test();

    FreeLibrary(hDLL);
    return;
};

bool dynamic_routes() {    
    std::cout << "Web++[Info]: Routes Type: Dynamic" << std::endl;
    std::wstring w_dll_name = CharToWChar(Global::dll_name.c_str());
    
    if (dllHandle) {
        FreeLibrary(dllHandle);
        dllHandle = nullptr;
    }

    std::string temp_dll = "core/msys-routes-temp.dll";
    std::wstring w_temp_dll = CharToWChar(temp_dll.c_str());
    
    if (std::filesystem::exists(temp_dll)) {
        std::filesystem::remove(temp_dll);
    }

    try {
        std::filesystem::copy_file(
            Global::dll_name,
            temp_dll,
            std::filesystem::copy_options::overwrite_existing
        );
    } catch (const std::exception& e) {
        std::cerr << "Copy failed: " << e.what() << std::endl;
        return false;
    }

    dllHandle = LoadLibrary(temp_dll.c_str());
    if(!dllHandle){
        DWORD err = GetLastError();
        std::cerr << "Failed To Load DLL. Error: " << err << std::endl;
        return false;
    }

    routefunc = (RouteFunc)GetProcAddress(dllHandle,"Routes_LinkUp");
    if(!routefunc){
        DWORD err = GetLastError();
        std::cerr << "Failed To Get Function. Error: " << err << std::endl;
        return false;
    }

    std::cout << "DEBUG: Before routefunc() call" << std::endl;
    
    try {
        routefunc();
        std::cout << "DEBUG: After routefunc() call" << std::endl;
    } catch (...) {
        std::cerr << "CRASH: routefunc() threw an exception!" << std::endl;
        return false;
    }

    return true;
}

void dynamic_routes_starter(){
    std::cout << "Routes Starter" << std::endl;
    lastWriteTime = std::filesystem::last_write_time(Global::dll_name);
    while(true){
        if(std::filesystem::last_write_time(Global::dll_name) != lastWriteTime){
            lastWriteTime = std::filesystem::last_write_time(Global::dll_name);
            std::cout << "Web++[Info]: Routes Updated" << std::endl;
            dynamic_routes();
            system("ninja -C build routes");
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
};

void start() {
    std::cout << "endpoint " << std::endl;
    std::wstring w_dll_name = CharToWChar(Global::dll_name.c_str());
    HMODULE hDLL = LoadLibraryW(w_dll_name.c_str());
    if (!hDLL) {
        std::cerr << "Failed to load routes DLL.\n";
        return;
    }

    auto Routes_LinkUp = (RoutesLinkUpFunc)GetProcAddress(hDLL, "Routes_LinkUp");
    if (!Routes_LinkUp) {
        std::cerr << "Failed to find Routes_LinkUp function in DLL.\n";
        FreeLibrary(hDLL);
        return;
    }

    Routes_LinkUp(Config::root.c_str(), Config::port.c_str(), 
                 Config::threads.c_str(), Config::keep_alive.c_str());

    // Keep main thread alive indefinitely
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}