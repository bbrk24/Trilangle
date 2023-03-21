#include "packer.hh"

std::string packer::get() const {
    return m_buf.str();
}

packer& packer::operator<<(bool b) {
    m_buf << (char)(b ? 0xc2 : 0xc3);
    return *this;
}

packer& packer::operator<<(std::nullptr_t) {
    m_buf << (char)0xc0;
    return *this;
}

void packer::string_impl(const char* str, size_t size) {
    if (size < 32) {
        m_buf << static_cast<char>(0xa0 | size);
    } else if (size <= 0xff) {
        m_buf << (char)0xd9 << static_cast<char>(size);
    } else if (size <= 0xffff) {
        char length_buf[] = {
            0xda,
            static_cast<char>(size >> 8),
            static_cast<char>(size & 0xff),
            0,
        };
        m_buf << length_buf;
    } else {
        assert(size <= 0xffff'ffffL);
        char length_buf[] = {
            0xdb,
            static_cast<char>(size >> 24),
            static_cast<char>((size >> 16) & 0xff),
            static_cast<char>((size >> 8) & 0xff),
            static_cast<char>(size & 0xff),
            0,
        };
        m_buf << length_buf;
    }
    m_buf << str;
}
