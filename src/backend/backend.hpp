#ifndef BACKEND_HPP
#define BACKEND_HPP
#include <filesystem>
#include <vector>
#include <deque>
namespace fs = std::filesystem;
/*TODO поддержка смены директории, маленький файловый менеджер(передвигаться по директориям)*/
/*TODO поддержка отрытия директории по пути */
#include "ImageData.hpp"
#include <future>
/*короче фотки будем хранить не все, будем хранить текущую и рядом с ней типо так   - - - * - - -
 *делаем связный список, храним указатели на вектор байтов
 *
 *
 */
namespace Backend {
    /*
        *Очень важный апгрейд
        Делай кеш:
        unordered_map<path, RenderImage>
        чтобы не пересобирать base64 каждый раз
         *
         *
    */
    /** навигатор по директориям
     * класс, с которым мы сможем переходить по директориям, текущую он отдаст на навигацию
     */
    class DirectoryLoader {
    private:
        fs::path current_path;
        std::vector<fs::path> pages;
        size_t current_page = 0;
        void change_directory(const fs::path& new_path);
        void get_images_from_directory();

        std::deque<std::shared_ptr<ImageData>> deque_;

        void debug_m();
    public:
        void move_right() {
            if (current_page == pages.size()) return;
            current_page++;
        }
        void move_left() {
            if (current_page == 0) return;
            current_page--;
        }

        [[nodiscard]]std::shared_ptr<ImageData> get_image_data() {
            return deque_[2];
        }
        [[nodiscard]] std::string get_page_by_index(size_t index) const;

        void step_right();
        void step_left();

        DirectoryLoader(const fs::path& cpath);
    };

    class DirectoryNavigator {
    private:

    public:
    };

    //загружаем картинку
    /*class ImageLoader {
    private:
        std::string base64_encode(const uint8_t* data, size_t len);
        const char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::shared_ptr<ImageData> image;
        void load_image(const fs::path& p);
    public:
        ImageLoader(const std::string_view& path);
        std::shared_ptr<ImageData> get_image_data();
    };*/

}

#endif //BACKEND_HPP