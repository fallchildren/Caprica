#pragma once

#include <common/ByteSwap.h>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <ctime>

namespace caprica {

struct CapricaBinaryReader {
  Endianness endianness { Endianness::Little };
  explicit CapricaBinaryReader(const std::string& file) : strm(file, std::ifstream::binary) {
    strm.exceptions(std::ifstream::badbit | std::ifstream::failbit);
  }
  CapricaBinaryReader(const CapricaBinaryReader&) = delete;
  ~CapricaBinaryReader() = default;

  bool eof() {
    strm.peek();
    return strm.eof();
  }

  template <typename T>
  T read() {
    if constexpr (std::is_same_v<T, std::string>) {
      auto len = read<uint16_t>();
      std::unique_ptr<char[]> buf(new char[len]);
      strm.read(buf.get(), len);
      return std::string(buf.get(), buf.get() + len);
    } else if constexpr (std::is_same_v<T, float>) {
      float val;
      strm.read((char*)&val, sizeof(val));
      return endianness == Endianness::Little ? val : byteswap_float(val);
    } else if constexpr (std::is_same_v<T, int8_t> || std::is_same_v<T, uint8_t> || std::is_same_v<T, int16_t> ||
                         std::is_same_v<T, uint16_t> || std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t> ||
                         std::is_same_v<T, time_t>) {
      if constexpr (std::is_same_v<T, time_t>)
        static_assert(sizeof(time_t) == 8, "time_t is not 64 bits");
      T val {};
      strm.read((char*)&val, sizeof(val));
      return endianness == Endianness::Little ? val : byteswap(val);
    } else {
      static_assert(std::is_same_v<T, void>, "Invalid type passed to read!");
    }
  }

protected:
  std::ifstream strm;
};

}
