#ifndef __LIB_H__
#define __LIB_H__

#include <types.h>

/**
 * @brief counts the length of a null-terminated string.
 * 
 * @param str the starting address
 * @return the length of the string
 */
size_t strlen(const char *str);

/**
 * @brief copies a string from source to destination.
 * 
 * @param dest the destination address
 * @param src the source address
 * @return the destination address
 */
char *strcpy(char *dest, const char *src);

/**
 * @brief copies a string from source to destination with a limit
 * 
 * @param dest the destination address
 * @param src the source address
 * @param n the maximum number of characters to copy
 * @return the destination address
 */
char *strncpy(char *dest, const char *src, size_t n);

/**
 * @brief compares two strings.
 * 
 * @param str1 the first string
 * @param str2 the second string
 * @return 0 if the strings are equal, otherwise the difference between the first differing characters
 */
int strcmp(const char *str1, const char *str2);

/**
 * @brief compares two strings with a limit.
 * 
 * @param str1 the first string
 * @param str2 the second string
 * @param n the maximum number of characters to compare
 * @return 0 if the strings are equal, otherwise the difference between the first differing characters
 */
int strncmp(const char *str1, const char *str2, size_t n);

/**
 * @brief finds the first occurrence of a character in a string.
 * 
 * @param str the string
 * @param c the character
 * @return the address of the first occurrence of the character, or NULL if not found
 */
char *strchr(const char *str, int c);

/**
 * @brief finds the last occurrence of a character in a string.
 * 
 * @param str the string
 * @param c the character
 * @return the address of the last occurrence of the character, or NULL if not found
 */
char *strrchr(const char *str, int c);

/**
 * @brief finds the first occurrence of a substring in a string.
 * 
 * @param haystack the string
 * @param needle the substring
 * @return the address of the first occurrence of the substring, or NULL if not found
 */
char *strstr(const char *haystack, const char *needle);

#endif
