#include "unity.h"
#define JSMN_PARENT_LINKS
#define JSMN_COMPATIBILITY_MODE 1

/* The module to test */
#include "jsmn_stream_token.h"
#include "jsmn_stream.h"
#include <string.h>


const char* json_data =
"{"
    "\"glossary\": {"
        "\"title\": \"example glossary\","
        "\"GlossDiv\": {"
            "\"title\": \"S\","
            "\"GlossList\": {"
                "\"GlossEntry\": {"
                    "\"ID\": \"SGML\","
                    "\"SortAs\": \"SGML\","
                    "\"GlossTerm\": \"Standard Generalized Markup Language\","
                    "\"Acronym\": \"SGML\","
                    "\"Abbrev\": \"ISO 8879:1986\","
                    "\"GlossDef\": {"
                        "\"para\": \"A meta-markup language, used to create markup languages such as DocBook.\","
                        "\"GlossSeeAlso\": [\"GML\", \"XML\"]"
                    "},"
                    "\"GlossSee\": \"markup\""
                "}"
            "}"
        "}"
    "}"
"}";


void setUp(void)
{

}

void tearDown(void)
{

}

void test_jsmn_stream_parse_tokens_init(void)
{
    jsmn_stream_token_parser_t parser;
    jsmn_streamtok_t tokens[32];

    jsmn_stream_parse_tokens_init(&parser, tokens, 32);

    TEST_ASSERT_EQUAL_PTR(tokens, parser.tokens);
    TEST_ASSERT_EQUAL(0, parser.next_token);
    TEST_ASSERT_EQUAL(32, parser.num_tokens);
    TEST_ASSERT_EQUAL(0, parser.char_count);
    TEST_ASSERT_EQUAL(-1, parser.super_token_id);
    TEST_ASSERT_EQUAL_PTR(tokens, parser.tokens);
    TEST_ASSERT_EQUAL_PTR(&parser, parser.stream_parser.user_arg);
}

int parse_tokens_helper(jsmn_stream_token_parser_t *parser, jsmn_streamtok_t *tokens, int num_tokens, char *json_data)
{
    char *p = json_data;

    jsmn_stream_parse_tokens_init(parser, tokens, num_tokens);

    for (int i = 0; i < strlen(json_data); i++)
    {
        jsmn_stream_parse_tokens(parser, *p++);
        if (parser->error != JSMN_STREAM_TOKEN_ERROR_NONE)
        {
            return -1;
        }
    }

    return 0;
}

void test_empty_object(void)
{
    jsmn_stream_token_parser_t parser;
    jsmn_streamtok_t tokens[1];

    parse_tokens_helper(&parser, tokens, 1, "{}");
    
    TEST_ASSERT_EQUAL(JSMN_STREAM_TOKEN_ERROR_NONE, parser.error);
    TEST_ASSERT_EQUAL(0, tokens[0].id);
    TEST_ASSERT_EQUAL(JSMN_STREAM_OBJECT, tokens[0].type);
    TEST_ASSERT_EQUAL(0, tokens[0].start);
    TEST_ASSERT_EQUAL(2, tokens[0].end);
    TEST_ASSERT_EQUAL(0, tokens[0].size);
    TEST_ASSERT_EQUAL(-1, tokens[0].parent_id);
}

void test_empty_array(void)
{
    jsmn_stream_token_parser_t parser;
    jsmn_streamtok_t tokens[1];

    parse_tokens_helper(&parser, tokens, 1, "[]");
    
    TEST_ASSERT_EQUAL(JSMN_STREAM_TOKEN_ERROR_NONE, parser.error);
    TEST_ASSERT_EQUAL(0, tokens[0].id);
    TEST_ASSERT_EQUAL(JSMN_STREAM_ARRAY, tokens[0].type);
    TEST_ASSERT_EQUAL(0, tokens[0].start);
    TEST_ASSERT_EQUAL(2, tokens[0].end);
    TEST_ASSERT_EQUAL(0, tokens[0].size);
    TEST_ASSERT_EQUAL(-1, tokens[0].parent_id);
}

