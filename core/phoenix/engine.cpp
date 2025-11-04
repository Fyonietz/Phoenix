#include "engine.hpp"
#include "route_register.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <string.h>
#include <cstring>
#include <thread>
typedef void (*setter_t)(SharedData*);
namespace fs = std::filesystem;
std::unordered_map<std::string, std::string> wpc_loader(const std::string& path){
 
  std::unordered_map<std::string, std::string> config;
  std::ifstream f(path);

  if(!f.is_open()){
    std::cerr <<Formatter::err<< "Error Opened Phoenix Files" <<std::endl;
    return config;
  }
  std::stringstream ss;

 std::string line;
  while (std::getline(f, line)) {
    if (line.empty() || line[0] == '#')
      continue;

    std::istringstream input_string(line);
    std::string key, value;
    if (std::getline(input_string, key, '=') &&
        std::getline(input_string, value)) {
      config[key] = value;
    }
  }
  return config;
}

bool watch_and_reload(const std::string& directory) {
    std::unordered_map<std::string, std::chrono::system_clock::time_point> file_mod_times;

    while (true) {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                const auto& path = entry.path();
                auto mod_time = fs::last_write_time(path);

                if (file_mod_times.find(path.string()) == file_mod_times.end() ||
                    file_mod_times[path.string()] != mod_time) {
                    std::cout << "File " << path << " has been modified. Reloading...\n";
                    file_mod_times[path.string()] = mod_time;

                    // Reload your server or library here
                    if (!engine_start()) {
                        std::cerr << "Error reloading engine" << std::endl;
                        return false;
                    }
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));  // Poll every second
    }
}
//Config Declrative
bool config_declare(){
    Formatter::err = "[> Phoenix [Core] > Error]: ";
    Formatter::info = "[> Phoenix [Core] > Info]: ";
     auto config = wpc_loader("config/core/server.wpc");
    if(config.empty()){
    std::cerr <<Formatter::err<< "Failed to load configuration\n";
    return false;
    }

    Config::root = config["Document_Root"];
    Config::port = config["Server_Port"];
    Config::threads = config["Number_Threads"];
    Config::keep_alive = config["Keep_Alive"];

 // Validate configuration
  if (Config::root.empty()) {
    std::cerr<<Formatter::err<< "Missing Document_Root in configuration\n";
  }
  if (Config::port.empty()) {
    std::cerr<<Formatter::err<< "Missing Server_Port in configuration\n";
  }
  if (Config::threads.empty()) {
    std::cerr<<Formatter::err<< "Missing Number_Threads in configuration\n";
  }
  if (Config::keep_alive.empty()) {
    std::cerr<<Formatter::err << "Missing Keep_Alive in configuration\n";
  }
    return true;
}

bool library_loader_and_routes_register(){
  // Create an actual SharedData object (not just a pointer)
  SharedData SD;
  SD.context = Config::context;
  SD.callbacks = Config::callbacks;
  
  void *handle = dlopen("./libroutes.so", RTLD_LAZY);
  if (!handle) {
    std::cerr << "failed to open library: " << dlerror() << std::endl;
    return false;  // Changed from 1 to false for consistency
  }
  dlerror();

  setter_t trigger_setter = (setter_t)dlsym(handle, "routes_update");
  const char *error = dlerror();
  if (error) {
    std::cerr << "Error Loading Function: " << error << std::endl;
    dlclose(handle);
    return false;  // Changed from 1 to false
  }
  
  // Pass the address of the actual object
  trigger_setter(&SD);
  
  return true;  // Changed from 0 to true
}
bool engine_start(){
  // First: Load configuration
  if(!config_declare()){
    std::cerr<<Formatter::err<<"Error At Config Declarative" << std::endl;
    return false;
  }

  // Set up server configuration with correct port format
  const char *options[] = {"document_root",
                           Config::root.c_str(),
                           "listening_ports",
                           Config::port.c_str(),
                           "num_threads",
                           Config::threads.c_str(),
                           "enable_keep_alive",
                           Config::keep_alive.c_str(),
                           "index_files",
                           "layout.html",
                           nullptr};

  // Print configuration for debugging
  std::cout <<Formatter::info<< "Starting server with configuration:\n";
  for (int i = 0; options[i] != nullptr; i += 2) {
    std::cout << Formatter::info << options[i] << ": " << options[i + 1] << "\n";
  }

  // Second: Start the CivetWeb server (this initializes Config::context)
  Config::context = mg_start(&Config::callbacks, nullptr, options);

  if (!Config::context) {
    std::cerr <<Formatter::err<< "Failed to start server: "<<strerror(errno)<< "\n";
    if (std::filesystem::exists("error.log")) {
      std::ifstream log("error.log");
      std::string line;
      while (std::getline(log, line)) {
        std::cerr << Formatter::info<< "Error log: " << line << "\n";
      }
    }
    return false;
  }
  
  std::cout << Formatter::info
            << "Server started successfully on http://localhost:"
            << Config::port << std::endl;

  // Third: Now that context exists, load library and register routes
  if(!library_loader_and_routes_register()){
    std::cerr << Formatter::err << "Failed to load routes library" << std::endl;
    return false;
  }

  return true;
}

