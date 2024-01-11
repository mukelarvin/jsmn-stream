#include "unity.h"
#define JSMN_PARENT_LINKS
#define JSMN_COMPATIBILITY_MODE 1

/* The module to test */
#include "jsmn_stream_utils.h"
#include "jsmn_stream.h"
#include <stdint.h>
#include <string.h>

const char* global_json_data = "{\n"
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
    jsmn_stream_token_parser_t parser;
    jsmn_stream_token_t value_token;
 
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(global_json_data), (void *)global_json_data));

    // first key
    jsmn_stream_utils_init_token(0);
    jsmn_stream_utils_init_token(&value_token);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, 0, "operations", &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_ARRAY, value_token.type);
    TEST_ASSERT_EQUAL_INT32(17, value_token.start_position);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_POSITION_UNDEFINED, value_token.end_position);
    TEST_ASSERT_EQUAL_INT32(4, value_token.parent_position);

    // first primitive
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, 0, "id", &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_PRIMITIVE, value_token.type);
    TEST_ASSERT_EQUAL_INT32(39, value_token.start_position);
    TEST_ASSERT_EQUAL_INT32(43, value_token.end_position);
    TEST_ASSERT_EQUAL_INT32(34, value_token.parent_position);

    // first string
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, 0, "class", &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_STRING, value_token.type);
    TEST_ASSERT_EQUAL_INT32(85, value_token.start_position);
    TEST_ASSERT_EQUAL_INT32(88, value_token.end_position);
    TEST_ASSERT_EQUAL_INT32(76, value_token.parent_position);

    // second instance of "id"
    // use the previous token which was after the first instance of "id"
    jsmn_stream_token_t second_id_token;
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, value_token.start_position, "id", &second_id_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_STRING, second_id_token.type);
    TEST_ASSERT_EQUAL_INT32(189, second_id_token.start_position);
    TEST_ASSERT_EQUAL_INT32(193, second_id_token.end_position);
    TEST_ASSERT_EQUAL_INT32(183, second_id_token.parent_position);

    // a string deeper down the tree
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, 0, "enabled", &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_STRING, value_token.type);
    TEST_ASSERT_EQUAL_INT32(500, value_token.start_position);
    TEST_ASSERT_EQUAL_INT32(504, value_token.end_position);
    TEST_ASSERT_EQUAL_INT32(489, value_token.parent_position);

    // a string that doesn't exist
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_KEY_NOT_FOUND, jsmn_stream_utils_get_value_token_by_key(&parser, 0, "does not exist", &value_token));
}

void test_jsmn_stream_utils_array_get_size(void)
{
    jsmn_stream_token_parser_t parser;
    jsmn_stream_token_t array_token;
    uint32_t size;

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(global_json_data), (void *)global_json_data));

    // get the array from the first key
    jsmn_stream_utils_init_token(&array_token);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, 0, "operations", &array_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_ARRAY, array_token.type);

    // get the size of the array
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_array_get_size(&parser, &array_token, &size));
    TEST_ASSERT_EQUAL_INT32(2, size);
}

void test_jsmn_stream_utils_array_get_next_object_token(void)
{
    jsmn_stream_token_parser_t parser;
    jsmn_stream_token_t array_token;
    jsmn_stream_token_t value_token;

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(global_json_data), (void *)global_json_data));

    // get the array from the first key
    jsmn_stream_utils_init_token(&value_token);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, 0, "operations", &array_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_ARRAY, array_token.type);
    TEST_ASSERT_EQUAL_INT32(17, array_token.start_position);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_POSITION_UNDEFINED, array_token.end_position);
    TEST_ASSERT_EQUAL_INT32(4, array_token.parent_position);

    // first object
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_array_get_next_object_token(&parser, &array_token, &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_OBJECT, value_token.type);
    TEST_ASSERT_EQUAL_INT32(23, value_token.start_position);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_POSITION_UNDEFINED, value_token.end_position);
    TEST_ASSERT_EQUAL_INT32(17, value_token.parent_position);

    // second object
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_array_get_next_object_token(&parser, &array_token, &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_OBJECT, value_token.type);
    TEST_ASSERT_EQUAL_INT32(280, value_token.start_position);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_POSITION_UNDEFINED, value_token.end_position);
    TEST_ASSERT_EQUAL_INT32(17, value_token.parent_position);

    // no third object at this depth
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_OBJECT_NOT_FOUND, jsmn_stream_utils_array_get_next_object_token(&parser, &array_token, &value_token));
}

