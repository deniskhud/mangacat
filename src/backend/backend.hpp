#pragma once
#include <filesystem>
#include <vector>
#include <deque>
namespace fs = std::filesystem;
/*TODO поддержка смены директории, маленький файловый менеджер(передвигаться по директориям)*/
/*TODO поддержка отрытия директории по пути */
#include "imageData.hpp"
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
        fs::path currentPath;
        std::vector<fs::path> pages;
        size_t currentPage = 0;
        void changeDirectory(const fs::path& newPath);
        void getImagesFromDirectory();

        std::deque<std::shared_ptr<ImageData>> deque;

        void debugM();
    public:
        void moveRight() {
            if (currentPage == pages.size()) return;
            currentPage++;
        }
        void moveLeft() {
            if (currentPage == 0) return;
            currentPage--;
        }

        [[nodiscard]]std::shared_ptr<ImageData> getImageData() {
            return deque[2];
        }
        [[nodiscard]] std::string getPageByIndex(size_t index) const;

        void stepRight();
        void stepLeft();

        DirectoryLoader(const fs::path& cpath);
    };
}
