#include "engine.hpp"
#include "route_register.hpp"
#include <chrono>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <pyro.hpp>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <unistd.h>

Pnix Server(4);

typedef void (*setter_t)(SharedData *);

void clear_screen() {
#ifdef _WIN32
  system("cls");
#else
  // ANSI escape code for clearing terminal and moving cursor to top-left
  std::cout << "\033[2J\033[H" << std::flush;
#endif
}
// Global variables for hot reload management
std::atomic<bool> hot_reload_in_progress{false};
std::atomic<bool> engine_running{false};
std::atomic<bool> shutdown_requested{false};
static void *current_routes_handle = nullptr;
static std::mutex reload_mutex;
static std::mutex context_mutex;
static std::string current_temp_so_path;
static std::filesystem::file_time_type last_check_time;

// Function to load configuration file
std::unordered_map<std::string, std::string>
wpc_loader(const std::string &path) {
  std::unordered_map<std::string, std::string> config;
  std::ifstream f(path);

  if (!f.is_open()) {
    std::cerr << Formatter::err << "Error Opening Phoenix Files" << std::endl;
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

// Clean old temporary shared libraries
void clean_old_temp_dlls() {
  try {
    std::string current_filename =
        std::filesystem::path(current_temp_so_path).filename().string();

    for (const auto &entry : std::filesystem::directory_iterator(".")) {
      std::string filename = entry.path().filename().string();

      if (filename.find("libroutes-temp-") == std::string::npos) {
        continue;
      }

      if (filename == current_filename) {
        continue;
      }

      try {
        std::filesystem::remove(entry.path());
        std::cout << Formatter::info << "Cleaned old temp library: " << filename
                  << std::endl;
        clear_screen();
        std::cout << Formatter::info << "Reloading Routes Is Completed."
                  << std::endl;
      } catch (...) {
        // Ignore deletion errors
      }
    }
  } catch (const std::exception &e) {
    std::cerr << Formatter::err << "Error cleaning temp libraries: " << e.what()
              << std::endl;
  }
}

// Helper function for restart without reload
void restart_server_without_reload() {
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

  Config::context = mg_start(&Config::callbacks, nullptr, options);
  if (!Config::context) {
    std::cerr << Formatter::err
              << "Failed to restart server: " << strerror(errno) << std::endl;
  } else {
    std::cout << Formatter::info << "Server restarted with previous routes."
              << std::endl;
  }
}

// Function to recompile and reload the routes
void recompile_and_reload() {
  std::lock_guard<std::mutex> lock(reload_mutex);

  if (shutdown_requested.load()) {
    std::cout << Formatter::info << "Shutdown in progress, skipping reload..."
              << std::endl;
    return;
  }

  if (hot_reload_in_progress.exchange(true)) {
    std::cout << Formatter::info << "Reload already in progress, skipping..."
              << std::endl;
    return;
  }

  if (!engine_running.load()) {
    hot_reload_in_progress = false;
    return;
  }

  std::lock_guard<std::mutex> ctx_lock(context_mutex);
  if (Config::context == nullptr) {
    std::cerr << Formatter::err
              << "Server context is not available, cannot reload routes!"
              << std::endl;
    hot_reload_in_progress = false;
    return;
  }

  std::cout << Formatter::info << "Recompiling routes..." << std::endl;
  int compile_result = system(
      "make -C build routes 2>&1 | grep -E '(Building|Linking|Error|error)'");
  if (compile_result != 0) {
    std::cerr << Formatter::err
              << "Compilation failed with code: " << compile_result
              << std::endl;
    hot_reload_in_progress = false;
    return;
  }

  std::string original_so = "./libroutes.so";
  if (!std::filesystem::exists(original_so)) {
    std::cerr << Formatter::err
              << "Error: Library compilation failed - libroutes.so not found!"
              << std::endl;
    hot_reload_in_progress = false;
    return;
  }

  // Extra safety check before proceeding
  if (shutdown_requested.load()) {
    std::cout << Formatter::info
              << "Shutdown requested during compilation, aborting reload..."
              << std::endl;
    hot_reload_in_progress = false;
    return;
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  std::string temp_dll =
      "./libroutes-temp-" +
      std::to_string(
          std::chrono::system_clock::now().time_since_epoch().count()) +
      ".so";

  try {
    std::filesystem::copy_file(
        original_so, temp_dll,
        std::filesystem::copy_options::overwrite_existing);
  } catch (const std::exception &e) {
    std::cerr << Formatter::err << "Failed to copy library: " << e.what()
              << std::endl;
    hot_reload_in_progress = false;
    return;
  }

  std::cout << Formatter::info << "Stopping server for hot reload..."
            << std::endl;

  struct mg_context *old_context = Config::context;
  Config::context = nullptr;

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  if (old_context) {
    try {
      mg_stop(old_context);
    } catch (...) {
      std::cerr << Formatter::err << "Exception during mg_stop" << std::endl;
    }
  }

  if (current_routes_handle) {
    dlclose(current_routes_handle);
    current_routes_handle = nullptr;
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  void *handle = dlopen(temp_dll.c_str(), RTLD_NOW | RTLD_LOCAL);
  if (!handle) {
    std::cerr << Formatter::err << "Error loading new library: " << dlerror()
              << std::endl;
    hot_reload_in_progress = false;
    restart_server_without_reload();
    try {
      std::filesystem::remove(temp_dll);
    } catch (...) {
    }
    return;
  }

  current_routes_handle = handle;
  current_temp_so_path = temp_dll;

  dlerror();

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

  Config::context = mg_start(&Config::callbacks, nullptr, options);
  if (!Config::context) {
    std::cerr << Formatter::err
              << "Failed to restart server: " << strerror(errno) << std::endl;
    dlclose(handle);
    current_routes_handle = nullptr;
    hot_reload_in_progress = false;
    try {
      std::filesystem::remove(temp_dll);
    } catch (...) {
    }
    return;
  }

  setter_t trigger_setter = (setter_t)dlsym(handle, "routes_update");
  const char *error = dlerror();
  if (error) {
    std::cerr << Formatter::err
              << "Error loading routes_update function: " << error << std::endl;
    mg_stop(Config::context);
    Config::context = nullptr;
    dlclose(handle);
    current_routes_handle = nullptr;
    hot_reload_in_progress = false;
    try {
      std::filesystem::remove(temp_dll);
    } catch (...) {
    }
    return;
  }

  SharedData SD;
  SD.context = Config::context;
  SD.callbacks = Config::callbacks;

  try {
    trigger_setter(&SD);
    std::cout << Formatter::info << "Routes registered successfully."
              << std::endl;
  } catch (const std::exception &e) {
    std::cerr << Formatter::err
              << "Exception during route registration: " << e.what()
              << std::endl;
    mg_stop(Config::context);
    Config::context = nullptr;
    dlclose(handle);
    current_routes_handle = nullptr;
    hot_reload_in_progress = false;
    try {
      std::filesystem::remove(temp_dll);
    } catch (...) {
    }
    return;
  }

  std::cout << Formatter::info
            << "Hot reload completed on http://localhost:" << Config::port
            << std::endl;

  clean_old_temp_dlls();
  hot_reload_in_progress = false;
}

// Get the latest modification time from all files in directory
std::filesystem::file_time_type
get_latest_modification_time(const std::string &directory) {
  auto latest_time = std::filesystem::file_time_type::min();

  try {
    for (const auto &entry :
         std::filesystem::recursive_directory_iterator(directory)) {
      if (entry.is_regular_file()) {
        std::string filename = entry.path().filename().string();

        // Only check .cpp and .hpp files
        if (filename.ends_with(".cpp") || filename.ends_with(".hpp")) {
          // Skip temporary/hidden files
          if (filename[0] == '.' || filename.find("~") != std::string::npos) {
            continue;
          }

          auto file_time = std::filesystem::last_write_time(entry.path());
          if (file_time > latest_time) {
            latest_time = file_time;
          }
        }
      }
    }
  } catch (const std::exception &e) {
    std::cerr << Formatter::err << "Error checking file times: " << e.what()
              << std::endl;
  }

  return latest_time;
}

// Simple polling-based hot reload watcher
void hot_reload(const std::string &path) {
  std::cout << Formatter::info << "Hot reload watcher started for: " << path
            << std::endl;

  // Initialize with current time
  last_check_time = get_latest_modification_time(path);

  while (engine_running.load() && !shutdown_requested.load()) {
    // Sleep for 1 second between checks
    std::this_thread::sleep_for(std::chrono::seconds(1));

    if (shutdown_requested.load() || !engine_running.load()) {
      break;
    }

    // Check if any files have been modified
    auto current_time = get_latest_modification_time(path);

    if (current_time > last_check_time) {
      std::cout << Formatter::info
                << "File changes detected, triggering reload..." << std::endl;
      last_check_time = current_time;

      // Wait a bit to ensure file write is complete
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      try {
        recompile_and_reload();
      } catch (const std::exception &e) {
        std::cerr << Formatter::err << "Exception during reload: " << e.what()
                  << std::endl;
      } catch (...) {
        std::cerr << Formatter::err << "Unknown exception during reload"
                  << std::endl;
      }
    }
  }

  std::cout << Formatter::info << "Hot reload watcher stopped." << std::endl;
}

// Configuration declaration
bool config_declare() {
  Formatter::err = "[> Phoenix [Core] > Error]: ";
  Formatter::info = "[> Phoenix [Core] > Info]: ";

  auto config = wpc_loader("config/core/server.wpc");
  if (config.empty()) {
    std::cerr << Formatter::err << "Failed to load configuration\n";
    return false;
  }

  Config::root = config["Document_Root"];
  Config::port = config["Server_Port"];
  Config::threads = config["Number_Threads"];
  Config::keep_alive = config["Keep_Alive"];

  if (Config::root.empty() || Config::port.empty() || Config::threads.empty() ||
      Config::keep_alive.empty()) {
    std::cerr << Formatter::err << "Missing required configuration\n";
    return false;
  }

  return true;
}

// Library loading and route registration
bool library_loader_and_routes_register() {
  SharedData SD;
  SD.context = Config::context;
  SD.callbacks = Config::callbacks;

  std::string original_so = "./libroutes.so";
  std::string temp_dll =
      "./libroutes-temp-" +
      std::to_string(
          std::chrono::system_clock::now().time_since_epoch().count()) +
      ".so";

  try {
    std::filesystem::copy_file(
        original_so, temp_dll,
        std::filesystem::copy_options::overwrite_existing);
  } catch (const std::exception &e) {
    std::cerr << Formatter::err
              << "Failed to copy library for initial load: " << e.what()
              << std::endl;
    return false;
  }

  void *handle = dlopen(temp_dll.c_str(), RTLD_NOW | RTLD_LOCAL);
  if (!handle) {
    std::cerr << Formatter::err << "Failed to open library: " << dlerror()
              << std::endl;
    try {
      std::filesystem::remove(temp_dll);
    } catch (...) {
    }
    return false;
  }

  current_routes_handle = handle;
  current_temp_so_path = temp_dll;

  dlerror();

  setter_t trigger_setter = (setter_t)dlsym(handle, "routes_update");
  const char *error = dlerror();
  if (error) {
    std::cerr << Formatter::err << "Error Loading Function: " << error
              << std::endl;
    dlclose(handle);
    current_routes_handle = nullptr;
    try {
      std::filesystem::remove(temp_dll);
    } catch (...) {
    }
    return false;
  }

  trigger_setter(&SD);

  return true;
}

bool engine_start() {
  if (engine_running) {
    std::cout << Formatter::info << "Engine is already running." << std::endl;
    return true;
  }

  if (!config_declare()) {
    std::cerr << Formatter::err << "Error At Config Declarative" << std::endl;
    return false;
  }

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

  std::cout << Formatter::info << "Starting server with configuration:\n";
  for (int i = 0; options[i] != nullptr; i += 2) {
    std::cout << Formatter::info << options[i] << ": " << options[i + 1]
              << "\n";
  }

  {
    std::lock_guard<std::mutex> lock(context_mutex);
    Config::context = mg_start(&Config::callbacks, nullptr, options);

    if (!Config::context) {
      std::cerr << Formatter::err
                << "Failed to start server: " << strerror(errno) << "\n";
      return false;
    }
  }

  std::cout << Formatter::info
            << "Server started successfully on http://localhost:"
            << Config::port << std::endl;

  if (!library_loader_and_routes_register()) {
    std::cerr << Formatter::err << "Failed to load routes library" << std::endl;
    std::lock_guard<std::mutex> lock(context_mutex);
    mg_stop(Config::context);
    Config::context = nullptr;
    return false;
  }

  shutdown_requested = false;
  engine_running = true;

  std::thread hot_reload_thread([]() { hot_reload("main/routes/"); });
  hot_reload_thread.detach();

  return true;
}

void engine_stop() {
  if (!engine_running)
    return;

  std::cout << Formatter::info << "Shutting down engine..." << std::endl;

  shutdown_requested = true;
  engine_running = false;

  // Wait for hot reload thread to exit
  std::this_thread::sleep_for(std::chrono::milliseconds(1500));

  // Stop server context first
  {
    std::lock_guard<std::mutex> lock(context_mutex);
    if (Config::context != nullptr) {
      mg_stop(Config::context);
      Config::context = nullptr;
    }
  }

  // Give server threads time to finish before shutting down thread pool
  std::this_thread::sleep_for(std::chrono::milliseconds(300));

  // Now it's safe to shutdown the thread pool
  Server.method.shutdown();

  if (current_routes_handle) {
    dlclose(current_routes_handle);
    current_routes_handle = nullptr;
  }

  if (!current_temp_so_path.empty()) {
    try {
      std::filesystem::remove(current_temp_so_path);
    } catch (...) {
    }
    current_temp_so_path.clear();
  }

  clean_old_temp_dlls();

  std::cout << Formatter::info << "Engine stopped successfully." << std::endl;
}
