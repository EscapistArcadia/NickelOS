#include <lib.h>

size_t strlen(const char *str) {
    size_t len = 0;
    for (; *str; ++str, ++len);
    return len;
}

char *strcpy(char *dest, const char *src) {
    char *pos = dest;
    for (; *src; ++src, ++pos) {
        *pos = *src;
    }
    *pos = '\0';
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
    char *pos = dest;
    for (; *src && n; ++src, ++pos, --n) {
        *pos = *src;
    }
    *pos = '\0';
    return dest;
}

int strcmp(const char *str1, const char *str2) {
    for (; *str1 && *str2; ++str1, ++str2) {
        if (*str1 != *str2) {
            return *str1 - *str2;
        }
    }
    return *str1 - *str2;
}

int strncmp(const char *str1, const char *str2, size_t n) {
    for (; *str1 && *str2 && n; ++str1, ++str2, --n) {
        if (*str1 != *str2) {
            return *str1 - *str2;
        }
    }
    return n ? *str1 - *str2 : 0;
}

char *strchr(const char *str, int c) {
    for (; *str; ++str) {
        if (*str == c) {
            return (char *)str;
        }
    }
    return NULL;
}

char *strrchr(const char *str, int c) {
    const char *pos = NULL;
    for (; *str; ++str) {
        if (*str == c) {
            pos = str;
        }
    }
    return (char *)pos;
}
