/*
    ws value ws
    ws = *(' ' \t \r \n)
    value = false / null / true / object / array / string / number
*/

#include <stdlib.h>
#include <assert.h>
#include <errno.h> // errno
#include <math.h>
#include "leptjson.h"

#define EXPECT(c, ch) do { assert((*c->json) == (ch)); c->json++;} while(0)
#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char* p = c->json;
    while(*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    c->json = p;
}


static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    const char* p = c->json;
    if (p[0] != 'u' || p[1] != 'l' || p[2] != 'l') return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context* c,lept_value* v) {
    EXPECT(c, 'f');
    const char* p = c->json;
    if (p[0] != 'a' && p[1] != 'l' && p[2] != 's' && p[3] != 'e') return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_true(lept_context* c,lept_value* v) {
    EXPECT(c, 't');
    const char* p = c->json;
    if (p[0] != 'r' || p[1] != 'u' || p[2] != 'e' ) return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
    const char* p = c->json;
    if (*p == '-') p++;
    if (*p == '0') p++;
    else {
        if (!ISDIGIT1TO9(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == '.') {
        p++;
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '-' || *p == '+') p++;
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    errno = 0; 
    v->u.n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL)) return LEPT_PARSE_NUMBER_TOO_BIG;
    v->type = LEPT_NUMBER;
    c->json = p;
    return LEPT_PARSE_OK;
}


lept_type lept_get_type(lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->u.n;
}


static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 'n': return lept_parse_null(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
        default:   return lept_parse_number(c, v);
        case 'f': return lept_parse_false(c, v);
        case 't': return lept_parse_true(c, v);
        // case '\"': return lept_parse_string(c, v);
    }
}

int lept_parse(lept_value* v, const char* json) {
    assert(v != NULL);
    int ret;
    lept_context c;
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
    return ret;
        
}