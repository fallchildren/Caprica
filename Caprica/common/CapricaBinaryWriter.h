#pragma once

#include <cassert>
#include <cstdint>
#include <limits>
#include <string_view>
#include <type_traits>
#include <cstring>
#include <ctime>

#include <common/allocators/ChainedPool.h>
#include <common/ByteSwap.h>
#include <common/FSUtils.h>

namespace caprica {

struct CapricaBinaryWriter {
  Endianness endianness { Endianness::Little };
  explicit CapricaBinaryWriter() = default;
  CapricaBinaryWriter(const CapricaBinaryWriter&) = delete;
  ~CapricaBinaryWriter() = default;

  template <typename F>
  void applyToBuffers(F&& func) {
    for (auto beg = strm.begin(), end = strm.end(); beg != end; ++beg)
      func(beg.data(), beg.size());
  }

  template <typename T>
  void boundWrite(size_t val) {
    assert(val <= std::numeric_limits<T>::max());
    write<T>((T)val);
  }

  template <typename T>
  void write(T val) {
    using U = std::remove_cv_t<std::remove_reference_t<T>>;
    if constexpr (std::is_same_v<U, int8_t> || std::is_same_v<U, uint8_t>) {
      strm.make<U>(val);
    } else if constexpr (std::is_same_v<U, int16_t> || std::is_same_v<U, uint16_t> ||
                         std::is_same_v<U, int32_t> || std::is_same_v<U, uint32_t>) {
      strm.make<U>(endianness == Endianness::Little ? val : byteswap(val));
    } else if constexpr (std::is_same_v<U, float>) {
      strm.make<float>(endianness == Endianness::Little ? val : byteswap_float(val));
    } else if constexpr (std::is_same_v<U, time_t>) {
      static_assert(sizeof(time_t) == 8, "time_t is not 64 bits");
      strm.make<time_t>(endianness == Endianness::Little ? val : byteswap(val));
    } else if constexpr (std::is_same_v<U, std::string_view> || std::is_same_v<U, identifier_ref>) {
      boundWrite<uint16_t>(val.size());
      if (val.size())
        append(val.data(), val.size());
    } else {
      static_assert(std::is_same_v<U, void>, "Invalid type passed to write!");
    }
  }

protected:
  allocators::ChainedPool strm { 1024 * 4 };

  void append(const char* __restrict a, size_t size) {
    // ChainedPool will re-order large allocations, so disallow them.
    assert(size < 4096);
    memcpy(strm.allocate(size), a, size);
  }
};

}
