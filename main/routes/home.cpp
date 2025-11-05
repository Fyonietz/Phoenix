#include <phoenix.hpp>

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
