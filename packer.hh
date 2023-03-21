#pragma once

#include <cstring>
#include <sstream>
#include <type_traits>
#include "compat.hh"

// A msgpack encoder. Supports operator<< similar to std::ostream.
// Doesn't support float, array, bin, or timestamp types.
class packer {
public:
    // Create a new packer with an empty buffer.
    packer() = default;

    // Create a new packer with the given initial buffer contents.
    packer(const std::string& contents) : m_buf(contents) {}

    // Retrieve the underlying buffer.
    std::string get() const;

    template<
        typename T,
        std::enable_if_t<std::is_integral<T>::value && !std::is_same<T, bool>::value, std::nullptr_t> = nullptr>
    packer& operator<<(T val) {
        // Handle small numbers as fixints
        if (val >= 0 && val < 128) {
            m_buf << static_cast<char>(val);
            return *this;
        }
        if (val < 0 && val >= -32) {
            m_buf << static_cast<char>(0xe0 | (val & 0x1f));
            return *this;
        }
        // Determine whether it's a signed or unsigned type
        IF_CONSTEXPR (std::numeric_limits<T>::min() < 0) {
            // msgpack is big-endian, so I need to use bitwise operators to reverse large numbers rather than copying
            // the bytes literally
            if (val <= 0x7f && val >= -0x80) {
                m_buf << (char)0xd0 << static_cast<char>(val);
                return *this;
            }
            if (val <= 0x7fff && val >= -0x8000L) {
                char num_buf[] = { 0xd1, static_cast<char>(val >> 8), static_cast<char>(val & 0xff), 0 };
                m_buf << num_buf;
                return *this;
            }
            if (val <= 0x7fff'ffffL && val >= -0x8000'0000LL) {
                char num_buf[] = {
                    0xd2,
                    static_cast<char>(val >> 24),
                    static_cast<char>((val >> 16) & 0xff),
                    static_cast<char>((val >> 8) & 0xff),
                    static_cast<char>(val & 0xff),
                    0,
                };
                m_buf << num_buf;
                return *this;
            }
            char num_buf[] = {
                0xd3,
                static_cast<char>(val >> 56),
                static_cast<char>((val >> 48) & 0xff),
                static_cast<char>((val >> 40) & 0xff),
                static_cast<char>((val >> 32) & 0xff),
                static_cast<char>((val >> 24) & 0xff),
                static_cast<char>((val >> 16) & 0xff),
                static_cast<char>((val >> 8) & 0xff),
                static_cast<char>(val & 0xff),
                0,
            };
            m_buf << num_buf;
        } else {
            if (val <= 0xff) {
                m_buf << (char)0xcc << static_cast<char>(val);
                return *this;
            }
            if (val <= 0xffff) {
                char num_buf[] = { 0xcd, static_cast<char>(val >> 8), static_cast<char>(val & 0xff), 0 };
                m_buf << num_buf;
                return *this;
            }
            if (val <= 0xffff'ffffL) {
                char num_buf[] = {
                    0xce,
                    static_cast<char>(val >> 24),
                    static_cast<char>((val >> 16) & 0xff),
                    static_cast<char>((val >> 8) & 0xff),
                    static_cast<char>(val & 0xff),
                    0,
                };
                m_buf << num_buf;
                return *this;
            }
            char num_buf[] = {
                0xce,
                static_cast<char>(val >> 56),
                static_cast<char>((val >> 48) & 0xff),
                static_cast<char>((val >> 40) & 0xff),
                static_cast<char>((val >> 32) & 0xff),
                static_cast<char>((val >> 24) & 0xff),
                static_cast<char>((val >> 16) & 0xff),
                static_cast<char>((val >> 8) & 0xff),
                static_cast<char>(val & 0xff),
                0,
            };
            m_buf << num_buf;
        }
        return *this;
    }

    inline packer& operator<<(const std::string& str) {
        string_impl(str.c_str(), str.size());
        return *this;
    }

    inline packer& operator<<(const char* c) {
        if (c == nullptr) {
            return *this << nullptr;
        } else {
            string_impl(c, strlen(c));
            return *this;
        }
    }

    packer& operator<<(bool b);
    packer& operator<<(std::nullptr_t);
private:
    void string_impl(const char* str, size_t size);

    std::ostringstream m_buf;
};
