#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    fs::path file_path = argv[1];
    fs::path dark_dir = "./dark_dir";

    try {

        if (!fs::exists(file_path)) {
            std::cerr << "Error: File does not exist.\n";
            return 1;
        }

        if (!fs::exists(dark_dir)) {
            fs::create_directory(dark_dir);
        }

        fs::permissions(
            dark_dir,
            std::filesystem::perms::owner_read | std::filesystem::perms::group_read | std::filesystem::perms:: others_read ,
            std::filesystem::perm_options::remove
        );

        fs::path new_file_path = dark_dir / file_path.filename();

        fs::rename(file_path, new_file_path);
        std::cout << "File successfully hidden in " << dark_dir << "\n";
    }

    catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