void test_jsmn_stream_utils_object_get_size(void)
{
    jsmn_stream_token_parser_t parser;
    jsmn_stream_token_t array_token;
    jsmn_stream_token_t object_token;
    uint32_t size;

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(global_json_data), (void *)global_json_data));

    // get the first object from the Operations array
    jsmn_stream_utils_init_token(&object_token);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, 0, "operations", &array_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_array_get_next_object_token(&parser, &array_token, &object_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_OBJECT, object_token.type);

    // get the size of the object
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_object_get_size(&parser, &object_token, &size));
    TEST_ASSERT_EQUAL_INT32(5, size);
}

void test_jsmn_stream_utils_object_get_next_kv_tokens(void)
{
    jsmn_stream_token_parser_t parser;
    jsmn_stream_token_t object_token = {0};
    jsmn_stream_token_t key_token = {0};
    jsmn_stream_token_t value_token = {0};

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(global_json_data), (void *)global_json_data));

    // get the array from the first key
    jsmn_stream_utils_init_token(&value_token);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, 0, "operation properties", &object_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_OBJECT, object_token.type);
    TEST_ASSERT_EQUAL_INT32(168, object_token.start_position);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_POSITION_UNDEFINED, object_token.end_position);
    TEST_ASSERT_EQUAL_INT32(136, object_token.parent_position);

    // first key/value pair
    jsmn_stream_utils_init_token(&key_token);
    jsmn_stream_utils_init_token(&value_token);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_object_get_next_kv_tokens(&parser, &object_token, &key_token, &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_KEY, key_token.type);
    TEST_ASSERT_EQUAL_INT32(183, key_token.start_position);
    TEST_ASSERT_EQUAL_INT32(185, key_token.end_position);
    TEST_ASSERT_EQUAL_INT32(168, key_token.parent_position);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_STRING, value_token.type);
    TEST_ASSERT_EQUAL_INT32(189, value_token.start_position);
    TEST_ASSERT_EQUAL_INT32(193, value_token.end_position);
    TEST_ASSERT_EQUAL_INT32(168, value_token.parent_position);

    // second key/value pair
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_object_get_next_kv_tokens(&parser, &object_token, &key_token, &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_KEY, key_token.type);
    TEST_ASSERT_EQUAL_INT32(209, key_token.start_position);
    TEST_ASSERT_EQUAL_INT32(215, key_token.end_position);
    TEST_ASSERT_EQUAL_INT32(168, key_token.parent_position);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_PRIMITIVE, value_token.type);
    TEST_ASSERT_EQUAL_INT32(218, value_token.start_position);
    TEST_ASSERT_EQUAL_INT32(222, value_token.end_position);
    TEST_ASSERT_EQUAL_INT32(168, value_token.parent_position);

    // third key/value pair
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_object_get_next_kv_tokens(&parser, &object_token, &key_token, &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_KEY, key_token.type);
    TEST_ASSERT_EQUAL_INT32(237, key_token.start_position);
    TEST_ASSERT_EQUAL_INT32(245, key_token.end_position);
    TEST_ASSERT_EQUAL_INT32(168, key_token.parent_position);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_STRING, value_token.type);
    TEST_ASSERT_EQUAL_INT32(249, value_token.start_position);
    TEST_ASSERT_EQUAL_INT32(257, value_token.end_position);
    TEST_ASSERT_EQUAL_INT32(168, value_token.parent_position);

    // no fourth key/value pair
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_OBJECT_NOT_FOUND, jsmn_stream_utils_object_get_next_kv_tokens(&parser, &object_token, &key_token, &value_token));    
}

void test_jsmn_stream_utils_get_object_token_containing_kv(void)
{
    jsmn_stream_token_parser_t parser;
    jsmn_stream_token_t value_token;

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(global_json_data), (void *)global_json_data));

    // first object
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_object_token_containing_kv(&parser, 0, "id", "1234", &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_OBJECT, value_token.type);
    TEST_ASSERT_EQUAL_INT32(23, value_token.start_position);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_POSITION_UNDEFINED, value_token.end_position);
    TEST_ASSERT_EQUAL_INT32(17, value_token.parent_position);

    // second object
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_object_token_containing_kv(&parser, 0, "id", "5678", &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_OBJECT, value_token.type);
    TEST_ASSERT_EQUAL_INT32(280, value_token.start_position);
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_POSITION_UNDEFINED, value_token.end_position);
    // TEST_ASSERT_EQUAL_INT32(17, value_token.parent_position); 

    // no object with this key/value pair
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_OBJECT_NOT_FOUND, jsmn_stream_utils_get_object_token_containing_kv(&parser, 0, "id", "9999", &value_token));
}

