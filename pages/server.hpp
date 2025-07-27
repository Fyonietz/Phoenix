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
    std::string copy_block(std::string layout, std::string tag){
        std::fstream file(layout);
    if (!file.is_open()) {
        std::cerr << "Cannot open layout file: " << layout << std::endl;
        return "";
    }

    std::stringstream file_buffer;
    file_buffer << file.rdbuf();
    std::string file_buffer_string(file_buffer.str());

    size_t scan_begin = file_buffer_string.find(tag);
    size_t scan_end = file_buffer_string.find("@end", scan_begin);

    std::string matching = file_buffer_string.substr(
        scan_begin + tag.length(),
        scan_end - (scan_begin + tag.length())
    );

    return matching;
};
    void insert_block(std::string child, std::string tag) {
    std::ifstream file_input(child);
    if (!file_input.is_open()) {
        std::cerr << "Cannot open child file: " << child << std::endl;
        return;
    }

    std::stringstream file_input_stream;
    file_input_stream << file_input.rdbuf();
    std::string file_input_str = file_input_stream.str();
    file_input.close();

    std::string insert = copy_block("public/layout.html", tag);
    if (insert.empty()) return;

    size_t scan_begin = 0;
    while ((scan_begin = file_input_str.find(tag, scan_begin)) != std::string::npos) {
        size_t line_after_tag = file_input_str.find('\n', scan_begin);
        if (line_after_tag == std::string::npos) break;

        size_t next_tag = file_input_str.find(tag, scan_begin + 1);
        std::string after_tag = file_input_str.substr(line_after_tag + 1, insert.length());

        if (after_tag.find(insert) != 0) {
            file_input_str.insert(line_after_tag + 1, insert + "\n");
            scan_begin = line_after_tag + 1 + insert.length(); 
        } else {
            scan_begin = line_after_tag + 1 + insert.length(); 
        }
    }

    std::ofstream file_output(child);
    file_output << file_input_str;
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

    // Perform block insertion and commentify tags
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
