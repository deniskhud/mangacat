#ifndef CLI_MANGA_READER_DIRECTORYBROWSER_HPP
#define CLI_MANGA_READER_DIRECTORYBROWSER_HPP
#pragma once
#include <filesystem>
#include <vector>
namespace fs =  std::filesystem;

struct DirectoryEntry {
    fs::path path;
    std::string name;
    bool isDirectory;
    bool containsImages;
};

class DirectoryBrowser final {
public:
    explicit DirectoryBrowser();

    const fs::path& getCurrentPath();




private:
    fs::path currentPath;
    std::vector<DirectoryEntry> entries;
};


#endif //CLI_MANGA_READER_DIRECTORYBROWSER_HPP