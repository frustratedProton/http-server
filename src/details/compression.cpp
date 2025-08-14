#include "compression.hpp"
#include <stdexcept>
#include <string>
#include <zconf.h>
#include <zlib.h>

std::string gzipCompress(const std::string &data) {
  z_stream zs{};
  // deflateInit2 for gzip data compression
  if (deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED,
                   15 + 16, // 15 window bits + 16 to write gzip header
                   8, Z_DEFAULT_STRATEGY))

  {
    throw std::runtime_error("deflateInit2 failed.");
  }

  zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(data.data()));
  zs.avail_in = static_cast<uInt>(data.size());

  char outBuffer[32768];
  std::string outString;

  int ret;
  do {
    zs.next_out = reinterpret_cast<Bytef *>(outBuffer);
    zs.avail_out = sizeof(outBuffer);

    ret = deflate(&zs, Z_FINISH);

    if (outString.size() < zs.total_out) {
      outString.append(outBuffer, zs.total_out - outString.size());
    }
  } while (ret == Z_OK);

  deflateEnd(&zs);

  if (ret != Z_STREAM_END) {
    throw std::runtime_error("deflate failed");
  }
  return outString;
}