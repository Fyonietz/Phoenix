#include "../core/starter.hpp"
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <filesystem>
#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else   
    #define EXPORT
#endif
extern "C" struct EXPORT Pnix{
std::string copy_block(std::string layout, std::string tag) {
    std::ifstream file(layout);
    if (!file.is_open()) {
        std::cerr << "Cannot open layout file: " << layout << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    std::string tag_start = "<!-- " + tag + " -->";
    std::string tag_end = "<!-- @end -->";

    size_t start = content.find(tag_start);
    if (start == std::string::npos) return "";

    start = content.find('\n', start); // skip the tag line
    if (start == std::string::npos) return "";

    size_t end = content.find(tag_end, start);
    if (end == std::string::npos) return "";

    return content.substr(start + 1, end - start - 1);
}

    void insert_block(const std::string& child_path, const std::string& tag) {
    std::ifstream file(child_path);
    if (!file.is_open()) {
        std::cerr << "Cannot open child file: " << child_path << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    std::string layout_block = copy_block("public/layout.html", tag);

    size_t tag_pos = 0;
    while ((tag_pos = content.find(tag, tag_pos)) != std::string::npos) {
        size_t insert_start = content.find('\n', tag_pos);
        if (insert_start == std::string::npos) break;
        insert_start += 1;

        // Find the end of the inserted block
        size_t insert_end = insert_start;

        // Keep going until next @tag or end of file or empty line
        while (insert_end < content.size()) {
            size_t next_line_end = content.find('\n', insert_end);
            if (next_line_end == std::string::npos) break;

            std::string line = content.substr(insert_end, next_line_end - insert_end);

            std::string trimmed = line;
            trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
            trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);

            if (trimmed.rfind("@", 0) == 0 || trimmed.empty()) {
                break; // Next tag or empty line — end of current inserted block
            }

            insert_end = next_line_end + 1;
        }

        // Replace existing block with layout block
        content.replace(insert_start, insert_end - insert_start, layout_block + "\n");

        tag_pos = insert_start + layout_block.length();
    }

    std::ofstream out(child_path);
    if (!out.is_open()) {
        std::cerr << "Cannot write to child file: " << child_path << std::endl;
        return;
    }

    out << content;
    out.close();
}


    std::string erase_block(const std::string& filepath, const std::string& tag) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filepath << '\n';
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    std::istringstream stream(content);
    std::ostringstream output;
    std::string line;

    while (std::getline(stream, line)) {
        std::string trimmed = line;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n")); // Trim left
        trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1); // Trim right

        if (trimmed == tag || trimmed == "@end") {
            continue; // Skip the tag line and @end line
        }

        output << line << '\n';
    }

    return output.str();
}
std::string commentify_tags(const std::string& content, const std::vector<std::string>& tags) {
    std::istringstream stream(content);
    std::ostringstream output;
    std::string line;

    for (const std::string& tag : tags) {
        if (tag.empty()) continue;
    }

    while (std::getline(stream, line)) {
        std::string trimmed = line;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
        trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);

        bool is_custom_tag = false;
        for (const std::string& tag : tags) {
            if (trimmed == tag || trimmed == "@end") {
                is_custom_tag = true;
                break;
            }
        }

        if (is_custom_tag) {
            output << "<!-- " << trimmed << " -->\n"; // commentify it
        } else {
            output << line << '\n';
        }
    }

    return output.str();
}




    void static_serve(const std::string path,struct mg_connection *connection){

    std::ifstream file(path);
    if (!file) {
        // File not found, return 404
        const char *msg = "404 Not Found";
        mg_printf(connection,
                  "HTTP/1.1 404 Not Found\r\n"
                  "Content-Type: text/plain\r\n"
                  "Content-Length: %zu\r\n\r\n%s",
                  strlen(msg), msg);
        return;
    }

    // Read the entire file into a string
    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // Send the HTTP headers and the HTML content
    mg_printf(connection,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/html\r\n"
              "Content-Length: %zu\r\n\r\n%s",
              content.length(), content.c_str());
    }
    void home(std::string path, struct mg_connection *connection){
         std::fstream syntax("public/syntax.list");
    std::vector<std::string> tag_list_vect;
    std::string tag_line;

    if (syntax.is_open()) {
        while (std::getline(syntax, tag_line)) {
            tag_list_vect.push_back(tag_line);
        }
    } else {
        std::cerr << "File may not exist: syntax.list" << std::endl;
        return;
    }

    // Make sure layout exists
    std::ifstream file_layout("public/layout.html");
    if (!file_layout) {
        std::cerr << "> Phoenix[Info]: Error: Layout.html not found" << std::endl;
        return;
    }

    // Load target file into string
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
    file.close();

    content = commentify_tags(content, tag_list_vect);

    // Send the HTTP response
    mg_printf(connection,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/html\r\n"
              "Content-Length: %zu\r\n\r\n%s",
              content.length(), content.c_str());
    }
     void SSR(std::string path, struct mg_connection *connection) {
    std::fstream syntax("public/syntax.list");
    std::vector<std::string> tag_list_vect;
    std::string tag_line;

    if (syntax.is_open()) {
        while (std::getline(syntax, tag_line)) {
            tag_list_vect.push_back(tag_line);
        }
    } else {
        std::cerr << "File may not exist: syntax.list" << std::endl;
        return;
    }

    // Make sure layout exists
    std::ifstream file_layout("public/layout.html");
    if (!file_layout) {
        std::cerr << "> Phoenix[Info]: Error: Layout.html not found" << std::endl;
        return;
    }

    // Load target file into string
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
    file.close();

    // // Perform block insertion and commentify tags
    for (const auto& for_tag : tag_list_vect) {
        copy_block("public/layout.html", for_tag); 
        insert_block(path, for_tag);

        // Re-load updated content after insertion
        std::ifstream updated_file(path);
        std::stringstream updated_buffer;
        updated_buffer << updated_file.rdbuf();
        content = updated_buffer.str();
  }
  

    content = commentify_tags(content, tag_list_vect);

    // Send the HTTP response
    mg_printf(connection,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/html\r\n"
              "Content-Length: %zu\r\n\r\n%s",
              content.length(), content.c_str());
}

};
