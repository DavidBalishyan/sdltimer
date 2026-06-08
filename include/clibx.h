#ifndef CLIBX_H
#define CLIBX_H

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>
#include <limits.h>
#include <stdint.h>

/*
 * CLIBX - Minimal C Utility Library
 * --------------------------------------
 * A lightweight header-only utility library providing:
 * - Array helpers
 * - Math macros
 * - Memory utilities
 * - Debug/logging tools
 * - Type utilities
 * - String helpers
 * --------------------------------------
 * Designed for convenience and zero dependencies.
 * */

//
// String type
//

/*
 * str (alias)
 * --------------------------------------
 * A simple alias for char* to improve readability when
 * working with strings.
 * --------------------------------------
 * Equivalent to: char*
 * */
typedef char *str;

//
// Array utilities
//

/*
 * ARRAY_SIZE
 * --------------------------------------
 * Computes the number of elements in a static array.
 * Only works for arrays, NOT pointers.
 * --------------------------------------
 * Complexity: O(1)
 * */
#define ARRAY_SIZE(arr) (sizeof arr / sizeof arr[0])

/*
 * print_int_array
 * --------------------------------------
 * Prints an integer array in [a, b, c] format.
 * --------------------------------------
 * Time complexity: O(n)
 * Space complexity: O(1)
 * */
static inline void clibx_print_int_array(const int *arr, size_t len) {
    printf("[");
    for (size_t i = 0; i < len; i++) {
        printf("%d", arr[i]);
        if (i < len - 1) printf(", ");
    }
    printf("]\n");
}

/*
 * print_double_array
 * --------------------------------------
 * Prints a double array in [a, b, c] format.
 * --------------------------------------
 * Time complexity: O(n)
 * Space complexity: O(1)
 * */
static inline void clibx_print_double_array(const double *arr, size_t len) {
    printf("[");
    for (size_t i = 0; i < len; i++) {
        printf("%f", arr[i]);
        if (i < len - 1) printf(", ");
    }
    printf("]\n");
}

/*
 * print_char_array
 * --------------------------------------
 * Prints a char array as ['a', 'b', 'c'].
 * --------------------------------------
 * Time complexity: O(n)
 * Space complexity: O(1)
 * */
static inline void clibx_print_char_array(const char *arr, size_t len) {
    printf("[");
    for (size_t i = 0; i < len; i++) {
        printf("'%c'", arr[i]);
        if (i < len - 1) printf(", ");
    }
    printf("]\n");
}

//
// Math utilities
//

/*
 * clibx_log2
 * --------------------------------------
 * Computes the base-2 logarithm of x.
 * Returns NaN if x <= 0.
 * --------------------------------------
 * Time complexity: O(1)
 * */
static inline double clibx_log2(double x) {
    union {
        double d;
        uint64_t u;
    } v = { x };

    // NaN for invalid input
    if (x <= 0.0)
        return 0.0 / 0.0;

    // Extract exponent
    int exponent = (int)((v.u >> 52) & 0x7FF) - 1023;

    // Normalize mantissa into [1, 2)
    v.u = (v.u & ((1ULL << 52) - 1)) | (1023ULL << 52);
    double m = v.d;

    // Compute log2(m)
    //
    // Using:
    //
    // log2(m) = ln(m) / ln(2)
    //
    // and:
    //
    // ln(m) = 2 * (y + y^3/3 + y^5/5 + ...)
    //
    // where:
    //
    // y = (m - 1) / (m + 1)

    double y  = (m - 1.0) / (m + 1.0);
    double y2 = y * y;

    double ln_m =
        2.0 * (
            y +
            (y * y2) / 3.0 +
            (y * y2 * y2) / 5.0 +
            (y * y2 * y2 * y2) / 7.0 +
            (y * y2 * y2 * y2 * y2) / 9.0
        );

    // 1 / ln(2)
    const double INV_LN2 = 1.4426950408889634074;

    return exponent + ln_m * INV_LN2;
}
/*
 * clibx_pow
 * --------------------------------------
 * Computes base raised to the power of exp.
 * Only supports integer exponents. Returns NaN for fractional exponents.
 * --------------------------------------
 * Time complexity: O(log exp)
 * */
