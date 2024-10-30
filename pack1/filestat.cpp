#include <iostream>
#include <filesystem>
#include <map>

namespace fs = std::filesystem;

int main() {

    std::map<std::string, int> file_types;
    fs::path current_dir = fs::current_path();

    for (const auto& entry : fs::directory_iterator(current_dir)) {

        std::cout << "name: " << entry.path().filename().string() << std::endl;

        if (fs::symlink_status(entry.path()).type() == fs::file_type::symlink) {
            file_types["symlink"]++;
            continue;
        }

        switch (fs::status(entry.path()).type())
        {
            case fs::file_type::none:
                file_types["has no type"]++;
                break;
            case fs::file_type::regular:
                file_types["regular file"]++;
                break;
            case fs::file_type::directory:
                file_types["directory"]++;
                break;
            case fs::file_type::block:
                file_types["block device"]++;
                break;
            case fs::file_type::character:
                file_types["character device"]++;
                break;
            case fs::file_type::fifo:
                file_types["named IPC pipe"]++;
                break;
            case fs::file_type::socket:
                file_types["named IPC socket"]++;
                break;
            case fs::file_type::unknown:
                file_types["unknown type"]++;
                break;
            default:
                file_types["implementation-defined type"]++;
                break;
        }

    }

    std::cout << "type of files in current dir:\n";

    for (const auto& [type, count] : file_types) {
        std::cout << type << ": " << count << "\n";
    }

    return 0;
}
