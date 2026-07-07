#include "DirectoryScanner.hpp"

void Backend::DirectoryScanner::changeDirectory(const fs::path& path) {
    if (!fs::exists(path) || !fs::is_directory(path))
        throw std::runtime_error("Not a directory: " + path.string());
    currentPath = path;
    scanDirectory();
}

void Backend::DirectoryScanner::scanDirectory() {
    pages.clear();
    for (const auto& entry : fs::directory_iterator(currentPath)) {
        if (!entry.is_regular_file()) continue;
        if (isFileFormatSupported(entry.path()))
            pages.push_back(entry.path());
    }
    sortPages();
}


bool Backend::DirectoryScanner::isFileFormatSupported(const fs::path& p) {
    std::string ext = p.extension().string();
    // в lowercase для регистронезависимого сравнения
    for (char& c : ext) c = static_cast<char>(std::tolower(c));

    bool supported = std::ranges::any_of(extensions, [&ext](auto& extension) {
        return ext == extension;
    });
    return supported;
}