static inline double clibx_pow(double base, double exp) {
    if (exp == 0.0) return 1.0;
    if (base == 0.0) return 0.0;
    if (exp == 1.0) return base;

    // Check if exp is integer
    double intpart = (int)exp;
    if (exp != intpart) return 0.0 / 0.0; // NaN

    // Handle negative exponents
    if (exp < 0.0) {
        base = 1.0 / base;
        exp = -exp;
    }

    // Integer exponent: exponentiation by squaring
    double result = 1.0;
    int iexp = (int)exp;
    while (iexp > 0) {
        if (iexp % 2 == 1) {
            result *= base;
        }
        base *= base;
        iexp /= 2;
    }
    return result;
}

/*
 * clibx_ceil
 * --------------------------------------
 * Returns the smallest integer greater than or equal to x.
 * --------------------------------------
 * Time complexity: O(1)
 * */
static inline double clibx_ceil(double x) {
    if (x == (int)x) return x;
    return (x > 0.0) ? (int)x + 1 : (int)x;
}

/*
 * clibx_floor
 * --------------------------------------
 * Returns the largest integer less than or equal to x.
 * --------------------------------------
 * Time complexity: O(1)
 * */
static inline double clibx_floor(double x) {
    if (x == (int)x) return x;
    return (x > 0.0) ? (int)x : (int)x - 1;
}

/*
 * clibx_round
 * --------------------------------------
 * Rounds x to the nearest integer.
 * --------------------------------------
 * Time complexity: O(1)
 * */
static inline double clibx_round(double x) {
    return (x >= 0.0) ? clibx_floor(x + 0.5) : clibx_ceil(x - 0.5);
}

/*
 * MIN / MAX
 * --------------------------------------
 * Returns the minimum / maximum of two values.
 * --------------------------------------
 * Complexity: O(1)
 * */
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/*
 * CLAMP
 * --------------------------------------
 * Restricts a value x to the range [min, max].
 * --------------------------------------
 * Complexity: O(1)
 * */
#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

/*
 * ABS
 * --------------------------------------
 * Computes absolute value.
 * --------------------------------------
 * Complexity: O(1)
 * */
#define ABS(x) ((x) < 0 ? -(x) : (x))

/*
 * LERP (Linear Interpolation)
 * --------------------------------------
 * Interpolates between v0 and v1 using parameter t.
 * t = 0 -> v0, t = 1 -> v1
 * --------------------------------------
 * Complexity: O(1)
 * https://en.wikipedia.org/wiki/Linear_interpolation
 * */
#define LERP(v0, v1, t) ((v0) + (t) * ((v1) - (v0)))

/*
 * IS_POWER_OF_2
 * --------------------------------------
 * Checks if n is a power of 2.
 * --------------------------------------
 * Complexity: O(1)
 * */
#define IS_POWER_OF_2(n) ((n) && !((n) & ((n) - 1)))

/*
 * NEXT_POWER_OF_2
 * --------------------------------------
 * Computes the next power of 2 greater than n.
 * Uses floating-point math internally.
 * --------------------------------------
 * Complexity: O(1)
 * */
#define NEXT_POWER_OF_2(n) ((int)clibx_pow(2, clibx_ceil(clibx_log2((n) + 1))))

//
// Memory utilities
//

/*
 * NEW / NEW_ARRAY / NEW_ZEROED
 * --------------------------------------
 * Memory allocation helpers:
 * - NEW: allocate one object
 * - NEW_ARRAY: allocate array
 * - NEW_ZEROED: allocate zero-initialized object
 * --------------------------------------
 * Complexity: O(1)
 * */
#define NEW(type)              ((type*)malloc(sizeof(type)))
#define NEW_ARRAY(type, count) ((type*)malloc(sizeof(type) * (count)))
#define NEW_ZEROED(type)       ((type*)calloc(1, sizeof(type)))

/*
 * FREE
 * --------------------------------------
 * Frees memory and sets pointer to NULL.
 * Prevents dangling pointer bugs.
 * --------------------------------------
 * Complexity: O(1)
 * */
