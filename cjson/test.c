
#include <stdio.h> // fprintf
#include <stdlib.h>
#include <string.h>
#include "leptjson.h"
// #include "leptjson.c"

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    test_count++;\
    if ((equality)) {\
        test_pass++;\
    }\
    else {\
        main_ret = 1;\
        fprintf(stderr, "%s:%d: expect:" format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
    }\

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define EXPECT_EQ_STRING(expect, actual, alength) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == alength && memcmp(expect, actual, alength) == 0, expect, actual, "%s")

#define TEST_ERROR(error, json) \
    do {\
        lept_value v;\
        v.type = LEPT_FALSE;\
        EXPECT_EQ_INT(error, lept_parse(&v, json));\
        EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));\
    } while(0)\

#define TEST_NUMBER(expect, json) \
    do {\
        lept_value v;\
        v.type = LEPT_TRUE;\
        EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json));\
        EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&v));\
        EXPECT_EQ_DOUBLE(expect, lept_get_number(&v));\
    } while(0)\

#define TEST_STRING(expect, json) \
    do {\
        lept_value v;\
        v.type = LEPT_TRUE;\
        EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json));\
        EXPECT_EQ_INT(LEPT_STRING, lept_get_type(&v));\
        EXPECT_EQ_STRING(expect, lept_get_string(&v), lept_get_string_length(&v));\
    } while(0)\

static int main_ret = 0;
static int test_pass = 0;
static int test_count = 0;

static void test_parse_null() {
    lept_value v;
    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "null"));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
}

static void test_parse_boolean(){
    lept_value v;
    v.type = LEPT_TRUE;

    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "false"));
    EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&v));
    
    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "true"));
    EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&v));

}

static void test_parse_number() {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(1234.0, "1234");
    TEST_NUMBER(-1234.0, "-1234");
    TEST_NUMBER(-1234.0, "-1234");
    TEST_NUMBER(123.45, "123.45");
    TEST_NUMBER(0.1234, "0.1234");
    TEST_NUMBER(-1.2e10, "-1.2e10");
    TEST_NUMBER(1.3e10, "1.3e10");
    TEST_NUMBER(1.45e-10, "1.45e-10");
    TEST_NUMBER(1.456e+10, "1.456e+10");

}

static void test_parse_string() {
    TEST_STRING("abcd", "\"abcd\"");
    TEST_STRING(" /\\", "\" /\\\\\"");
    TEST_STRING("\t", "\"\\t\"");
    TEST_STRING("\f", "\"\\f\"");
    TEST_STRING("\r\n", "\"\\r\\n\"");
    TEST_STRING("f\"", "\"f\\\"\"");
    TEST_STRING("f\"", "\"f\\\"\"");
    TEST_STRING("这", "\"\\u8fd9\"");
    TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
    TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
    TEST_STRING("Hello\0World", "\"Hello\\u0000World\""); // memcmp 
}

static void test_parse_array() {
    lept_value v;
    // v.type = LEPT_FALSE;
    // EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "[]"));
    // EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(&v));
    // EXPECT_EQ_INT(0, lept_get_array_size(&v));
    // EXPECT_EQ_INT(NULL, v.u.a.size);

    lept_free(&v);
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "[ 1 , \"KL\", 123, false, true, null ]"));
    EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(&v));
    EXPECT_EQ_INT(6, lept_get_array_size(&v));

    EXPECT_EQ_DOUBLE(1.0, lept_get_number(lept_get_array_element(&v, 0)));
    EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(lept_get_array_element(&v, 0)));

    EXPECT_EQ_STRING("KL", lept_get_string(lept_get_array_element(&v, 1)), lept_get_string_length(lept_get_array_element(&v, 1)));
    EXPECT_EQ_INT(LEPT_STRING, lept_get_type(lept_get_array_element(&v, 1)));

    EXPECT_EQ_DOUBLE(123, lept_get_number(lept_get_array_element(&v, 2)));
    EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(lept_get_array_element(&v, 2)));

    EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(lept_get_array_element(&v, 3)));
    EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(lept_get_array_element(&v, 4)));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(lept_get_array_element(&v, 5)));

    lept_free(&v);
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
    EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(&v));
    EXPECT_EQ_INT(4, lept_get_array_size(&v));
    for (size_t i = 0; i< 4; i++) {
        lept_value* a = lept_get_array_element(&v, i);
        EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(a));
        EXPECT_EQ_INT(i, lept_get_array_size(a));
        for (size_t j = 0; j < i; j++) {
            lept_value* e = lept_get_array_element(a, j);
            EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(e));
            EXPECT_EQ_DOUBLE((double)j, lept_get_number(e));
        }
    }
    lept_free(&v);

}

static void test_parse_expect_value() {
    TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, "");
    TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() {
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "?");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "[,");

    /* invalid number */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "1.");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, ".1");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "1.2E");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "1.2e");
}

static void test_parse_root_not_singular() {
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "null x");
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "\"hh\0ll\"");
}

static void test_parse_invalid_string_escape() {
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\l\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\tt\\l\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\u023f\\k\"");
}

static void test_parse_invalid_unicode_surrogate() {
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uD800\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\u1234\\uD800\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\u1234\\uDBFF\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\\uE000\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\\uDBFF\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFFk\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\\o\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\k\"");
}

static void test_parse_invalid_unicode_hex() {
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u09\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u110\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\um110\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u\\u1234\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\uD800\\ukmj1\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\uD800\\u\\u\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\uD800\\u01\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\uD800\\u000\"");
}

static void test_parse_invalid_string_char() {
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_parse_miss_comma_or_square_bracket() {
    TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1,\"123\"");
    TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
}

static void test_parse() {
    test_parse_null();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_boolean();
    test_parse_number();
    test_parse_string();
    test_parse_invalid_string_escape();
    test_parse_invalid_unicode_surrogate();
    test_parse_invalid_unicode_hex();
    test_parse_invalid_string_char();
    test_parse_miss_comma_or_square_bracket();
    test_parse_array();
}



int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}