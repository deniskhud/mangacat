#ifndef IMAGEDATA_HPP
#define IMAGEDATA_HPP
#include <cstdint>
#include <string>
#include <vector>
#include "../cli/cli.hpp"
struct ImageLayout {
    int out_w, out_h;
    int x, y;
    std::vector<uint8_t> resized;

    static ImageLayout compute(const uint8_t* pixels, int img_w, int img_h);
};

struct Chunk {
    std::string prefix;
    std::string data;
};

struct ImageData {
    std::string path;
    ImageLayout layout;          // уже посчитанный layout
    std::vector<Chunk> chunks;
    bool loaded = false;
};










#endif //IMAGEDATA_HPP
