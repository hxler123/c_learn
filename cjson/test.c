
#include <stdio.h> // fprintf
#include <stdlib.h>
#include "leptjson.h"

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

#define TEST_ERROR(error, json) \
    do {\
        lept_value v;\
        v.type = LEPT_FALSE;\
        EXPECT_EQ_INT(error, lept_parse(&v, json));\
        EXPECT_EQ_INT(LEPT_NULL, lept_get_type);\
    }\

#define TEST_NUMBER(expect, json) \
    do {\
        lept_value v;\
        v.type = LEPT_TRUE;\
        EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json));\
        EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&v));\
        EXPECT_EQ_DOUBLE(expect, lept_get_number(&v));\
    }\
    while(0)\

static int main_ret = 0;
static int test_pass = 0;
static int test_count = 0;

static void test_parse_null() {
    lept_value v;
    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "null"));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
}

static void test_parse_expect_value() {
    TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, "");
    TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() {
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "?");

    /* invalid number */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "0");
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
    TEST_NUMBER(1234, "1234");
    TEST_NUMBER(-1234, "-1234");
    TEST_NUMBER(-1234, "-1234");
    TEST_NUMBER(123.45, "123.45");
    TEST_NUMBER(0.1234, "0.1234");
    TEST_NUMBER(-1.2e10, "-1.2e10");
    TEST_NUMBER(1.3e10, "1.3e10");
    TEST_NUMBER(1.45e-10, "1.45e-10");
    TEST_NUMBER(1.456e+10, "1.456e+10");
}


static void test_parse() {
    test_parse_null();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_boolean();
    test_parse_number();
}

int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}