#define FREE(ptr) do { \
    free(ptr);         \
    ptr = NULL;        \
} while(0)

//
// Compiler-specific utilities (GCC/Clang)
//

#ifdef __GNUC__

/*
 * SWAP
 * --------------------------------------
 * Swaps two variables using type inference.
 * --------------------------------------
 * Complexity: O(1)
 * */
#define SWAP(a, b) do {          \
    __typeof__(a) _tmp = (a);    \
    (a) = (b);                   \
    (b) = _tmp;                  \
} while(0)

/*
 * Branch prediction hints
 * */
#define LIKELY(x)    __builtin_expect(!!(x), 1)
#define UNLIKELY(x)  __builtin_expect(!!(x), 0)

/*
 * UNUSED
 * --------------------------------------
 * Suppresses unused variable warnings.
 * */
#define UNUSED(x) (void)(x)

/*
 * Attributes
 * */
#define DEPRECATED   __attribute__((deprecated))
#define NODISCARD    __attribute__((warn_unused_result))

/*
 * Type utilities
 * */
#define TYPE_NAME(x) __typeof__(x)
#define TYPE_FUNC(x) _Generic((x), default: __PRETTY_FUNCTION__)

#else

/*
 * Portable SWAP fallback (memcpy-based)
 * */
#define SWAP(a, b) do {     \
    char _tmp[sizeof(a)];   \
    memcpy(_tmp,  &(a), sizeof(a)); \
    memcpy(&(a),  &(b), sizeof(a)); \
    memcpy(&(b),  _tmp, sizeof(a)); \
} while(0)

#define LIKELY(x)    (x)
#define UNLIKELY(x)  (x)
#define UNUSED(x)    (void)(x)
#define DEPRECATED
#define NODISCARD

#define TYPE_NAME(x) void
#define TYPE_FUNC(x) "<TYPE_FUNC unavailable: requires GCC/Clang>"

#endif

//
// Debug / logging
//

/*
 * LOG
 * --------------------------------------
 * Prints a formatted log message to stderr.
 * */
#define LOG(fmt, ...) \
    fprintf(stderr, "\033[1m\033[1;33m[LOG] \033[0m " fmt "\n", ##__VA_ARGS__)

/*
 * ERROR
 * --------------------------------------
 * Prints error message and exits program.
 * Includes file and line information.
 * */
#define ERROR(fmt, ...) do { \
    fprintf(stderr, "\033[1m\033[1;31m[ERROR] \033[0m %s:%d: " fmt "\n", \
            __FILE__, __LINE__, ##__VA_ARGS__); \
    exit(EXIT_FAILURE); \
} while(0)

/*
 * ASSERT
 * --------------------------------------
 * Runtime assertion with message.
 * */
#define ASSERT(cond, msg) do { \
    if (!(cond)) { ERROR("Assertion failed: %s", msg); } \
} while(0)

#define UNIMPLEMENTED() ERROR("not yet implemented")
#define UNREACHABLE()   ERROR("unreachable code reached")

//
// Loop helpers
//

/*
 * FOR / FOR_RANGE
 * --------------------------------------
 * Simplified loop macros.
 * --------------------------------------
 * FOR(i, n):         0 -> n-1
 * FOR_RANGE(i,a,b):  a -> b-1
 * */
#define FOR(i, n)                for (size_t i = 0; i < (n); i++)
#define FOR_RANGE(i, start, end) for (size_t i = (start); i < (end); i++)

//
// Bitwise operations
//

/*
 * BIT / SET_BIT / CLEAR_BIT / TOGGLE_BIT / CHECK_BIT
 * --------------------------------------
 * Bit manipulation macros.
 * --------------------------------------
 * Complexity: O(1)
 * */
#define BIT(x) (1ULL << (x))
#define SET_BIT(val, pos)   ((val) |= BIT(pos))
#define CLEAR_BIT(val, pos) ((val) &= ~BIT(pos))
#define TOGGLE_BIT(val, pos) ((val) ^= BIT(pos))
#define CHECK_BIT(val, pos) (((val) & BIT(pos)) != 0)

//
// Booleans
//

