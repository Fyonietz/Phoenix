#include <engine.hpp>
#include <fstream>
#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <functional>
#include <queue>
#include <condition_variable>
#include <future>
#include <cstring>
#include <sstream>
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

struct Pnix {
  // ThreadPool nested struct inside Pnix
  struct ThreadPool {
    std::vector<std::thread> workers;       // List of all threads
    std::queue<std::function<void()>> tasks; // List of all tasks
    std::mutex queueLock;                   // Protect data access
    std::condition_variable condition;      // Condition for sync
    std::atomic<bool> stop;                 // Stop flag

    explicit ThreadPool(size_t numThreads) : stop(false) {
      workers.reserve(numThreads);

      // Start worker threads
      for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this]() {
          while (true) {
            std::function<void()> task;
            {
              std::unique_lock<std::mutex> lock(queueLock);

              // Wait until there is a task or stop signal
              condition.wait(lock, [this]() { return stop || !tasks.empty(); });

              if (stop && tasks.empty()) return; // Stop if done

              task = std::move(tasks.front());
              tasks.pop();
            }
            task(); // Execute the task
          }
        });
      }
    }

    ~ThreadPool() {
      stop = true;
      condition.notify_all();

      for (auto& worker : workers) {
        worker.join();
      }
    }

    // Enqueue a new task to the pool
    template <class F, class... Args>
    auto async(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type> {
      using returnType = typename std::invoke_result<F, Args...>::type;

      auto task = std::make_shared<std::packaged_task<returnType()>>(
          std::bind(std::forward<F>(f), std::forward<Args>(args)...)
      );

      std::future<returnType> res = task->get_future();
      {
        std::unique_lock<std::mutex> lock(queueLock);

        if (stop) throw std::runtime_error("Enqueue on stopped ThreadPool");
        tasks.push([task]() { (*task)(); });
      }

      condition.notify_one();
      return res;
    }
  };

  // ThreadPool instance as a member
  ThreadPool method;

  Pnix(size_t numThreads) : method(numThreads) {}
  std::unordered_map<std::string,std::string> env(){
    auto env = wpc_loader("config/main/app.wpc");

    return env;
  }
  int Response(struct mg_connection *conn, int status_code,
               const std::string &status_text, const std::string &json_body) {
    mg_printf(conn,
              "HTTP/1.1 %d %s\r\n"
              "Content-Type: application/json\r\n"
              "Content-Length: %zu\r\n"
              "\r\n"
              "%s",
              status_code, status_text.c_str(), json_body.length(),
              json_body.c_str());

    return status_code;
  }
  
  void static_serve(const std::string path, struct mg_connection *connection) {
    std::ifstream file(path);
    if (!file) {
      const char *msg = "404 Not Found";
      mg_printf(connection,
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: %zu\r\n\r\n%s",
                strlen(msg), msg);
      return;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    mg_printf(connection,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/html\r\n"
              "Content-Length: %zu\r\n\r\n%s",
              content.length(), content.c_str());
  }

  std::string Read(struct mg_connection *connection) {
    std::string body;
    char buffer[2048];
    int bytes_read;

    const char *content_length_str =
        mg_get_header(connection, "Content-Length");
    if (content_length_str) {
      size_t content_length = std::stoul(content_length_str);
      body.reserve(content_length);
    }

    while ((bytes_read = mg_read(connection, buffer, sizeof(buffer))) > 0) {
      body.append(buffer, bytes_read);
    }

    return body;
  }

  int ResponseAsFile(struct mg_connection *conn, int status_code,
                     const std::string &status_text, std::string html_file) {
    // Check if we have an HTML file to serve
    if (!html_file.empty()) {
      std::ifstream file(html_file);
      if (file.is_open()) {
        // Read the HTML file content
        std::string html_content((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
        file.close();

        // Serve the HTML content with the corresponding status code and headers
        mg_printf(conn,
                  "HTTP/1.1 %d %s\r\n"
                  "Content-Type: text/html\r\n"
                  "Content-Length: %zu\r\n"
                  "\r\n"
                  "%s",
                  status_code, status_text.c_str(), html_content.length(),
                  html_content.c_str());
        return status_code;
      } else {
        // If the HTML file can't be opened, return a JSON response as a
        // fallback
        std::string error_message = "{\"error\": \"HTML file not found for " +
                                    std::to_string(status_code) + "\"}";
        mg_printf(conn,
                  "HTTP/1.1 %d %s\r\n"
                  "Content-Type: application/json\r\n"
                  "Content-Length: %zu\r\n"
                  "\r\n"
                  "%s",
                  status_code, status_text.c_str(), error_message.length(),
                  error_message.c_str());
        return status_code;
      }
    }

    // If no specific HTML file, return a JSON response with an error message
    std::string error_message = "{\"error\": \"Unhandled status code " +
                                std::to_string(status_code) + "\"}";
    mg_printf(conn,
              "HTTP/1.1 %d %s\r\n"
              "Content-Type: application/json\r\n"
              "Content-Length: %zu\r\n"
              "\r\n"
              "%s",
              status_code, status_text.c_str(), error_message.length(),
              error_message.c_str());

    return status_code;
  }

};
extern Pnix Server; 
