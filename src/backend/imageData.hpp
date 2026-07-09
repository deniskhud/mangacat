#pragma once
#include "../cli/cli.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace Cli { struct TermSize; }
/* contains image coords, size */
struct ImageLayout {
  std::uint32_t layoutWidth = 0, layoutHeight = 0;
  std::uint32_t x = 0, y = 0;
  std::vector<uint8_t> resized;
  std::uint32_t imageCols = 0, imageRows = 0;

  // reserved_rows — строки занятые UI (StatusBar сверху, filmstrip снизу и т.д.)
  static ImageLayout computeImageLayout(const uint8_t* pixels, int imageWidth, int imageHeight, const Cli::TermSize& term, std::uint32_t reservedRows = 1);
};

struct Chunk {
  std::string prefix;
  std::string data;
};

struct ImageData {
  ImageLayout layout;
  std::vector<Chunk> chunks;
  bool loaded{false};
};
