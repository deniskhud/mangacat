#ifndef IMAGEDATA_HPP
#define IMAGEDATA_HPP
#include "../cli/cli.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace Cli { struct TermSize; }
/* contains image coords, size */
struct ImageLayout {
  int out_w = 0, out_h = 0;
  int x = 0, y = 0;
  std::vector<uint8_t> resized;

  // reserved_rows — строки занятые UI (StatusBar сверху, filmstrip снизу и т.д.)
  static ImageLayout compute(const uint8_t* pixels, int img_w, int img_h,const Cli::TermSize& term,int reserved_rows = 1);
};

struct Chunk {
  std::string prefix;
  std::string data;
};

struct ImageData {
  std::string path;
  ImageLayout layout;
  std::vector<Chunk> chunks;
  bool loaded = false;
};
#endif // IMAGEDATA_HPP
