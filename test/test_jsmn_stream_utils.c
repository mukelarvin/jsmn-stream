#include "unity.h"
#define JSMN_PARENT_LINKS
#define JSMN_COMPATIBILITY_MODE 1

/* The module to test */
#include "jsmn_stream_token_utils.h"
#include "jsmn_stream_token.h"
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
    "            \"enabled\": \"false\"\n"
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

void test_jsmn_stream_token_utils_parse_with_cb_no_mem(void)
{
    jsmn_stream_token_parser_t parser;
    parser.cb = get_char_cb;
    parser.user_arg = (void *)json_data;
    jsmn_streamtok_t tokens[30];
    jsmn_stream_parse_tokens_init(&parser, tokens, 30);

    TEST_ASSERT_EQUAL(JSMN_STREAM_TOKEN_ERROR_NOMEM, jsmn_stream_token_utils_parse_with_cb(&parser, strlen(json_data)));
}

void test_jsmn_stream_token_utils_parse_with_cb_success(void)
{
    jsmn_stream_token_parser_t parser;
    parser.cb = get_char_cb;
    parser.user_arg = (void *)json_data;
    jsmn_streamtok_t tokens[100];
    jsmn_stream_parse_tokens_init(&parser, tokens, 100);

    TEST_ASSERT_EQUAL(JSMN_STREAM_TOKEN_ERROR_NONE, jsmn_stream_token_utils_parse_with_cb(&parser, strlen(json_data)));
}

/**
 * @brief Key search is currently case sensitive.
 * 
 */
void test_jsmn_stream_token_utils_get_value_token_by_key_not_found(void)
{
    jsmn_stream_token_parser_t parser;
    parser.cb = get_char_cb;
    parser.user_arg = (void *)json_data;
    jsmn_streamtok_t tokens[100];
    jsmn_streamtok_t *value_token;
    jsmn_stream_parse_tokens_init(&parser, tokens, 100);
    jsmn_stream_token_utils_parse_with_cb(&parser, strlen(json_data));

    TEST_ASSERT_EQUAL(JSMN_STREAM_TOKEN_UTILS_ERROR_KEY_NOT_FOUND, jsmn_stream_token_utils_get_value_token_by_key(&parser, tokens, "ID", &value_token));
}

void test_jsmn_stream_token_utils_get_value_token_by_key_success(void)
{
    jsmn_stream_token_parser_t parser;
    parser.cb = get_char_cb;
    parser.user_arg = (void *)json_data;
    jsmn_streamtok_t tokens[100];
    jsmn_streamtok_t *value_token;
    jsmn_stream_parse_tokens_init(&parser, tokens, 100);
    jsmn_stream_token_utils_parse_with_cb(&parser, strlen(json_data));

    TEST_ASSERT_EQUAL(JSMN_STREAM_TOKEN_ERROR_NONE, jsmn_stream_token_utils_get_value_token_by_key(&parser, tokens, "id", &value_token));
}

void test_jsmn_stream_token_utils_array_get_next_object_token_success(void)
{
    jsmn_stream_token_parser_t parser;
    parser.cb = get_char_cb;
    parser.user_arg = (void *)json_data;
    jsmn_streamtok_t tokens[100];
    jsmn_streamtok_t *array_token = NULL;
    jsmn_streamtok_t *iterator_token = NULL;
    jsmn_stream_parse_tokens_init(&parser, tokens, 100);
    jsmn_stream_token_utils_parse_with_cb(&parser, strlen(json_data));
    jsmn_stream_token_utils_get_value_token_by_key(&parser, tokens,"operations", &array_token);
    TEST_ASSERT_EQUAL(JSMN_STREAM_TOKEN_ERROR_NONE, jsmn_stream_token_utils_array_get_next_object_token(&parser, array_token, &iterator_token));
    TEST_ASSERT_EQUAL(JSMN_STREAM_OBJECT, iterator_token->type);
}

