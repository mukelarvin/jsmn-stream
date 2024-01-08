#include "unity.h"
#define JSMN_PARENT_LINKS
#define JSMN_COMPATIBILITY_MODE 1

/* The module to test */
#include "jsmn_stream_utils.h"
#include "jsmn_stream.h"
#include <stdint.h>
#include <string.h>

const char* json_data = "{\n"
    " \"operations\": [\n"
    "    {\n"
    "        \"id\": 1234,\n"
    "        \"version\": 1,\n"
    "        \"class\": \"pwm\",\n"
    "        \"label\": \"instance of pwm\",\n"
    "        \"operation properties\": \n"
    "        {\n"
    "            \"id\": \"PWMA\",\n"
    "            \"period\": 50.5,\n"
    "            \"polarity\": \"positive\"\n"
    "        }\n"
    "    },\n"
    "    {\n"
    "        \"id\": 5678,\n"
    "        \"version\": 1,\n"
    "        \"class\": \"gpio\",\n"
    "        \"label\": \"instance of gpio\",\n"
    "        \"operation properties\": \n"
    "        {\n"
    "            \"port\": \"A\",\n"
    "            \"pin\": 1,\n"
    "            \"enabled\": \"true\",\n"
    "            \"offset\": -3\n"
    "        }\n"
    "    }\n"
    " ]\n"
"}";


static int32_t get_char_cb(uint32_t index, size_t length, void *user_arg, char *ch)
{
    const char *data = (const char *)user_arg;
    memcpy(ch, &data[index], length);
    return 0;
}

void setUp(void)
{

}

void tearDown(void)
{

}

void test_jsmn_stream_utils_get_value_token_by_key(void)
{
    new_jsmn_stream_token_parser_t parser;
    jsmn_stream_token_t value_token;
 
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(json_data), (void *)json_data));

    // first key
    jsmn_stream_utils_init_token(&value_token);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, "operations", &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_ARRAY, value_token.type);
    TEST_ASSERT_EQUAL_INT32(17, value_token.start_position);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_POSITION_UNDEFINED, value_token.end_position);
    TEST_ASSERT_EQUAL_INT32(4, value_token.parent_position);

    // first primitive
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, "id", &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_PRIMITIVE, value_token.type);
    TEST_ASSERT_EQUAL_INT32(39, value_token.start_position);
    TEST_ASSERT_EQUAL_INT32(43, value_token.end_position);
    TEST_ASSERT_EQUAL_INT32(34, value_token.parent_position);

    // first string
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, "class", &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_STRING, value_token.type);
    TEST_ASSERT_EQUAL_INT32(85, value_token.start_position);
    TEST_ASSERT_EQUAL_INT32(88, value_token.end_position);
    TEST_ASSERT_EQUAL_INT32(76, value_token.parent_position);

    // a string deeper down the tree
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, "enabled", &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_STRING, value_token.type);
    TEST_ASSERT_EQUAL_INT32(500, value_token.start_position);
    TEST_ASSERT_EQUAL_INT32(504, value_token.end_position);
    TEST_ASSERT_EQUAL_INT32(489, value_token.parent_position);

    // a string that doesn't exist
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_KEY_NOT_FOUND, jsmn_stream_utils_get_value_token_by_key(&parser, "does not exist", &value_token));
}

void test_jsmn_stream_utils_get_next_object_token(void)
{
    new_jsmn_stream_token_parser_t parser;
    jsmn_stream_token_t array_token;
    jsmn_stream_token_t value_token;

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(json_data), (void *)json_data));

    // get the array from the first key
    jsmn_stream_utils_init_token(&value_token);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, "operations", &array_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_ARRAY, array_token.type);
    TEST_ASSERT_EQUAL_INT32(17, array_token.start_position);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_POSITION_UNDEFINED, array_token.end_position);
    TEST_ASSERT_EQUAL_INT32(4, array_token.parent_position);

    // first object
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_next_object_token(&parser, &array_token, &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_OBJECT, value_token.type);
    TEST_ASSERT_EQUAL_INT32(23, value_token.start_position);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_POSITION_UNDEFINED, value_token.end_position);
    TEST_ASSERT_EQUAL_INT32(17, value_token.parent_position);

    // second object
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_next_object_token(&parser, &array_token, &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_OBJECT, value_token.type);
    TEST_ASSERT_EQUAL_INT32(280, value_token.start_position);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_POSITION_UNDEFINED, value_token.end_position);
    TEST_ASSERT_EQUAL_INT32(17, value_token.parent_position);

    // no third object at this depth
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_OBJECT_NOT_FOUND, jsmn_stream_utils_get_next_object_token(&parser, &array_token, &value_token));
}

