#include "engine.hpp"

//Global Variable
std::mutex dllMutex;


std::wstring CharToWChar(const char* str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str, -1, &wstr[0], size_needed);
    // Remove the extra null terminator for std::wstring
    if (!wstr.empty() && wstr.back() == L'\0') wstr.pop_back();
    return wstr;
}


void release(){
    std::cout <<Global::info << "Routes Type: Static" << std::endl;

    std::wstring w_dll_name = CharToWChar(Global::dll_name.c_str());
    HMODULE hDLL = LoadLibraryW(w_dll_name.c_str()); 
    if(!hDLL){
        std::cerr <<Global::info<< "Failed To Load .dll" << std::endl;
        return;
    }
   typedef void(*UpdateFunc)(struct mg_context *);
    UpdateFunc update = (UpdateFunc)GetProcAddress(hDLL, "update");
    if(!update){
        std::cerr <<Global::info<< "Failed to find function" << std::endl;
    }
    update(Config::context);
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


//Debug
HMODULE dllHandle = nullptr;
RouteFunc routefunc = nullptr;
std::filesystem::file_time_type lastWriteTime;
bool debug() {    
    std::lock_guard<std::mutex> lock(dllMutex); 
    std::cout<<Global::info << "Routes Type: Dynamic" << std::endl;
    
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
            std::cerr <<Global::info << "Failed To Load DLL. Error: " << err << "\n";
            return false;
        }

        typedef void(*UpdateFunc)(struct mg_context *);
        UpdateFunc update = (UpdateFunc)GetProcAddress(dllHandle, "update");
        if(!update) {
            std::cerr <<Global::info << "Failed To Get Function\n";
            FreeLibrary(dllHandle);
            dllHandle = nullptr;
            return false;
        }
        update(Config::context);
        // Update the last write time
        lastWriteTime = std::filesystem::last_write_time(Global::dll_name);
        
        // Clean up old temp files (optional)
        clean_old_temp_dlls();
        
        return true;
    } catch(const std::exception& e) {
        std::cerr << Global::info<< "Error in dynamic_routes: " << e.what() << "\n";
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
                    std::cout <<Global::info<< "Successfully reloaded routes!\n";
                } else {
                    std::cerr <<Global::info<< "Failed to reload routes\n";
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
            std::cerr <<Global::info<< "Error in reload thread\n";
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
}

void debug_start(const char *root,const char *port,const char *threads,const char *alive){
    updateConfig();
};

void release_start(const char *root,const char *port,const char *threads,const char *alive){
    updateConfig();

};