#ifndef __cplusplus
	#define clibx_isset_boolean
    #define clibx_bool  int
    #define clibx_true  1
    #define clibx_false 0
#endif

/*
 * CLIBX_PRINT_BOOL
 * --------------------------------------
 * Prints boolean as "true" or "false".
 * */
#define CLIBX_PRINT_BOOL(b) ((b) == 1 ? printf("true\n") : printf("false\n"))

//
// String utilities
//

/*
 * STREQ
 * --------------------------------------
 * Compares two strings for equality.
 * */
#define STREQ(a, b) (strcmp((a), (b)) == 0)

/*
 * STR_EMPTY
 * --------------------------------------
 * Checks if string is empty.
 * */
#define STR_EMPTY(s) ((s)[0] == '\0')

/*
 * STR_STARTS_WITH
 * --------------------------------------
 * Checks if string starts with prefix.
 * */
#define STR_STARTS_WITH(s, p) (strncmp((s), (p), strlen(p)) == 0)

//
// Type introspection
//

/*
 * TYPE_STR
 * --------------------------------------
 * Returns string representation of type.
 * */
#define TYPE_STR(x) _Generic((x),   \
    int:     "int",                \
    long:    "long",               \
    float:   "float",              \
    double:  "double",             \
    char:    "char",               \
    default: "unknown by TYPE_STR")

/*
 * PRINT
 * --------------------------------------
 * Generic print macro for basic types.
 * */
#define PRINT(x) _Generic((x),        \
    int:     printf("%d\n",  (x)),    \
    long:    printf("%ld\n", (x)),    \
    float:   printf("%f\n",  (x)),    \
    double:  printf("%f\n",  (x)),    \
    char:    printf("%c\n",  (x)),    \
    char*:   printf("%s\n",  (x)),    \
    default: printf("<unknown type>\n"))

//
// I/O utilities
//

/*
 * read_line
 * --------------------------------------
 * Reads a line from stdin safely.
 * Removes trailing newline if present.
 * --------------------------------------
 * Time complexity: O(n)
 * */
static inline void clibx_read_line(char *buffer, size_t size) {
    if (fgets(buffer, size, stdin)) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n')
            buffer[len - 1] = '\0';
    }
}

//
// Dynamic array (Vec)
//

typedef struct {
    str *data;
    size_t length;
    size_t capacity;
} clibx_str_vec;

/*
 * vec_init
 * --------------------------------------
 * Initialize an empty string vector.
 * */
static inline clibx_str_vec clibx_vec_init(void) {
    return (clibx_str_vec){ .data = NULL, .length = 0, .capacity = 0 };
}

/*
 * vec_push
 * --------------------------------------
 * Append a string to the vector.
 * */
static inline void clibx_vec_push(clibx_str_vec *vec, str value) {
    if (vec->length >= vec->capacity) {
        size_t new_cap = vec->capacity == 0 ? 8 : vec->capacity * 2;
        vec->data = (str *)realloc(vec->data, sizeof(str) * new_cap);
        ASSERT(vec->data != NULL, "vec_push: realloc failed");
        vec->capacity = new_cap;
    }
    vec->data[vec->length++] = value;
}

/*
 * vec_free
 * --------------------------------------
 * Free the vector's internal data.
 * Does NOT free individual strings.
 * */
static inline void clibx_vec_free(clibx_str_vec *vec) {
    FREE(vec->data);
    vec->length = 0;
    vec->capacity = 0;
}

//
// String utilities
//

/*
 * strtrim
 * --------------------------------------
 * Returns a newly allocated trimmed string (leading/trailing whitespace removed).
 * Caller must free the result.
 * --------------------------------------
 * Time complexity: O(n)
 * */
static inline str clibx_strtrim(str s) {
    if (!s || *s == '\0') return strdup("");

    str copy = strdup(s);
    str start = copy;
    while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r') start++;
    if (*start == '\0') { free(copy); return strdup(""); }

    str end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r'))
        end--;
    *(end + 1) = '\0';

    str result = strdup(start);
    free(copy);
    return result;
}

/*
 * strsplit
 * --------------------------------------
 * Splits a string by delimiter into a vec.
 * Caller must free vec and individual strings.
 * --------------------------------------
 * Time complexity: O(n)
 * */