void test_array_with_two_empty_objects(void)
{
    jsmn_stream_token_parser_t parser;
    jsmn_streamtok_t tokens[3];

    parse_tokens_helper(&parser, tokens, 3, "[{},{}]");
    
    TEST_ASSERT_EQUAL(JSMN_STREAM_TOKEN_ERROR_NONE, parser.error);
    TEST_ASSERT_EQUAL(0, tokens[0].id);
    TEST_ASSERT_EQUAL(JSMN_STREAM_ARRAY, tokens[0].type);
    TEST_ASSERT_EQUAL(0, tokens[0].start);
    TEST_ASSERT_EQUAL(7, tokens[0].end);
    TEST_ASSERT_EQUAL(2, tokens[0].size);
    TEST_ASSERT_EQUAL(-1, tokens[0].parent_id);

    TEST_ASSERT_EQUAL(1, tokens[1].id);
    TEST_ASSERT_EQUAL(JSMN_STREAM_OBJECT, tokens[1].type);
    TEST_ASSERT_EQUAL(1, tokens[1].start);
    TEST_ASSERT_EQUAL(3, tokens[1].end);
    TEST_ASSERT_EQUAL(0, tokens[1].size);
    TEST_ASSERT_EQUAL(0, tokens[1].parent_id);

    TEST_ASSERT_EQUAL(2, tokens[2].id);
    TEST_ASSERT_EQUAL(JSMN_STREAM_OBJECT, tokens[2].type);
    TEST_ASSERT_EQUAL(4, tokens[2].start);
    TEST_ASSERT_EQUAL(6, tokens[2].end);
    TEST_ASSERT_EQUAL(0, tokens[2].size);
    TEST_ASSERT_EQUAL(0, tokens[2].parent_id);
}

void test_object_with_two_kv_pairs(void)
{
    char *json = "{\"key1\":\"value1\", \"key2\":\"value2\"}";

    jsmn_stream_token_parser_t parser;
    jsmn_streamtok_t tokens[5];

    parse_tokens_helper(&parser, tokens, 5, json);

    TEST_ASSERT_EQUAL(JSMN_STREAM_TOKEN_ERROR_NONE, parser.error);
    TEST_ASSERT_EQUAL(0, tokens[0].id);
    TEST_ASSERT_EQUAL(JSMN_STREAM_OBJECT, tokens[0].type);
    TEST_ASSERT_EQUAL(0, tokens[0].start);
    TEST_ASSERT_EQUAL(34, tokens[0].end);
    TEST_ASSERT_EQUAL(2, tokens[0].size);
    TEST_ASSERT_EQUAL(-1, tokens[0].parent_id);

    TEST_ASSERT_EQUAL(1, tokens[1].id);
    TEST_ASSERT_EQUAL(JSMN_STREAM_KEY, tokens[1].type);
    TEST_ASSERT_EQUAL(2, tokens[1].start);
    TEST_ASSERT_EQUAL(6, tokens[1].end);
    TEST_ASSERT_EQUAL(1, tokens[1].size);
    TEST_ASSERT_EQUAL(0, tokens[1].parent_id);

    TEST_ASSERT_EQUAL(2, tokens[2].id);
    TEST_ASSERT_EQUAL(JSMN_STREAM_STRING, tokens[2].type);
    TEST_ASSERT_EQUAL(9, tokens[2].start);
    TEST_ASSERT_EQUAL(15, tokens[2].end);
    TEST_ASSERT_EQUAL(0, tokens[2].size); 
    TEST_ASSERT_EQUAL(1, tokens[2].parent_id);

    TEST_ASSERT_EQUAL(3, tokens[3].id);
    TEST_ASSERT_EQUAL(JSMN_STREAM_KEY, tokens[3].type);
    TEST_ASSERT_EQUAL(19, tokens[3].start);
    TEST_ASSERT_EQUAL(23, tokens[3].end);
    TEST_ASSERT_EQUAL(1, tokens[3].size);
    TEST_ASSERT_EQUAL(0, tokens[3].parent_id);

    TEST_ASSERT_EQUAL(4, tokens[4].id);
    TEST_ASSERT_EQUAL(JSMN_STREAM_STRING, tokens[4].type);
    TEST_ASSERT_EQUAL(26, tokens[4].start);
    TEST_ASSERT_EQUAL(32, tokens[4].end);
    TEST_ASSERT_EQUAL(0, tokens[4].size);
    TEST_ASSERT_EQUAL(3, tokens[4].parent_id);
}

