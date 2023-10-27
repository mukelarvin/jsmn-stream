#include "jsmn_stream_token_utils.h"
#include <stdbool.h>
#include <stdlib.h>

static bool string_compare(const char *str1, const char *str2, size_t length);

int32_t jsmn_stream_token_utils_parse_with_cb(jsmn_stream_token_parser_t *parser, jsmn_stream_token_get_char_cb_t get_char_cb, size_t length, void *user_arg)
{
    if ((parser == NULL) || (get_char_cb == NULL))
    {
        return JSMN_STREAM_TOKEN_ERROR_INVALID;
    }

    for (uint32_t i = 0U; i < (uint32_t)length; i++)
    {
        char ch;
        if (get_char_cb(i, 1, user_arg, &ch) == JSMN_STREAM_TOKEN_GET_CHAR_CB_ERROR_NONE)
        {
            if (jsmn_stream_parse_tokens(parser, ch) != JSMN_STREAM_TOKEN_ERROR_NONE)
            {
                return parser->error;
            }
        }
    }
    return JSMN_STREAM_TOKEN_ERROR_NONE;
}

int32_t jsmn_stream_token_utils_array_get_next_object_token(jsmn_stream_token_parser_t *parser, jsmn_streamtok_t *array_token, jsmn_streamtok_t **iterator_token)
{
    jsmn_streamtok_t *token;
    if (array_token == NULL)
    {
        return JSMN_STREAM_TOKEN_ERROR_INVALID;
    }

    if (*iterator_token == NULL)
    {
        *iterator_token = array_token;
    }

    token = *iterator_token;

    while (token->id < parser->num_tokens)
    {
        token++;
        if ((token->type == JSMN_STREAM_OBJECT)
            && (token->parent_id == array_token->id))
        {
            *iterator_token = token;
            return JSMN_STREAM_TOKEN_ERROR_NONE;
        }
    }

    return JSMN_STREAM_TOKEN_UTILS_ERROR_OBJECT_NOT_FOUND;
}

int32_t jsmn_stream_token_utils_get_value_token_by_key(jsmn_stream_token_get_char_cb_t get_char_cb, void *user_arg, jsmn_streamtok_t *array_token, uint32_t num_tokens, const char *key, jsmn_streamtok_t **value_token)
{
    if ((get_char_cb == NULL) 
        || (array_token == NULL) 
        || (key == NULL)
        || (value_token == NULL))
    {
        return JSMN_STREAM_TOKEN_ERROR_INVALID;
    }

    for (uint32_t i = 0; i < num_tokens; i++)
    {
        jsmn_streamtok_t *token = array_token + i;

        if (token->type == JSMN_STREAM_KEY)
		{
            size_t string_length = (size_t)(token->end - token->start);
            char buffer[string_length];

            if (get_char_cb(token->start, string_length, user_arg, buffer) == JSMN_STREAM_TOKEN_GET_CHAR_CB_ERROR_NONE)
            {
                if (string_compare(buffer, key, string_length) == true)
                {
                    *value_token = token + 1;
                    return JSMN_STREAM_TOKEN_ERROR_NONE;
                }
            }
        }
    }

    return JSMN_STREAM_TOKEN_UTILS_ERROR_KEY_NOT_FOUND;
}

static bool string_compare(const char *str1, const char *str2, size_t length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        if (str1[i] != str2[i])
        {
            return false;
        }
    }

    return true;
}

int32_t jsmn_stream_token_utils_get_string_from_token(jsmn_stream_token_get_char_cb_t get_char_cb, void *user_arg, jsmn_streamtok_t *token, char *buffer)
{
    if ((get_char_cb == NULL) 
        || (token == NULL) 
        || (buffer == NULL))
    {
        return JSMN_STREAM_TOKEN_ERROR_INVALID;
    }

    size_t string_length = (size_t)(token->end - token->start);
    return get_char_cb(token->start, string_length, user_arg, buffer);
}

int32_t jsmn_stream_token_utils_get_int_from_token(jsmn_stream_token_get_char_cb_t get_char_cb, void *user_arg, jsmn_streamtok_t *token,  int32_t *value)
{
     if ((get_char_cb == NULL) 
        || (token == NULL))
    {
        return JSMN_STREAM_TOKEN_ERROR_INVALID;
    }

    size_t string_length = (size_t)(token->end - token->start);
    char buffer[string_length];
    if (get_char_cb(token->start, string_length, user_arg, buffer) == JSMN_STREAM_TOKEN_GET_CHAR_CB_ERROR_NONE)
    {
        *value = strtol(buffer, NULL, 10);
        return JSMN_STREAM_TOKEN_ERROR_NONE;
    }

    return JSMN_STREAM_TOKEN_UTILS_ERROR_FAIL;
}

int32_t jsmn_stream_token_utils_get_double_from_token(jsmn_stream_token_get_char_cb_t get_char_cb, void *user_arg, jsmn_streamtok_t *token,  double *value)
{
        if ((get_char_cb == NULL) 
            || (token == NULL))
        {
            return JSMN_STREAM_TOKEN_ERROR_INVALID;
        }
    
        size_t string_length = (size_t)(token->end - token->start);
        char buffer[string_length];
        if (get_char_cb(token->start, string_length, user_arg, buffer) == JSMN_STREAM_TOKEN_GET_CHAR_CB_ERROR_NONE)
        {
            *value = strtod(buffer, NULL);
            return JSMN_STREAM_TOKEN_ERROR_NONE;
        }
    
        return JSMN_STREAM_TOKEN_UTILS_ERROR_FAIL;
}

int32_t jsmn_stream_token_utils_get_bool_from_token(jsmn_stream_token_get_char_cb_t get_char_cb, void *user_arg, jsmn_streamtok_t *token,  bool *value)
{
    if ((get_char_cb == NULL) 
        || (token == NULL))
    {
        return JSMN_STREAM_TOKEN_ERROR_INVALID;
    }

    size_t string_length = (size_t)(token->end - token->start);
    char buffer[string_length];
    if (get_char_cb(token->start, string_length, user_arg, buffer) == JSMN_STREAM_TOKEN_GET_CHAR_CB_ERROR_NONE)
    {
        if (string_compare(buffer, "true", string_length) == true)
        {
            *value = true;
            return JSMN_STREAM_TOKEN_ERROR_NONE;
        }
        else if (string_compare(buffer, "false", string_length) == true)
        {
            *value = false;
            return JSMN_STREAM_TOKEN_ERROR_NONE;
        }
    }

    return JSMN_STREAM_TOKEN_UTILS_ERROR_FAIL;
}
