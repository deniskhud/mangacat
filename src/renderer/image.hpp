#ifndef IMAGE_HPP
#define IMAGE_HPP
#include <optional>
#include <string>
#include <vector>

#include "../backend/ImageData.hpp"
class Image {
private:
    std::vector<std::uint8_t> pixels;
    int width, height;
    // Масштабировать под размер
    Image resize(int target_w, int target_h) const;
    std::string path{};

    int x = 0, y = 0;
public:
    void show();
    Image(const std::string& path);
    void render(const ImageData& img);
};
static const char b64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
std::string base64_encode(const uint8_t* data, size_t len);

void kitty_clear();
void kitty_show(const uint8_t* pixels, int w, int h, int x, int y);

#endif //IMAGE_HPP