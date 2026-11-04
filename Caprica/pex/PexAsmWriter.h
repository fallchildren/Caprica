#pragma once

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>

#include <pex/PexUserFlags.h>

namespace caprica { namespace pex {

struct PexAsmWriter final {
  // The indent level. Yes, this spelling is deliberate.
  size_t ident { 0 };

  explicit PexAsmWriter(std::ostream& dest) : strm(dest) { }
  PexAsmWriter(const PexAsmWriter&) = delete;
  ~PexAsmWriter() = default;

  template <typename T>
  void writeKV(const char* key, T val) {
    ensureIndent();
    if constexpr (std::is_same_v<T, time_t>) {
      // TODO: Add a comment output of the times in the local time.
      strm << '.' << key << ' ' << (unsigned long long)val;
    } else if constexpr (std::is_same_v<T, std::string>) {
      strm << '.' << key << " \"" << escapeString(val) << "\"";
    } else if constexpr (std::is_same_v<T, std::string_view>) {
      strm << '.' << key << " \"" << escapeString(std::string(val)) << "\"";
    } else if constexpr (std::is_same_v<T, PexUserFlags>) {
      strm << '.' << key << " " << val.data;
    } else {
      static_assert(std::is_same_v<T, void>, "Unknown type for the value!");
    }
    writeln();
  }

  template <typename... Args>
  void write(std::string_view msg, Args&&... args) {
    ensureIndent();
    strm << formatString(msg, args...);
  }

  template <typename... Args>
  void writeln(std::string_view msg, Args&&... args) {
    ensureIndent();
    strm << formatString(msg, args...);
    writeln();
  }

  void writeln() {
    haveIndented = false;
    strm << std::endl;
  }

  static std::string escapeString(const std::string& str) {
    std::ostringstream dest;

    for (auto c : str) {
      switch (c) {
        case '\n':
          dest.put('\\');
          dest.put('n');
          break;
        case '\t':
          dest.put('\\');
          dest.put('t');
          break;
        case '"':
          dest.put('\\');
          dest.put('"');
          break;
        case '\\':
          dest.put('\\');
          dest.put('\\');
          break;
        default:
          dest.put(c);
          break;
      }
    }

    return dest.str();
  }

private:
  bool haveIndented { false };
  std::ostream& strm;

  void ensureIndent() {
    if (!haveIndented) {
      for (size_t i = 0; i < ident; i++)
        strm << "  ";
      haveIndented = true;
    }
  }

  template <typename... Args>
  static std::string formatString(std::string_view msg, Args&&... args) {
    constexpr bool hasArgs = sizeof...(args) != 0;
    if (hasArgs) {
      size_t size = std::snprintf(nullptr, 0, msg.data(), args...) + 1;
      std::unique_ptr<char[]> buf(new char[size]);
      std::snprintf(buf.get(), size, msg.data(), args...);
      return std::string(buf.get(), buf.get() + size - 1);
    } else {
      return std::string(msg);
    }
  }
};

}}
