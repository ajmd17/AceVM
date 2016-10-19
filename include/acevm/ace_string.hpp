#ifndef ACE_STRING_HPP
#define ACE_STRING_HPP

#include <cstring>
#include <cassert>

template <size_t MaxSize>
class String {
public:
    String(const char *str)
    {
        std::memcpy(m_str, str, MaxSize + 1);
    }

    String(const String &other)
    {
        std::memcpy(m_str, other.m_str, MaxSize + 1);
    }

    inline char &operator[](size_t index)
    {
        return m_str[index];
    }

    inline char operator[](size_t index) const
    {
        return m_str[index];
    }

    inline bool operator==(const String &other) const
    {
        return !(std::strcmp(m_str, other.m_str));
    }

    inline bool operator!=(const String &other) const
    {
        return !(operator==(other));
    }

    inline operator const char* () const
    {
        return m_str;
    }

    inline size_t Length() const
    {
        return std::strlen(m_str);
    }

    constexpr size_t MaxLength()
    {
        return MaxSize;
    }

private:
    char m_str[MaxSize + 1];
};

#endif
