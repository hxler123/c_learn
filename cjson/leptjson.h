#ifndef LEPTJSON_H__
#define LEPTJSON_H__

/*
e.g.
    lept_value v;
    const char json[] = ...;
    lept_parse(&v, json);
*/

typedef struct lept_value lept_value;

typedef enum { LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_STRING, LEPT_NUMBER, LEPT_ARRAY, LEPT_OBJECT }lept_type;

struct lept_value{
    lept_type type;
    union {
        double n;
        struct { char* s; size_t len; }s;
        struct { lept_value* e; size_t size; }a;
    }u;
};

enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR,
    LEPT_PARSE_NUMBER_TOO_BIG,
    LEPT_PARSE_INVALID_STRING_CHAR,
    LEPT_PARSE_INVALID_UNICODE_HEX,
    LEPT_PARSE_INVALID_UNICODE_SURROGATE,
    LEPT_PARSE_INVALID_STRING_ESCAPE,
    LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET
};

int lept_parse(lept_value* v, const char* json);
lept_type lept_get_type(lept_value* v);
double lept_get_number(lept_value* v);
char* lept_get_string(lept_value* v);
int lept_get_string_length(lept_value* v);
size_t lept_get_array_size(lept_value* v);
lept_value* lept_get_array_element(lept_value* v, size_t);
void lept_free(lept_value* v);

#endif /* LEPTJSON_H__ */