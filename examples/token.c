#include "../jsmn_stream_token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * A small example of jsmn-stream token parsing when the JSON structure is known and number of
 * tokens is predictable.
 * 
 * Say you are using an embedded system and cannot fit all of the JSON in memory.
 */

#define MAX_TOKENS (32U)
#define MAX_BUFFER_SIZE (16U)

const char *json_data =
    "{"
        "\"user\": \"johndoe\", "
        "\"admin\": false, "
        "\"uid\": 1000,"
        "\"groups\": ["
            "\"users\", "
            "\"wheel\", "
            "\"audio\", "
            "\"video\""
        "]"
    "}";

/**
 * @brief Pretend that you have a big file in flash or you are reading over a network.
 * 
 * @param start_index 
 * @param size in bytes
 * @param buffer 
 */
void pretend_to_read_a_file(int start_index, int size, char *buffer)
{
    for (int i = 0; i < size; i++)
    {
        buffer[i] = json_data[start_index + i];
    }
}

int main(void)
{
    jsmn_stream_token_parser_t token_parser = {0};
    jsmn_streamtok_t tokens[MAX_TOKENS] = {0};
    int json_size = strlen(json_data);

    jsmn_stream_parse_tokens_init(&token_parser, tokens, MAX_TOKENS);

    // Get your tokens first.
    for (int i = 0; i < json_size; i++)
    { 
        char byte = 0;
        pretend_to_read_a_file(i, 1, &byte);

        if (jsmn_stream_parse_tokens(&token_parser, byte) != JSMN_STREAM_TOKEN_ERROR_NONE)
        {
            return 1;
        }
    }

    // Now you can use the tokens to get smaller chunks of data.
    for (int i = 0; i < MAX_TOKENS; i++)
    {
        char buffer[MAX_BUFFER_SIZE] = {0};
        int length = tokens[i].end - tokens[i].start;

        if (length < MAX_BUFFER_SIZE)
        {
            pretend_to_read_a_file(tokens[i].start, length, buffer);

            if (tokens[i].type == JSMN_STREAM_KEY)
            {
                // check if this is the key you are looking for.
            }
            else if (tokens[i].type == JSMN_STREAM_STRING)
            {
                // do something with the string.
            }
            else if (tokens[i].type == JSMN_STREAM_PRIMITIVE)
            {
                // check if it is boolean or numeric
                bool bool_value = false;
                double double_value = 0.0;

                if (strcasecmp(buffer, "true") == 0)
                {
                    bool_value = true;
                }
                else if (strcasecmp(buffer, "false") == 0)
                {
                    bool_value = false;
                }
                else
                {
                    double_value = strtod(buffer, NULL);
                }
            }
        }
    }

    return EXIT_SUCCESS;
}