void test_jsmn_stream_utils_get_object_token_containing_kv(void)
{
    new_jsmn_stream_token_parser_t parser;
    jsmn_stream_token_t value_token;

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(json_data), (void *)json_data));

    // first object
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_object_token_containing_kv(&parser, "id", "1234", &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_OBJECT, value_token.type);
    TEST_ASSERT_EQUAL_INT32(23, value_token.start_position);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_POSITION_UNDEFINED, value_token.end_position);
    TEST_ASSERT_EQUAL_INT32(17, value_token.parent_position);

    // second object
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_object_token_containing_kv(&parser, "id", "5678", &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_OBJECT, value_token.type);
    TEST_ASSERT_EQUAL_INT32(280, value_token.start_position);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_POSITION_UNDEFINED, value_token.end_position);
    // TEST_ASSERT_EQUAL_INT32(17, value_token.parent_position); 

    // no object with this key/value pair
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_OBJECT_NOT_FOUND, jsmn_stream_utils_get_object_token_containing_kv(&parser, "id", "9999", &value_token));
}

void test_jsmn_stream_utils_get_bool_from_token(void)
{
    new_jsmn_stream_token_parser_t parser;
    jsmn_stream_token_t value_token;
    bool value;

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(json_data), (void *)json_data));

    // find "enabled": true
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, "enabled", &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_bool_from_token(&parser, &value_token, &value));
    TEST_ASSERT_EQUAL_INT32(true, value);
}

void test_jsmn_stream_utils_get_bool_by_key(void)
{
    new_jsmn_stream_token_parser_t parser;
    bool value;

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(json_data), (void *)json_data));

    // find "enabled": true
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_bool_by_key(&parser, "enabled", &value));
    TEST_ASSERT_EQUAL_INT32(true, value);
}

void test_jsmn_stream_utils_get_int_from_token(void)
{
    new_jsmn_stream_token_parser_t parser;
    jsmn_stream_token_t value_token;
    int32_t value;

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(json_data), (void *)json_data));

    // find "offset": -3
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, "offset", &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_int_from_token(&parser, &value_token, &value));
    TEST_ASSERT_EQUAL_INT32(-3, value);
}

void test_jsmn_stream_utils_get_int_by_key(void)
{
    new_jsmn_stream_token_parser_t parser;
    int32_t value;

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(json_data), (void *)json_data));

    // find "offset": -3
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_int_by_key(&parser, "offset", &value));
    TEST_ASSERT_EQUAL_INT32(-3, value);
}

void test_jsmn_stream_utils_get_uint_from_token(void)
{
    new_jsmn_stream_token_parser_t parser;
    jsmn_stream_token_t value_token;
    uint32_t value;

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(json_data), (void *)json_data));

    // find "id": 1234
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, "id", &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_uint_from_token(&parser, &value_token, &value));
    TEST_ASSERT_EQUAL_INT32(1234, value);
}

void test_jsmn_stream_utils_get_uint_by_key(void)
{
    new_jsmn_stream_token_parser_t parser;
    uint32_t value;

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(json_data), (void *)json_data));
    
    // find "id": 1234
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_uint_by_key(&parser, "id", &value));
    TEST_ASSERT_EQUAL_INT32(1234, value);
}

void test_jsmn_stream_utils_get_double_from_token(void)
{
    new_jsmn_stream_token_parser_t parser;
    jsmn_stream_token_t value_token;
    double value;

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(json_data), (void *)json_data));
    
    // find "period": 50.5
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, "period", &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_double_from_token(&parser, &value_token, &value));
    TEST_ASSERT_EQUAL_DOUBLE(50.5, value);
}

void test_jsmn_stream_utils_get_double_by_key(void)
{
    new_jsmn_stream_token_parser_t parser;
    double value;

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(json_data), (void *)json_data));
    
    // find "period": 50.5
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_double_by_key(&parser, "period", &value));
    TEST_ASSERT_EQUAL_DOUBLE(50.5, value);
}

void test_jsmn_stream_utils_get_string_from_token(void)
{
    new_jsmn_stream_token_parser_t parser;
    jsmn_stream_token_t value_token;
    char buffer[32];

    memset(buffer, 0, sizeof(buffer));

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(json_data), (void *)json_data));
    
    // find "label": "instance of pwm"
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, "label", &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_string_from_token(&parser, &value_token, buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING("instance of pwm", buffer);
}

void test_jsmn_stream_utils_get_string_by_key(void)
{
    new_jsmn_stream_token_parser_t parser;
    char buffer[32];

    memset(buffer, 0, sizeof(buffer));

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(json_data), (void *)json_data));
    
    // find "label": "instance of pwm"
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_string_by_key(&parser, "label", buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING("instance of pwm", buffer);
}