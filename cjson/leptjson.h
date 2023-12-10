#ifndef LEPTJSON_H__
#define LEPTJSON_H__

/*
e.g.
    lept_value v;
    const char json[] = ...;
    lept_parse(&v, json);
*/

typedef enum { LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_STRING, LEPT_NUMBER, LETP_ARRAY, LEPT_OBJECT }lept_type;

typedef struct {
    lept_type type;
    union {
        double n;
        struct { char* s; size_t len; }s;
    }u;
}lept_value;

enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR,
    LEPT_PARSE_NUMBER_TOO_BIG
};

int lept_parse(lept_value* v, const char* json);
lept_type lept_get_type(lept_value* v);
double lept_get_number(lept_value* v);

#endif /* LEPTJSON_H__ */