static inline clibx_str_vec clibx_strsplit(str input, char delim) {
    clibx_str_vec result = clibx_vec_init();
    char *copy = strdup(input);
    char delim_str[2] = {delim, '\0'};
    char *token = strtok(copy, delim_str);

    while (token != NULL) {
        clibx_vec_push(&result, strdup(token));
        token = strtok(NULL, delim_str);
    }

    free(copy);
    return result;
}

/*
 * strjoin
 * --------------------------------------
 * Joins an array of strings with a separator.
 * Returns a newly allocated string. Caller must free.
 * --------------------------------------
 * Time complexity: O(n)
 * */
static inline str clibx_strjoin(str *arr, size_t len, str sep) {
    if (len == 0) return strdup("");

    size_t total_len = 0;
    for (size_t i = 0; i < len; i++) {
        total_len += strlen(arr[i]);
        if (i < len - 1) total_len += strlen(sep);
    }

    str result = NEW_ARRAY(char, total_len + 1);
    ASSERT(result != NULL, "strjoin: malloc failed");

    result[0] = '\0';
    for (size_t i = 0; i < len; i++) {
        strcat(result, arr[i]);
        if (i < len - 1) strcat(result, sep);
    }

    return result;
}

/*
 * str_contains
 * --------------------------------------
 * Checks if haystack contains needle.
 * */
#define STR_CONTAINS(haystack, needle) (strstr((haystack), (needle)) != NULL)

/*
 * str_to_lower
 * --------------------------------------
 * Converts string to lowercase in place. Returns same pointer.
 * */
static inline str clibx_str_to_lower(str s) {
    for (size_t i = 0; s[i]; i++) {
        if (s[i] >= 'A' && s[i] <= 'Z')
            s[i] = s[i] + 32;
    }
    return s;
}

/*
 * str_to_upper
 * --------------------------------------
 * Converts string to uppercase in place. Returns same pointer.
 * */
static inline str clibx_str_to_upper(str s) {
    for (size_t i = 0; s[i]; i++) {
        if (s[i] >= 'a' && s[i] <= 'z')
            s[i] = s[i] - 32;
    }
    return s;
}

//
// Path utilities
//

/*
 * path_basename
 * --------------------------------------
 * Returns the filename component of a path.
 * */
static inline str clibx_path_basename(str path) {
    char *copy = strdup(path);
    str result = strdup(basename(copy));
    free(copy);
    return result;
}

/*
 * path_dirname
 * --------------------------------------
 * Returns the directory component of a path.
 * */
static inline str clibx_path_dirname(str path) {
    char *copy = strdup(path);
    str result = strdup(dirname(copy));
    free(copy);
    return result;
}

/*
 * path_extension
 * --------------------------------------
 * Returns the file extension (without dot), or empty string.
 * */
static inline str clibx_path_extension(str path) {
    str base = clibx_path_basename(path);
    str dot = strrchr(base, '.');
    str ext = dot ? strdup(dot + 1) : strdup("");
    free(base);
    return ext;
}

/*
 * path_join
 * --------------------------------------
 * Joins two path components with '/'.
 * Returns a newly allocated string. Caller must free.
 * */
static inline str clibx_path_join(str a, str b) {
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    size_t sep = (len_a > 0 && a[len_a - 1] != '/') ? 1 : 0;

    str result = NEW_ARRAY(char, len_a + sep + len_b + 1);
    ASSERT(result != NULL, "path_join: malloc failed");

    sprintf(result, "%s%s%s", a, sep ? "/" : "", b);
    return result;
}

/*
 * path_exists
 * --------------------------------------
 * Checks if a path exists on disk.
 * */
static inline clibx_bool clibx_path_exists(str path) {
    return access(path, F_OK) == 0;
}

/*
 * path_is_dir
 * --------------------------------------
 * Checks if a path is a directory.
 * */
static inline clibx_bool clibx_path_is_dir(str path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

/*
 * path_is_file
 * --------------------------------------
 * Checks if a path is a regular file.
 * */
static inline clibx_bool clibx_path_is_file(str path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISREG(st.st_mode));
}

