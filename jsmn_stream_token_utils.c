#include "jsmn_stream_token_utils.h"
#include <stdbool.h>
#include <stdlib.h>

static bool string_compare(const char *str1, const char *str2, size_t length);

int32_t jsmn_stream_token_utils_parse_with_cb(jsmn_stream_token_parser_t *parser, size_t length)
{
    if (parser == NULL)
    {
        return JSMN_STREAM_TOKEN_ERROR_INVALID;
    }

    for (uint32_t i = 0U; i < (uint32_t)length; i++)
    {
        char ch;
        if (parser->cb(i, 1, parser->user_arg, &ch) == JSMN_STREAM_TOKEN_GET_CHAR_CB_ERROR_NONE)
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

int32_t jsmn_stream_token_utils_get_token_by_key(jsmn_stream_token_parser_t *parser, jsmn_streamtok_t *parent, const char *key, jsmn_streamtok_t **key_token)
{
    if ((parser == NULL) 
        || (parent == NULL) 
        || (key == NULL)
        || (key_token == NULL))
    {
        return JSMN_STREAM_TOKEN_ERROR_INVALID;
    }

    for (uint32_t i = parent->id; i < parser->num_tokens; i++)
    {
        jsmn_streamtok_t *token = parser->tokens + i;

        if (token->type == JSMN_STREAM_KEY)
        {
            size_t string_length = (size_t)(token->end - token->start);
            char buffer[string_length];

            if (parser->cb(token->start, string_length, parser->user_arg, buffer) == JSMN_STREAM_TOKEN_GET_CHAR_CB_ERROR_NONE)
            {
                if (string_compare(buffer, key, string_length) == true)
                {
                    *key_token = token;
                    return JSMN_STREAM_TOKEN_ERROR_NONE;
                }
            }
        }
    }

    return JSMN_STREAM_TOKEN_UTILS_ERROR_KEY_NOT_FOUND;
}

int32_t jsmn_stream_token_utils_get_value_token_by_key(jsmn_stream_token_parser_t *parser, jsmn_streamtok_t *parent, const char *key, jsmn_streamtok_t **value_token)
{
    if ((parser == NULL) 
        || (parent == NULL) 
        || (key == NULL)
        || (value_token == NULL))
    {
        return JSMN_STREAM_TOKEN_ERROR_INVALID;
    }

    jsmn_streamtok_t *key_token;
    if (jsmn_stream_token_utils_get_token_by_key(parser, parent, key, &key_token) == JSMN_STREAM_TOKEN_ERROR_NONE)
    {
        jsmn_streamtok_t *token = key_token + 1;
        if (token->parent_id == key_token->id)
        {
            *value_token = token;
            return JSMN_STREAM_TOKEN_ERROR_NONE;
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

int32_t jsmn_stream_token_utils_get_string_from_token(jsmn_stream_token_parser_t *parser, jsmn_streamtok_t *token, char *buffer)
{
    if ((parser == NULL) 
        || (token == NULL) 
        || (buffer == NULL))
    {
        return JSMN_STREAM_TOKEN_ERROR_INVALID;
    }

    size_t string_length = (size_t)(token->end - token->start);
    return parser->cb(token->start, string_length, parser->user_arg, buffer);
}

int32_t jsmn_stream_token_utils_get_string_by_key(jsmn_stream_token_parser_t *parser, jsmn_streamtok_t *parent, const char *key, char *buffer)
{
    if ((parser == NULL) 
        || (parent == NULL) 
        || (key == NULL)
        || (buffer == NULL))
    {
        return JSMN_STREAM_TOKEN_ERROR_INVALID;
    }

    jsmn_streamtok_t *token;
    if (jsmn_stream_token_utils_get_value_token_by_key(parser, parent, key, &token) == JSMN_STREAM_TOKEN_ERROR_NONE)
    {
        return jsmn_stream_token_utils_get_string_from_token(parser, token, buffer);
    }

    return JSMN_STREAM_TOKEN_UTILS_ERROR_KEY_NOT_FOUND;
}

int32_t jsmn_stream_token_utils_get_int_from_token(jsmn_stream_token_parser_t *parser, jsmn_streamtok_t *token,  int32_t *value)
{
     if ((parser == NULL) 
        || (token == NULL))
    {
        return JSMN_STREAM_TOKEN_ERROR_INVALID;
    }

    size_t string_length = (size_t)(token->end - token->start);
    char buffer[string_length];
    if (parser->cb(token->start, string_length, parser->user_arg, buffer) == JSMN_STREAM_TOKEN_GET_CHAR_CB_ERROR_NONE)
    {
        *value = strtol(buffer, NULL, 10);
        return JSMN_STREAM_TOKEN_ERROR_NONE;
    }

    return JSMN_STREAM_TOKEN_UTILS_ERROR_FAIL;
}

int32_t jsmn_stream_token_utils_get_int_by_key(jsmn_stream_token_parser_t *parser, jsmn_streamtok_t *parent, const char *key, int32_t *value)
{
    if ((parser == NULL) 
        || (parent == NULL) 
        || (key == NULL)
        || (value == NULL))
    {
        return JSMN_STREAM_TOKEN_ERROR_INVALID;
    }

    jsmn_streamtok_t *token;
    if (jsmn_stream_token_utils_get_value_token_by_key(parser, parent, key, &token) == JSMN_STREAM_TOKEN_ERROR_NONE)
    {
        return jsmn_stream_token_utils_get_int_from_token(parser, token, value);
    }

    return JSMN_STREAM_TOKEN_UTILS_ERROR_KEY_NOT_FOUND;
}

int32_t jsmn_stream_token_utils_get_double_from_token(jsmn_stream_token_parser_t *parser, jsmn_streamtok_t *token,  double *value)
{
        if ((parser == NULL) 
            || (token == NULL))
        {
            return JSMN_STREAM_TOKEN_ERROR_INVALID;
        }
    
        size_t string_length = (size_t)(token->end - token->start);
        char buffer[string_length];
        if (parser->cb(token->start, string_length, parser->user_arg, buffer) == JSMN_STREAM_TOKEN_GET_CHAR_CB_ERROR_NONE)
        {
            *value = strtod(buffer, NULL);
            return JSMN_STREAM_TOKEN_ERROR_NONE;
        }
    
        return JSMN_STREAM_TOKEN_UTILS_ERROR_FAIL;
}

int32_t jsmn_stream_token_utils_get_double_by_key(jsmn_stream_token_parser_t *parser, jsmn_streamtok_t *parent, const char *key, double *value)
{
    if ((parser == NULL) 
        || (parent == NULL) 
        || (key == NULL)
        || (value == NULL))
    {
        return JSMN_STREAM_TOKEN_ERROR_INVALID;
    }

    jsmn_streamtok_t *token;
    if (jsmn_stream_token_utils_get_value_token_by_key(parser, parent, key, &token) == JSMN_STREAM_TOKEN_ERROR_NONE)
    {
        return jsmn_stream_token_utils_get_double_from_token(parser, token, value);
    }

    return JSMN_STREAM_TOKEN_UTILS_ERROR_KEY_NOT_FOUND;
}

int32_t jsmn_stream_token_utils_get_bool_from_token(jsmn_stream_token_parser_t *parser, jsmn_streamtok_t *token,  bool *value)
{
    if ((parser == NULL) 
        || (token == NULL))
    {
        return JSMN_STREAM_TOKEN_ERROR_INVALID;
    }

    size_t string_length = (size_t)(token->end - token->start);
    char buffer[string_length];
    if (parser->cb(token->start, string_length, parser->user_arg, buffer) == JSMN_STREAM_TOKEN_GET_CHAR_CB_ERROR_NONE)
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

int32_t jsmn_stream_token_utils_get_bool_by_key(jsmn_stream_token_parser_t *parser, jsmn_streamtok_t *parent, const char *key, bool *value)
{
    if ((parser == NULL) 
        || (parent == NULL) 
        || (key == NULL)
        || (value == NULL))
    {
        return JSMN_STREAM_TOKEN_ERROR_INVALID;
    }

    jsmn_streamtok_t *token;
    if (jsmn_stream_token_utils_get_value_token_by_key(parser, parent, key, &token) == JSMN_STREAM_TOKEN_ERROR_NONE)
    {
        return jsmn_stream_token_utils_get_bool_from_token(parser, token, value);
    }

    return JSMN_STREAM_TOKEN_UTILS_ERROR_KEY_NOT_FOUND;
}

int32_t jsmn_stream_token_utils_get_object_containing_key_int(jsmn_stream_token_parser_t *token_parser, jsmn_streamtok_t *parent, const char *key, int32_t value, jsmn_streamtok_t **object_token)
{
    jsmn_streamtok_t *iterator_token = parent;

    // Find the key token
    while (jsmn_stream_token_utils_get_token_by_key(token_parser, iterator_token, key, &iterator_token) != JSMN_STREAM_TOKEN_UTILS_ERROR_KEY_NOT_FOUND)
    {
        // Get the value token
        jsmn_streamtok_t *value_token = iterator_token + 1;
        if (value_token->parent_id == iterator_token->id)
        {
            int32_t temp_value;
            if (jsmn_stream_token_utils_get_int_from_token(token_parser, value_token, &temp_value) == JSMN_STREAM_TOKEN_UTILS_ERROR_NONE)
            {
                if (temp_value == value)
                {
                    *object_token = iterator_token;
                    return JSMN_STREAM_TOKEN_UTILS_ERROR_NONE;
                }
            }
        }
        
        // Move to the next token
        iterator_token++;
    }

    return JSMN_STREAM_TOKEN_UTILS_ERROR_OBJECT_NOT_FOUND;
}