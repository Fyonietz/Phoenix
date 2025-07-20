#include "engine.hpp"

//Global Variable
std::mutex dllMutex;
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

void release(){
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
    getchar();
    mg_stop(&context);
    return;
}
void clean_old_temp_dlls() {
    try {
        for (const auto& entry : std::filesystem::directory_iterator("core")) {
            if (entry.path().string().find("msys-routes-temp-") != std::string::npos) {
                // Don't delete our current temp DLL
                if (dllHandle && GetModuleFileNameA(dllHandle, nullptr, 0) != 0) {
                    char modulePath[MAX_PATH];
                    GetModuleFileNameA(dllHandle, modulePath, MAX_PATH);
                    if (entry.path().string() == modulePath) {
                        continue;
                    }
                }
                
                try {
                    std::filesystem::remove(entry.path());
                } catch (...) {
                    // Ignore deletion errors
                }
            }
        }
    } catch (...) {
        // Ignore directory iteration errors
    }
}

HMODULE dllHandle = nullptr;
RouteFunc routefunc = nullptr;
std::filesystem::file_time_type lastWriteTime;
bool debug() {    
    std::lock_guard<std::mutex> lock(dllMutex); 
    std::cout << "Web++[Info]: Routes Type: Dynamic" << std::endl;
    
    // Unload previous DLL if loaded
    if(dllHandle) {
        FreeLibrary(dllHandle);
        dllHandle = nullptr;
        routefunc = nullptr;
    }

    // Create a uniquely named temp DLL
    std::string temp_dll = "core/msys-routes-temp-" + 
                          std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + 
                          ".dll";
    
    try {
        // Copy to temp file
        std::filesystem::copy_file(
            Global::dll_name,
            temp_dll,
            std::filesystem::copy_options::overwrite_existing
        );

        // Load the new DLL
        dllHandle = LoadLibraryA(temp_dll.c_str());
        if(!dllHandle) {
            DWORD err = GetLastError();
            std::cerr << "Failed To Load DLL. Error: " << err << "\n";
            return false;
        }

        // Get the update function
        routefunc = (RouteFunc)GetProcAddress(dllHandle, "update");
        if(!routefunc) {
            std::cerr << "Failed To Get Function\n";
            FreeLibrary(dllHandle);
            dllHandle = nullptr;
            return false;
        }

        // Update the last write time
        lastWriteTime = std::filesystem::last_write_time(Global::dll_name);
        
        // Clean up old temp files (optional)
        clean_old_temp_dlls();
        
        return true;
    } catch(const std::exception& e) {
        std::cerr << "Error in dynamic_routes: " << e.what() << "\n";
        return false;
    }
}

void debug_watcher() {
    while (true) {
        try {
            // Check for file changes
            auto currentTime = std::filesystem::last_write_time(Global::dll_name);
            if (currentTime != lastWriteTime) {
               
                if (debug()) {
                    std::cout << "Successfully reloaded routes!\n";
                } else {
                    std::cerr << "Failed to reload routes\n";
                }
            }
              
            // Call update function if available
            if (routefunc) {
                std::lock_guard<std::mutex> lock(dllMutex);
                routefunc();
            }

            std::this_thread::sleep_for(std::chrono::seconds(5));
             system("ninja -C build routes > NUL 2>&1");
        } catch (...) {
            std::cerr << "Error in reload thread\n";
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
}

void debug_start(const char *root,const char *port,const char *threads,const char *alive){
  
    const char *config[]={
    "document_root",root,
    "listening_ports",port,
    "num_threads",threads,
    "enable_keep_alive",alive,
    "enable_directory_listing","no",
    "index_files","layout.html",
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
    
};

void release_start(const char *root,const char *port,const char *threads,const char *alive){
  
    const char *config[]={
    "document_root",root,
    "listening_ports",port,
    "num_threads",threads,
    "enable_keep_alive",alive,
    "enable_directory_listing","no",
    "index_files","layout.html",
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
};