void test_object_with_nested_objects(void)
{
    char *json = "{\"key1\":\"value1\", \"key2\":{\"key3\":\"value3\"}, \"key4\":\"value4\"}";

    jsmn_stream_token_parser_t parser;
    jsmn_streamtok_t tokens[9];

    parse_tokens_helper(&parser, tokens, 9, json);

    TEST_ASSERT_EQUAL(JSMN_STREAM_TOKEN_ERROR_NONE, parser.error);
    TEST_ASSERT_EQUAL(0, tokens[0].id);
    TEST_ASSERT_EQUAL(JSMN_STREAM_OBJECT, tokens[0].type);
    TEST_ASSERT_EQUAL(0, tokens[0].start);
    TEST_ASSERT_EQUAL(60, tokens[0].end);
    TEST_ASSERT_EQUAL(3, tokens[0].size);
    TEST_ASSERT_EQUAL(-1, tokens[0].parent_id);

    TEST_ASSERT_EQUAL(1, tokens[1].id);
    TEST_ASSERT_EQUAL(JSMN_STREAM_KEY, tokens[1].type);
    TEST_ASSERT_EQUAL(2, tokens[1].start);
    TEST_ASSERT_EQUAL(6, tokens[1].end);
    TEST_ASSERT_EQUAL(1, tokens[1].size);
    TEST_ASSERT_EQUAL(0, tokens[1].parent_id);

    TEST_ASSERT_EQUAL(2, tokens[2].id);
    TEST_ASSERT_EQUAL(JSMN_STREAM_STRING, tokens[2].type);
    TEST_ASSERT_EQUAL(9, tokens[2].start);
    TEST_ASSERT_EQUAL(15, tokens[2].end);
    TEST_ASSERT_EQUAL(0, tokens[2].size);
    TEST_ASSERT_EQUAL(1, tokens[2].parent_id);

    TEST_ASSERT_EQUAL(3, tokens[3].id);
    TEST_ASSERT_EQUAL(JSMN_STREAM_KEY, tokens[3].type);
    TEST_ASSERT_EQUAL(19, tokens[3].start);
    TEST_ASSERT_EQUAL(23, tokens[3].end);
    TEST_ASSERT_EQUAL(1, tokens[3].size);
    TEST_ASSERT_EQUAL(0, tokens[3].parent_id);

    TEST_ASSERT_EQUAL(4, tokens[4].id);
    TEST_ASSERT_EQUAL(JSMN_STREAM_OBJECT, tokens[4].type);
    TEST_ASSERT_EQUAL(25, tokens[4].start);
    TEST_ASSERT_EQUAL(42, tokens[4].end);
    TEST_ASSERT_EQUAL(1, tokens[4].size);
    TEST_ASSERT_EQUAL(3, tokens[4].parent_id);

    TEST_ASSERT_EQUAL(5, tokens[5].id);
    TEST_ASSERT_EQUAL(JSMN_STREAM_KEY, tokens[5].type);
    TEST_ASSERT_EQUAL(27, tokens[5].start);
    TEST_ASSERT_EQUAL(31, tokens[5].end);
    TEST_ASSERT_EQUAL(1, tokens[5].size);
    TEST_ASSERT_EQUAL(4, tokens[5].parent_id);

    TEST_ASSERT_EQUAL(6, tokens[6].id);
    TEST_ASSERT_EQUAL(JSMN_STREAM_STRING, tokens[6].type);
    TEST_ASSERT_EQUAL(34, tokens[6].start);
    TEST_ASSERT_EQUAL(40, tokens[6].end);
    TEST_ASSERT_EQUAL(0, tokens[6].size);
    TEST_ASSERT_EQUAL(5, tokens[6].parent_id);

    TEST_ASSERT_EQUAL(7, tokens[7].id);
    TEST_ASSERT_EQUAL(JSMN_STREAM_KEY, tokens[7].type);
    TEST_ASSERT_EQUAL(45, tokens[7].start);
    TEST_ASSERT_EQUAL(49, tokens[7].end);
    TEST_ASSERT_EQUAL(1, tokens[7].size);
    TEST_ASSERT_EQUAL(0, tokens[7].parent_id);

    TEST_ASSERT_EQUAL(8, tokens[8].id);
    TEST_ASSERT_EQUAL(JSMN_STREAM_STRING, tokens[8].type);
    TEST_ASSERT_EQUAL(52, tokens[8].start);
    TEST_ASSERT_EQUAL(58, tokens[8].end);
    TEST_ASSERT_EQUAL(0, tokens[8].size);
    TEST_ASSERT_EQUAL(7, tokens[8].parent_id);

}

// ** These tests are not active. I used them to confirm
// ** that the we get the same behaviour as the original
// ** jsmn library. I'm leaving this here as a reference.

// void check_tokens_match(jsmntok_t *jsmn, jsmn_streamtok_t *jsmn_stream)
// {
//     // TEST_ASSERT_EQUAL(jsmn->type, jsmn_stream->type);
//     TEST_ASSERT_EQUAL(jsmn->start, jsmn_stream->start);
//     TEST_ASSERT_EQUAL(jsmn->end, jsmn_stream->end);
//     TEST_ASSERT_EQUAL(jsmn->size, jsmn_stream->size);
//     TEST_ASSERT_EQUAL(jsmn->parent, jsmn_stream->parent);
// }

// void test_complex_example(void)
// {
//     jsmn_parser p;
//     jsmntok_t t[100];

//     jsmn_init(&p);
//     int r = jsmn_parse(&p, json_data, strlen(json_data), t, 100);

//     jsmn_stream_token_parser_t parser;
//     jsmn_streamtok_t tokens[100];

//     parse_tokens_helper(&parser, tokens, 100, json_data);

//     TEST_ASSERT_EQUAL(JSMN_STREAM_TOKEN_ERROR_NONE, parser.error);
//     TEST_ASSERT_EQUAL(JSMN_STREAM_OBJECT, tokens[0].type);

//     check_tokens_match(&t[0], &tokens[0]);
// }
