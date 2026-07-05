#ifndef DIRECTORYSCANNER_HPP
#define DIRECTORYSCANNER_HPP
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

    const std::vector<std::string> EXTENSIONS {
        ".jpg", ".jpeg", ".png", ".webp"
    };

    explicit DirectoryScanner(const fs::path& path) {
        change_directory(path);
    }

    // Сменить директорию и сразу пересканировать
    void change_directory(const fs::path& path) {
        if (!fs::exists(path) || !fs::is_directory(path))
            throw std::runtime_error("Not a directory: " + path.string());
        current_path_ = path;
        scan_directory();
    }

    // Перечитать текущую директорию с диска
    void scan_directory() {
        pages_.clear();
        for (const auto& entry : fs::directory_iterator(current_path_)) {
            if (!entry.is_regular_file()) continue;
            if (is_supported(entry.path()))
                pages_.push_back(entry.path());
        }
        sortPages();
    }
    /** getters **/
    [[nodiscard]] const std::vector<fs::path>& get_pages() const { return pages_; }
    [[nodiscard]] const fs::path& get_current_path() const { return current_path_; }
    size_t get_size() const { return pages_.size(); }
    bool is_empty() const { return pages_.empty(); }

private:
    bool is_supported(const fs::path& p) {
        std::string ext = p.extension().string();
        // в lowercase для регистронезависимого сравнения
        for (char& c : ext) c = static_cast<char>(std::tolower(c));

        bool isSupported = std::ranges::any_of(EXTENSIONS, [&ext](auto& extension) {
            return ext == extension;
        });

        return isSupported;
    }

    void sortPages() {
        std::sort(pages_.begin(), pages_.end(), [](const fs::path& a, const fs::path& b) {
            try {
                return std::stoi(a.stem().string()) < std::stoi(b.stem().string());
            } catch (...) {
                return a < b;
            }
        });
    }
    /* full directory path */
    fs::path current_path_;
    /* array of path pictures */
    std::vector<fs::path> pages_;
};
} // namespace Backend
#endif //DIRECTORYSCANNER_HPP