void test_jsmn_stream_utils_get_bool_from_token(void)
{
    jsmn_stream_token_parser_t parser;
    jsmn_stream_token_t value_token;
    bool value;

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(global_json_data), (void *)global_json_data));

    // find "enabled": true
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, 0, "enabled", &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_bool_from_token(&parser, &value_token, &value));
    TEST_ASSERT_EQUAL_INT32(true, value);
}

void test_jsmn_stream_utils_get_bool_by_key(void)
{
    jsmn_stream_token_parser_t parser;
    bool value;

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(global_json_data), (void *)global_json_data));

    // find "enabled": true
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_bool_by_key(&parser, 0,  "enabled", &value));
    TEST_ASSERT_EQUAL_INT32(true, value);
}

void test_jsmn_stream_utils_get_int_from_token(void)
{
    jsmn_stream_token_parser_t parser;
    jsmn_stream_token_t value_token;
    int32_t value;

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(global_json_data), (void *)global_json_data));

    // find "offset": -3
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, 0, "offset", &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_int_from_token(&parser, &value_token, &value));
    TEST_ASSERT_EQUAL_INT32(-3, value);
}

void test_jsmn_stream_utils_get_int_by_key(void)
{
    jsmn_stream_token_parser_t parser;
    int32_t value;

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(global_json_data), (void *)global_json_data));

    // find "offset": -3
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_int_by_key(&parser, 0, "offset", &value));
    TEST_ASSERT_EQUAL_INT32(-3, value);
}

void test_jsmn_stream_utils_get_uint_from_token(void)
{
    jsmn_stream_token_parser_t parser;
    jsmn_stream_token_t value_token;
    uint32_t value;

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(global_json_data), (void *)global_json_data));

    // find "id": 1234
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, 0, "id", &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_uint_from_token(&parser, &value_token, &value));
    TEST_ASSERT_EQUAL_INT32(1234, value);
}

void test_jsmn_stream_utils_get_uint_by_key(void)
{
    jsmn_stream_token_parser_t parser;
    uint32_t value;

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(global_json_data), (void *)global_json_data));
    
    // find "id": 1234
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_uint_by_key(&parser, 0, "id", &value));
    TEST_ASSERT_EQUAL_INT32(1234, value);
}

void test_jsmn_stream_utils_get_double_from_token(void)
{
    jsmn_stream_token_parser_t parser;
    jsmn_stream_token_t value_token;
    double value;

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(global_json_data), (void *)global_json_data));
    
    // find "period": 50.5
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, 0, "period", &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_double_from_token(&parser, &value_token, &value));
    TEST_ASSERT_EQUAL_DOUBLE(50.5, value);
}

void test_jsmn_stream_utils_get_double_by_key(void)
{
    jsmn_stream_token_parser_t parser;
    double value;

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(global_json_data), (void *)global_json_data));
    
    // find "period": 50.5
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_double_by_key(&parser, 0, "period", &value));
    TEST_ASSERT_EQUAL_DOUBLE(50.5, value);
}

void test_jsmn_stream_utils_get_string_from_token(void)
{
    jsmn_stream_token_parser_t parser;
    jsmn_stream_token_t value_token;
    char buffer[32];

    memset(buffer, 0, sizeof(buffer));

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(global_json_data), (void *)global_json_data));
    
    // find "label": "instance of pwm"
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_value_token_by_key(&parser, 0, "label", &value_token));
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_string_from_token(&parser, &value_token, buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING("instance of pwm", buffer);
}

void test_jsmn_stream_utils_get_string_by_key(void)
{
    jsmn_stream_token_parser_t parser;
    char buffer[32];

    memset(buffer, 0, sizeof(buffer));

    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_init_parser(&parser, get_char_cb, strlen(global_json_data), (void *)global_json_data));
    
    // find "label": "instance of pwm"
    TEST_ASSERT_EQUAL_INT32(JSMN_STREAM_UTILS_ERROR_NONE, jsmn_stream_utils_get_string_by_key(&parser, 0, "label", buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING("instance of pwm", buffer);
}