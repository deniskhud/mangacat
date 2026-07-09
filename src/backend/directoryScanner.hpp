#pragma once
#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace Backend {

/**
 * DirectoryScanner — сканирует директорию и возвращает
 * отсортированный список путей к картинкам.
 *
 * Не загружает пиксели, не знает о кеше — только fs::.
 *
 * Использование:
 *   DirectoryScanner scanner("/path/to/dir");
 *   const auto& pages = scanner.pages();  // отсортированный вектор
 *   scanner.rescan();                      // перечитать с диска
 *   scanner.change("/other/dir");          // сменить директорию
 */
class DirectoryScanner {
public:
    // Поддерживаемые расширения (регистронезависимо через lowercase)

    const std::vector<std::string> extensions {
        ".jpg", ".jpeg", ".png", ".webp"
    };

    explicit DirectoryScanner(const fs::path& path) {
        changeDirectory(path);
    }

    // Сменить директорию и сразу пересканировать
    void changeDirectory(const fs::path& path);

    // Перечитать текущую директорию с диска
    void scanDirectory();

    /** getters **/
    [[nodiscard]] const std::vector<fs::path>& getPages() const { return pages; }
    [[nodiscard]] const fs::path& getCurrentPath() const { return currentPath; }
    size_t getSize() const { return pages.size(); }
    bool isEmpty() const { return pages.empty(); }

private:
    bool isFileFormatSupported(const fs::path& p);

    void sortPages() {
        std::sort(pages.begin(), pages.end(), [](const fs::path& a, const fs::path& b) {
            try {
                return std::stoi(a.stem().string()) < std::stoi(b.stem().string());
            } catch (...) {
                return a < b;
            }
        });
    }
    /* full directory path */
    fs::path currentPath;
    /* array of path pictures */
    std::vector<fs::path> pages;
};
} // namespace Backend
