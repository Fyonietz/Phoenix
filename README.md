# Phoenix 🚀

**A high-performance, lightweight MVC web framework for C++**

Phoenix is a **fast, modular, and developer-friendly** web framework built in **C++**, designed for those who value both **control and performance**. It brings the familiar **Model–View–Controller (MVC)** structure to native C++ web development — making it easier to organize your code, serve static content, and build dynamic web applications with minimal overhead.

---

## ✨ Features

* ⚡ **Blazing Fast** — Written in C++ for maximum performance and efficiency.
* 🔁 **Hot Reloading** — Instantly reload routes and logic without restarting the server.
* 🧩 **MVC Structure** — Clean separation of concerns for maintainable, scalable projects.
* 📁 **Static File Support** — Serve static assets directly from your `main/public` directory.
* 🧠 **Lightweight Core** — Minimal dependencies, focused on simplicity and speed.

---

## 🏗️ Project Structure

```
Phoenix/
├── build/              # Compiled binaries and build artifacts
├── CMakeLists.txt      # Build configuration
├── config/             # Framework and application configuration files
├── core/               # Phoenix core engine (framework internals)
├── libroutes.so        # Shared library for dynamic route handling (supports hot reload)
├── main/               # Your project/application source
│   ├── model/          # Data models
│   ├── public/         # Static assets (CSS, JS, images, etc.)
│   └── routes/         # Endpoint logic and route definitions
├── Phoenix*            # Framework executable

---

## ⚙️ Building Phoenix

### Prerequisites

* **CMake** (≥ 3.16)
* **C++17** or newer compiler
* **Make** or another build toolchain

### Build Steps

```bash
# Clone the repository
git clone https://github.com/Fyonietz/Phoenix.git
cd Phoenix

# Configure and build
cmake -B build
cmake --build build

# Run Phoenix
./Phoenix
```

---

## 🧱 Creating a Project

Inside the `main/` directory, you’ll find the default structure for your web app:

* `model/` — Define your data structures and logic.
* `routes/` — Create route handlers for your API endpoints.
* `public/` — Add static files such as HTML, CSS, and JavaScript.

Phoenix automatically maps routes and serves files according to your configuration in `pyro.conf`.

---

## 🔥 Hot Reload

Phoenix supports **dynamic route reloading** via `libroutes.so`.
You can modify route logic without restarting the server — ideal for rapid development.

---

## 🧩 Configuration

Edit the `config/core/server.wpc` file to define:

* Server port and address
* Route file paths

Example:

```ini

Document_Root=main/public
Server_Port=9000
Number_Threads=64
Keep_Alive=yes
```

---

---

## 🧩 Env

Edit the `config/main/app.wpc` file to define:

* Database Server port and address
* API KEY

Example:

```ini

env=100
API_KEY=onehundred

Server.env()["API_KEY"];<-onehundred

```

---

## 🛠️ Example Usage

```cpp
// main/routes/example.cpp
#include <phoenix.h>

route("/api", api) {
  Server.Response(connection, 200, "OK",
                  R"({"Messages":"Hello From Phoenix"})");
  return 200;
};

route("/api/env", api_list) {
  std::string messages = R"({"Messages": "Hello From Env", "Env": ")" +
                         Server.env()["env"] + R"("})";
  if (ASYNC) {
    auto result = Server.method.async(
        [&]() { Server.Response(connection, 200, "OK", messages.c_str()); });

    result.get();
  }

  return 200;
};
```

Compile, run, and visit:
👉 **[http://localhost:9001/](http://localhost:9001/)**

---

## 🧑‍💻 Contributing

Contributions are welcome!
Fork the repository, make your changes, and submit a pull request.
Please ensure your code is formatted and documented appropriately.

---

## 📜 License

Phoenix is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

---
