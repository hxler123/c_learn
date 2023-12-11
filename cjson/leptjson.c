/*
    ws value ws
    ws = *(' ' \t \r \n)
    value = false / null / true / object / array / string / number
*/

#include <stdlib.h>
#include <assert.h>
#include <errno.h> // errno
#include <math.h>
#include <string.h>
#include "leptjson.h"

#define EXPECT(c, ch) do { assert((*c->json) == (ch)); c->json++;} while(0)
#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')
#define PUT_C(c, ch)  do {*(char*)lept_context_push((c), sizeof(char)) = (ch);} while(0)


#ifndef LEPT_STACK_INIT_SIZE
#define LEPT_STACK_INIT_SIZE 512
#endif

typedef struct {
    char* stack;
    size_t size, top; /* top is next element index*/
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char* p = c->json;
    while(*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v, char* literal, lept_type type) {
    EXPECT(c, literal[0]);
    size_t i;
    for(i = 0; literal[i+1]; i++) {
        if (c->json[i] != literal[i+1]) return LEPT_PARSE_INVALID_VALUE;
    }
    c->json += i;
    v->type = type;
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

static void* lept_context_push(lept_context* c, size_t size) {
    assert(size > 0);
    void* ret;
    if (c->top + size >= c->size) {
        if (c->size == 0) 
            c->stack = (char*)malloc(LEPT_STACK_INIT_SIZE);
            c->size = LEPT_STACK_INIT_SIZE;
        while (c->top + size >= c->size)
            c->size >>= 1;
        c->stack = (char*)realloc(c->stack, c->size);
    }
    ret = c->stack + c->top;
    c->top += size;
    return ret;
}

static void lept_free(lept_value* v) {
    assert(v != NULL);
    if (v->type == LEPT_STRING) {
        free(v->u.s.s);
    }
    v->type = LEPT_NULL;
}

static void* lept_context_pop(lept_context* c, size_t size) {
    /* 0 1 2 3 4 */
    assert(c->top >= size);
    return c->stack + (c->top -= size);
}

static void lept_set_string(lept_value* v, char* ch, size_t length) {
    lept_free(v);
    v->u.s.s = (char*)malloc(length + 1);
    memcpy(v->u.s.s, ch, length + 1);
    v->type = LEPT_STRING;
    v->u.s.s[length] = '\0';
    v->u.s.len = length;
}

static const char* lept_parse_hex4(const char* p, unsigned* u) {
    *u = 0;
    for (size_t i = 0; i < 4; i++) {
        char ch = *p++;
        *u <<= 4;
        if (ch >= '0' & ch <= '9') *u |= (ch - '0');
        else if (ch >= 'A' & ch <= 'F') *u |= (ch - 'A' + 10);
        else if (ch >= 'a' & ch <= 'f') *u |= (ch - 'a' + 10);
        else return NULL;
    }
    return p;
}

static void lept_encode_utf8(lept_context* c, unsigned u) {
    if (u <= 0x7F) 
        PUT_C(c, u);
    else if (u >= 0x80 && u <= 0x7FF) {
        PUT_C(c, 0xC0 | u >> 6);
        PUT_C(c, 0x80 | (u & 0x3F));
    }
    else if (u >= 0x0800 && u <= 0xFFFF) {
        PUT_C(c, 0xE0 | u >> 12);
        PUT_C(c, 0x80 | (0x3F & (u >> 6)));
        PUT_C(c, 0x80 | (0x3F & u));
    }
    else {
        assert(u <= 0x10FFFF);
        PUT_C(c, 0xF0 | u >> 18);
        PUT_C(c, 0x80 | (0x3F & (u >> 12)));
        PUT_C(c, 0x80 | (0x3F & (u >> 6)));
        PUT_C(c, 0x80 | (0x3F & u));    
    }
}

#define STRING_ERROR(ret) do { c->top = head; return ret;} while(0)

static int lept_parse_string(lept_context* c, lept_value* v) {
    /*
    1. parse char exclude 0x0-0x20 to ascii
    2. parse unicode and escape
        unicode: 
            \u0000 - \u007F: 0XXXXXXX
            \u0080 - \u07FF: 110XXXXX 10XXXXXX
            \u0800 - \uFFFF: 1110XXXX 10XXXXXX 10XXXXXX
            \u10000 - \u10FFFF:  11110XXX 10XXXXXX 10XXXXXX 10XXXXXX
            NOTE:
                [\uD800 - \uDBFF][\uDC00 - \uDFFF] => \u10000 - \u10FFFF

    3. 'a\0b' is LEPT_PARSE_ROOT_NOT_SINGULAR
    */
    EXPECT(c, '\"');
    unsigned u,u2;
    size_t head = c->top, length;     
    const char* p = c->json;
    for (;;) {
        char ch = *p++;
        switch (ch) {
            case '\0': return LEPT_PARSE_ROOT_NOT_SINGULAR;
            case '\"':
                length = c->top - head;
                lept_set_string(v, lept_context_pop(c, length), length);
                c->json = p;
                return LEPT_PARSE_OK;    
            case '\\':
                switch (*p++) {
                    case 't': PUT_C(c, '\t'); break;
                    case 'n': PUT_C(c, '\n'); break;
                    case 'r': PUT_C(c, '\r'); break;
                    case 'f': PUT_C(c, '\f'); break;
                    case '"': PUT_C(c, '"'); break;
                    case '\\': PUT_C(c, '\\'); break;
                    case '/':  PUT_C(c, '/'); break;
                    case 'u':
                        if (!(p = lept_parse_hex4(p, &u))) {
                            STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
                        }
                        if (u >= 0xD800 && u <= 0xDBFF) {
                            if (*p++ != '\\') {
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            }
                            if (*p++ != 'u') {
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            }
                            if (!(p = lept_parse_hex4(p, &u2))) {
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
                            }
                            if (u2 < 0xDC00 || u2 > 0xDFFF) {
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            }
                            u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                        }
                        lept_encode_utf8(c, u);
                        break;
                    default:
                        STRING_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE);
                }
                break;
            default: {
                if (ch < 0x20)
                    STRING_ERROR(LEPT_PARSE_INVALID_STRING_CHAR);
                PUT_C(c, ch);
            }
        }
    }
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 'n': return lept_parse_literal(c, v, "null", LEPT_NULL);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
        default:   return lept_parse_number(c, v);
        case 'f': return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 't': return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case '\"': return lept_parse_string(c, v);
    }
}

int lept_parse(lept_value* v, const char* json) {
    assert(v != NULL);
    int ret;
    lept_context c;
    c.size = c.top = 0;
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
    return ret;
        
}

lept_type lept_get_type(lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->u.n;
}

int lept_get_string_length(lept_value* v) {
    assert(v != NULL && v->type == LEPT_STRING);
    return v->u.s.len;
}

char* lept_get_string(lept_value* v) {
    assert(v != NULL && v->type == LEPT_STRING);
    return v->u.s.s;
}