void test_jsmn_stream_token_utils_get_string_from_token(void)
{
    jsmn_stream_token_parser_t parser;
    parser.cb = get_char_cb;
    parser.user_arg = (void *)json_data;
    jsmn_streamtok_t tokens[100];
    jsmn_streamtok_t *value_token;
    char buffer[100] = {0};
    jsmn_stream_parse_tokens_init(&parser, tokens, 100);
    jsmn_stream_token_utils_parse_with_cb(&parser, strlen(json_data));
    jsmn_stream_token_utils_get_value_token_by_key(&parser, tokens, "class", &value_token);
    TEST_ASSERT_EQUAL(JSMN_STREAM_TOKEN_ERROR_NONE, jsmn_stream_token_utils_get_string_from_token(&parser, value_token, buffer));
    TEST_ASSERT_EQUAL_STRING("pwm", buffer);
}

void test_jsmn_stream_token_utils_get_int_from_token(void)
{
    jsmn_stream_token_parser_t parser;
    parser.cb = get_char_cb;
    parser.user_arg = (void *)json_data;
    jsmn_streamtok_t tokens[100];
    jsmn_streamtok_t *value_token;
    int32_t value;
    jsmn_stream_parse_tokens_init(&parser, tokens, 100);
    jsmn_stream_token_utils_parse_with_cb(&parser, strlen(json_data));
    jsmn_stream_token_utils_get_value_token_by_key(&parser, tokens, "id", &value_token);
    TEST_ASSERT_EQUAL(JSMN_STREAM_TOKEN_ERROR_NONE, jsmn_stream_token_utils_get_int_from_token(&parser, value_token, &value));
    TEST_ASSERT_EQUAL(1234, value);
}

void test_jsmn_stream_token_utils_get_double_from_token(void)
{
    jsmn_stream_token_parser_t parser;
    parser.cb = get_char_cb;
    parser.user_arg = (void *)json_data;
    jsmn_streamtok_t tokens[100];
    jsmn_streamtok_t *value_token;
    double value;
    jsmn_stream_parse_tokens_init(&parser, tokens, 100);
    jsmn_stream_token_utils_parse_with_cb(&parser, strlen(json_data));
    jsmn_stream_token_utils_get_value_token_by_key(&parser, tokens, "period", &value_token);
    TEST_ASSERT_EQUAL(JSMN_STREAM_TOKEN_ERROR_NONE, jsmn_stream_token_utils_get_double_from_token(&parser, value_token, &value));
    TEST_ASSERT_EQUAL_DOUBLE(50.5, value);
}

void test_jsmn_stream_token_utils_get_bool_from_token(void)
{
    jsmn_stream_token_parser_t parser;
    parser.cb = get_char_cb;
    parser.user_arg = (void *)json_data;
    jsmn_streamtok_t tokens[100];
    jsmn_streamtok_t *value_token;
    bool value;
    jsmn_stream_parse_tokens_init(&parser, tokens, 100);
    jsmn_stream_token_utils_parse_with_cb(&parser, strlen(json_data));
    jsmn_stream_token_utils_get_value_token_by_key(&parser, tokens, "enabled", &value_token);
    TEST_ASSERT_EQUAL(JSMN_STREAM_TOKEN_ERROR_NONE, jsmn_stream_token_utils_get_bool_from_token(&parser, value_token, &value));
    TEST_ASSERT_EQUAL(false, value);
}

/**
 * @brief Find the object containing the key and value, "id": 5678 at index 21
 * 
 */
void test_jsmn_stream_token_utils_get_object_containing_key_int_success(void)
{
    jsmn_stream_token_parser_t parser;
    parser.cb = get_char_cb;
    parser.user_arg = (void *)json_data;
    jsmn_streamtok_t tokens[100];
    jsmn_streamtok_t *object_token = NULL;
    char buffer[100] = {0};
    int32_t value;

    jsmn_stream_parse_tokens_init(&parser, tokens, 100);
    jsmn_stream_token_utils_parse_with_cb(&parser, strlen(json_data));
    
    TEST_ASSERT_EQUAL(JSMN_STREAM_TOKEN_ERROR_NONE, jsmn_stream_token_utils_get_object_containing_key_int(&parser, tokens, "id", 5678, &object_token));
    TEST_ASSERT_EQUAL_PTR(&tokens[21], object_token);
    jsmn_stream_token_utils_get_string_from_token(&parser, object_token, buffer);
    jsmn_stream_token_utils_get_int_from_token(&parser, object_token + 1, &value);
    TEST_ASSERT_EQUAL_STRING("id", buffer);
    TEST_ASSERT_EQUAL(5678, value);
}