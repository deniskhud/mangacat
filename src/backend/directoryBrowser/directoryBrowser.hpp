#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace Backend {

struct DirectoryEntry {
    std::filesystem::path path;
    std::string name;
    bool isDirectory = false;
    bool containsImages = false;
};

class DirectoryBrowser final {
public:
    DirectoryBrowser()
        : currentPath(std::filesystem::current_path()) {}

    [[nodiscard]] const std::filesystem::path& getCurrentPath() const noexcept {
        return currentPath;
    }

    [[nodiscard]] const std::vector<DirectoryEntry>& getEntries() const noexcept {
        return entries;
    }

private:
    std::filesystem::path currentPath;
    std::vector<DirectoryEntry> entries;
};

} // namespace Backend
