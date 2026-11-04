#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <type_traits>

#include <common/CapricaBinaryReader.h>
#include <common/CapricaReportingContext.h>

#include "common/GameID.h"
#include <pex/PexString.h>
#include <pex/PexUserFlags.h>
#include <pex/PexValue.h>

namespace caprica { namespace pex {

struct PexReader final : public CapricaBinaryReader {
  explicit PexReader(const std::string& file) : CapricaBinaryReader(file) { }
  PexReader(const PexReader&) = delete;
  ~PexReader() = default;

  template <typename T>
  T read() {
    if constexpr (std::is_same_v<T, GameID>) {
      return (GameID)read<uint16_t>();
    } else if constexpr (std::is_same_v<T, PexString>) {
      PexString val;
      val.index = read<uint16_t>();
      return val;
    } else if constexpr (std::is_same_v<T, PexUserFlags>) {
      PexUserFlags val;
      val.data = read<uint32_t>();
      return val;
    } else if constexpr (std::is_same_v<T, PexValue>) {
      return readValue();
    } else {
      return CapricaBinaryReader::read<T>();
    }
  }

  PexValue readValue() {
    PexValue val;
    val.type = (PexValueType)read<uint8_t>();
    switch (val.type) {
      case PexValueType::None:
        return val;
      case PexValueType::Identifier:
      case PexValueType::String:
        val.val.s = read<PexString>();
        return val;
      case PexValueType::Integer:
        val.val.i = (int32_t)read<uint32_t>();
        return val;
      case PexValueType::Float:
        val.val.f = read<float>();
        return val;
      case PexValueType::Bool:
        val.val.b = read<uint8_t>() ? true : false;
        return val;

      case PexValueType::Label:
      case PexValueType::TemporaryVar:
      case PexValueType::Invalid:
        break;
    }
    CapricaReportingContext::logicalFatal("Unknown PexValueType!");
  }
};

}}