/*
 * path_file_size
 * --------------------------------------
 * Returns file size in bytes, or -1 on error.
 * */
static inline long clibx_path_file_size(str path) {
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return (long)st.st_size;
}

//
// Hash Map
//

/*
 * CLIBX_HASHMAP - Type-agnostic hash map
 * --------------------------------------
 * A generic hash map implementation supporting arbitrary key/value types.
 * Uses separate chaining for collision resolution.
 *
 * Users provide:
 *   - hash_fn:  hash function for keys
 *   - cmp_fn:   key comparison function (return non-zero if equal)
 *   - free_key: destructor for keys (can be NULL if keys don't need freeing)
 *   - free_val: destructor for values (can be NULL if values don't need freeing)
 *
 * A string-specialized convenience API (hashmap_init_str, etc.) is
 * provided for the common str→str use case.
 * --------------------------------------
 */

#define CLIBX_HASHMAP_DEFAULT_CAPACITY 64

typedef unsigned long (*clibx_hash_fn)(void *key);
typedef int           (*clibx_cmp_fn)(void *a, void *b);
typedef void          (*clibx_free_fn)(void *data);

static inline unsigned long clibx_hashmap_djb2(void *key) {
    unsigned long hash = 5381;
    int c;
    str s = (str)key;
    while ((c = *s++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

static inline int clibx_hashmap_str_cmp(void *a, void *b) {
    return STREQ((str)a, (str)b);
}

typedef struct clibx_hashmap_entry {
    void *key;
    void *value;
    struct clibx_hashmap_entry *next;
} clibx_hashmap_entry;

typedef struct {
    clibx_hashmap_entry **buckets;
    size_t capacity;
    size_t count;
    clibx_hash_fn hash_fn;
    clibx_cmp_fn  cmp_fn;
    clibx_free_fn free_key;
    clibx_free_fn free_val;
} clibx_hashmap;

/*
 * hashmap_init
 * --------------------------------------
 * Create a new empty hashmap with custom callbacks.
 *
 * Parameters:
 *   hash_fn  - hash function for keys
 *   cmp_fn   - key comparison function (return non-zero if equal)
 *   free_key - destructor for keys (NULL if no cleanup needed)
 *   free_val - destructor for values (NULL if no cleanup needed)
 *
 * Example (generic):
 *   clibx_hashmap map = clibx_hashmap_init(my_hash, my_cmp, NULL, NULL);
 *
 * Example (strings):
 *   clibx_hashmap map = clibx_hashmap_init_str();
 * --------------------------------------
 */
static inline clibx_hashmap clibx_hashmap_init(
    clibx_hash_fn hash_fn,
    clibx_cmp_fn  cmp_fn,
    clibx_free_fn free_key,
    clibx_free_fn free_val
) {
    clibx_hashmap map;
    map.capacity  = CLIBX_HASHMAP_DEFAULT_CAPACITY;
    map.count     = 0;
    map.hash_fn   = hash_fn;
    map.cmp_fn    = cmp_fn;
    map.free_key  = free_key;
    map.free_val  = free_val;
    map.buckets   = NEW_ARRAY(clibx_hashmap_entry*, map.capacity);
    memset(map.buckets, 0, sizeof(clibx_hashmap_entry*) * map.capacity);
    return map;
}

/*
 * hashmap_init_str
 * --------------------------------------
 * Convenience: create a string-to-string hashmap.
 * Uses DJB2 hashing, strcmp-based comparison, and free() for both keys and values.
 *
 * Example:
 *   clibx_hashmap map = clibx_hashmap_init_str();
 *   clibx_hashmap_put(&map, "name", strdup("Alice"), free, free);
 *   char *v = (char*)clibx_hashmap_get(&map, "name");
 *   clibx_hashmap_free(&map);
 * --------------------------------------
 */
static inline clibx_hashmap clibx_hashmap_init_str(void) {
    return clibx_hashmap_init(
        clibx_hashmap_djb2,
        clibx_hashmap_str_cmp,
        free,
        free
    );
}

/*
 * hashmap_put
 * --------------------------------------
 * Insert or update a key-value pair.
 * The map takes ownership of both key and value pointers.
 * If the key already exists, the old value is freed via free_val
 * before storing the new value.
 *
 * Parameters:
 *   map   - pointer to the hashmap
 *   key   - key pointer (ownership transferred)
 *   value - value pointer (ownership transferred)
 *
 * Example:
 *   clibx_hashmap_put(&map, strdup("key"), strdup("value"), free, free);
 * --------------------------------------
 */
static inline void clibx_hashmap_put(
    clibx_hashmap *map,
    void *key,
    void *value
) {
    unsigned long hash = map->hash_fn(key) % map->capacity;
    clibx_hashmap_entry *entry = map->buckets[hash];

    while (entry != NULL) {
        if (map->cmp_fn(entry->key, key)) {
            if (map->free_key) map->free_key(entry->key);
            if (map->free_val) map->free_val(entry->value);
            entry->key   = key;
            entry->value = value;
            return;
        }
        entry = entry->next;
    }

    clibx_hashmap_entry *new_entry = NEW_ZEROED(clibx_hashmap_entry);
    new_entry->key   = key;
    new_entry->value = value;
    new_entry->next  = map->buckets[hash];
    map->buckets[hash] = new_entry;
    map->count++;
}

/*
 * hashmap_get
 * --------------------------------------
 * Look up a value by key.  Returns NULL if the key is not found.
 *
 * Example:
 *   char *v = (char*)clibx_hashmap_get(&map, "name");
 * --------------------------------------
 */
static inline void *clibx_hashmap_get(clibx_hashmap *map, void *key) {
    unsigned long hash = map->hash_fn(key) % map->capacity;
    clibx_hashmap_entry *entry = map->buckets[hash];

    while (entry != NULL) {
        if (map->cmp_fn(entry->key, key))
            return entry->value;
        entry = entry->next;
    }
    return NULL;
}

/*
 * hashmap_contains
 * --------------------------------------
 * Return non-zero if the key exists in the map.
 *
 * Example:
 *   if (clibx_hashmap_contains(&map, "name")) { ... }
 * --------------------------------------
 */
static inline clibx_bool clibx_hashmap_contains(clibx_hashmap *map, void *key) {
    return clibx_hashmap_get(map, key) != NULL;
}

/*
 * hashmap_remove
 * --------------------------------------
 * Remove a key-value pair from the map.
 * The key and value are freed via the map's free_key / free_val callbacks.
 * Returns 1 if the key was found and removed, 0 otherwise.
 *
 * Example:
 *   clibx_hashmap_remove(&map, "name");
 * --------------------------------------
 */
static inline int clibx_hashmap_remove(clibx_hashmap *map, void *key) {
    unsigned long hash = map->hash_fn(key) % map->capacity;
    clibx_hashmap_entry *entry = map->buckets[hash];
    clibx_hashmap_entry *prev = NULL;

    while (entry != NULL) {
        if (map->cmp_fn(entry->key, key)) {
            if (prev)
                prev->next = entry->next;
            else
                map->buckets[hash] = entry->next;

            if (map->free_key) map->free_key(entry->key);
            if (map->free_val) map->free_val(entry->value);
            free(entry);
            map->count--;
            return 1;
        }
        prev  = entry;
        entry = entry->next;
    }
    return 0;
}

/*
 * hashmap_free
 * --------------------------------------
 * Free all memory used by the hashmap.
 * Calls free_key / free_val on every entry before freeing the bucket array.
 *
 * Example:
 *   clibx_hashmap_free(&map);
 * --------------------------------------
 */
static inline void clibx_hashmap_free(clibx_hashmap *map) {
    for (size_t i = 0; i < map->capacity; i++) {
        clibx_hashmap_entry *entry = map->buckets[i];
        while (entry != NULL) {
            clibx_hashmap_entry *next = entry->next;
            if (map->free_key) map->free_key(entry->key);
            if (map->free_val) map->free_val(entry->value);
            free(entry);
            entry = next;
        }
    }
    FREE(map->buckets);
    map->count    = 0;
    map->capacity = 0;
}

#endif /* CLIBX_H */
