#include "jsmn_stream_utils.h"
#include <stdbool.h>
#include <stdlib.h>

// forward declarations

static int32_t parse_json(new_jsmn_stream_token_parser_t *token_parser, jsmn_stream_parser *stream_parser);

// get value by key helper structs and callbacks
typedef struct get_value_token_by_key_user_arg
{
    new_jsmn_stream_token_parser_t *token_parser;
    jsmn_stream_parser *stream_parser;
    const char *key;
    jsmn_stream_token_t *value_token;
    uint32_t processing_start_index;
    bool found_key;
} get_value_token_by_key_user_arg_t;

static void get_value_token_by_key_start_object_callback(void *user_arg);
static void get_value_token_by_key_end_object_callback(void *user_arg);
static void get_value_token_by_key_object_key_callback(const char *key, size_t key_length, void *user_arg);
static void get_value_token_by_key_string_callback(const char *value, size_t length, void *user_arg);
static void get_value_token_by_key_primitive_callback(const char *value, size_t length, void *user_arg);
static void get_value_token_by_key_start_array_callback(void *user_arg);
static void get_value_token_by_key_end_array_callback(void *user_arg);

// get next object helper structs and callbacks
typedef struct get_next_object_user_arg
{
    new_jsmn_stream_token_parser_t *token_parser;
    jsmn_stream_parser *stream_parser;
    jsmn_stream_token_t *parent_token;
    jsmn_stream_token_t *iterator_token;
} get_next_object_user_arg_t;

static void get_next_object_start_object_callback(void *user_arg);
static void get_next_object_end_object_callback(void *user_arg);
static void get_next_object_object_key_callback(const char *key, size_t key_length, void *user_arg);
static void get_next_object_string_callback(const char *value, size_t length, void *user_arg);
static void get_next_object_primitive_callback(const char *value, size_t length, void *user_arg);
static void get_next_object_start_array_callback(void *user_arg);
static void get_next_object_end_array_callback(void *user_arg);

// get object containing key/value helper structs and callbacks
typedef struct get_object_token_containing_kv_arg
{
    new_jsmn_stream_token_parser_t *token_parser;
    jsmn_stream_parser *stream_parser;
    char *key;
    char *value;
    jsmn_stream_token_t *object_token;
    bool found_key;
    bool found_value;
} get_object_token_containing_kv_arg_t;

static void get_object_token_containing_kv_start_object_callback(void *user_arg);
static void get_object_token_containing_kv_start_array_callback(void *user_arg);
static void get_object_token_containing_kv_object_key_callback(const char *key, size_t key_length, void *user_arg);
static void get_object_token_containing_kv_string_callback(const char *value, size_t length, void *user_arg);
static void get_object_token_containing_kv_primitive_callback(const char *value, size_t length, void *user_arg);



int32_t jsmn_stream_utils_init_parser(new_jsmn_stream_token_parser_t *parser, get_char_callback_t get_char_callback, size_t json_length, void *user_arg)
{
    if (parser == NULL || get_char_callback == NULL || json_length == 0)
    {
        return JSMN_STREAM_UTILS_ERROR_INVALID_PARAM;
    }

    parser->get_char_callback = get_char_callback;
    parser->json_length = json_length;
    parser->user_arg = user_arg;
    parser->state = JSMN_STREAM_TOKEN_PARSER_STATE_INCOMPLETE;
    parser->index = 0;

    return JSMN_STREAM_UTILS_ERROR_NONE;
}

int32_t jsmn_stream_utils_init_token(jsmn_stream_token_t *token)
{
    if (token == NULL)
    {
        return JSMN_STREAM_UTILS_ERROR_INVALID_PARAM;
    }

    token->type = JSMN_STREAM_UNDEFINED;
    token->start_position = 0;
    token->end_position = 0;
    token->parent_position = JSMN_STREAM_POSITION_UNDEFINED;

    return JSMN_STREAM_UTILS_ERROR_NONE;

}

static int32_t parse_json(new_jsmn_stream_token_parser_t *token_parser, jsmn_stream_parser *stream_parser)
{
    // right now we have to start parsing from the beginning of the json
    // because jsmn was having an issue with key/value ordering if you don't start
    // with an object tag
    for (uint32_t i = 0; i < (uint32_t)token_parser->json_length; i++)
    {
        char ch;
        if (token_parser->get_char_callback(i, 1, token_parser->user_arg, &ch) == JSMN_STREAM_TOKEN_GET_CHAR_CB_ERROR_NONE)
        {
            token_parser->index = i;
            int32_t parse_result = jsmn_stream_parse(stream_parser, ch);

            // note: JSMN_STREAM_ERROR_PART is okay because we have more chars coming.
            if ( (parse_result == JSMN_STREAM_ERROR_NOMEM)
                || (parse_result == JSMN_STREAM_ERROR_INVAL)
                || (parse_result == JSMN_STREAM_ERROR_MAX_DEPTH)
                || (token_parser->state == JSMN_STREAM_TOKEN_PARSER_STATE_ERROR ))
            {
                return JSMN_STREAM_UTILS_ERROR_FAIL;
            }
 
            if (token_parser->state == JSMN_STREAM_TOKEN_PARSER_STATE_COMPLETE)
            {
                return JSMN_STREAM_UTILS_ERROR_NONE;
            }
        }
    }

    return JSMN_STREAM_UTILS_ERROR_NONE;
}

int32_t jsmn_stream_utils_get_value_token_by_key(new_jsmn_stream_token_parser_t *token_parser, const char *key, jsmn_stream_token_t *value_token)
{
    // check params
    if (token_parser == NULL || key == NULL || value_token == NULL)
    {
        return JSMN_STREAM_UTILS_ERROR_INVALID_PARAM;
    }

    // init stream parser
    jsmn_stream_parser stream_parser;
    jsmn_stream_callbacks_t jsmn_stream_token_callbacks =
    {
        .start_array_callback = get_value_token_by_key_start_array_callback,
        .end_array_callback = get_value_token_by_key_end_array_callback,
        .start_object_callback = get_value_token_by_key_start_object_callback,
        .end_object_callback = get_value_token_by_key_end_object_callback,
        .object_key_callback = get_value_token_by_key_object_key_callback,
        .string_callback = get_value_token_by_key_string_callback,
        .primitive_callback = get_value_token_by_key_primitive_callback
    };

    // init user arg
    get_value_token_by_key_user_arg_t user_arg = 
    {
        .token_parser = token_parser,
        .stream_parser = &stream_parser,
        .key = key,
        .value_token = value_token,
        .found_key = false
    };

    jsmn_stream_init(&stream_parser, &jsmn_stream_token_callbacks, &user_arg);
    token_parser->index = value_token->start_position;
    token_parser->state = JSMN_STREAM_TOKEN_PARSER_STATE_INCOMPLETE;

    if (parse_json(token_parser, &stream_parser) != JSMN_STREAM_UTILS_ERROR_NONE)
    {
        return JSMN_STREAM_UTILS_ERROR_FAIL;
    }

    if (user_arg.found_key == true)
    {
        return JSMN_STREAM_UTILS_ERROR_NONE;
    }
    else
    {
        return JSMN_STREAM_UTILS_ERROR_KEY_NOT_FOUND;
    }

}

static void get_value_token_by_key_start_object_callback(void *user_arg)
{
    get_value_token_by_key_user_arg_t *arg = (get_value_token_by_key_user_arg_t *)user_arg;

    // update token
    arg->value_token->parent_position = arg->value_token->start_position;
    arg->value_token->start_position = arg->token_parser->index;
    arg->value_token->end_position = JSMN_STREAM_POSITION_UNDEFINED; // @todo
    arg->value_token->type = JSMN_STREAM_OBJECT;
    arg->value_token->depth = arg->stream_parser->stack_height;

    if (arg->found_key == true)
    {
        arg->token_parser->state = JSMN_STREAM_TOKEN_PARSER_STATE_COMPLETE;
    }
}

static void get_value_token_by_key_end_object_callback(void *user_arg)
{
    // @todo set the end of the object and parent as needed
}

static void get_value_token_by_key_object_key_callback(const char *key, size_t key_length, void *user_arg)
{
    get_value_token_by_key_user_arg_t *arg = (get_value_token_by_key_user_arg_t *)user_arg;

    // update token
    arg->value_token->parent_position = arg->value_token->start_position;
    arg->value_token->start_position = arg->token_parser->index - key_length;
    arg->value_token->end_position = arg->token_parser->index;
    arg->value_token->type = JSMN_STREAM_KEY;
    arg->value_token->depth = arg->stream_parser->stack_height;

    // check if key matches
    if (strncmp(arg->key, key, key_length) == 0)
    {
        arg->found_key = true;
    }
}

static void get_value_token_by_key_string_callback(const char *value, size_t length, void *user_arg)
{
    get_value_token_by_key_user_arg_t *arg = (get_value_token_by_key_user_arg_t *)user_arg;

    // update token
    arg->value_token->parent_position = arg->value_token->start_position;
    arg->value_token->start_position = arg->token_parser->index - length;
    arg->value_token->end_position = arg->token_parser->index;
    arg->value_token->type = JSMN_STREAM_STRING;
    arg->value_token->depth = arg->stream_parser->stack_height;
    
    // check if key was found
    if (arg->found_key == true)
    {
        // update state
        arg->token_parser->state = JSMN_STREAM_TOKEN_PARSER_STATE_COMPLETE;
    }
}

static void get_value_token_by_key_primitive_callback(const char *value, size_t length, void *user_arg)
{
    get_value_token_by_key_user_arg_t *arg = (get_value_token_by_key_user_arg_t *)user_arg;

    // update token
    arg->value_token->parent_position = arg->value_token->start_position;
    arg->value_token->start_position = arg->token_parser->index - length;
    arg->value_token->end_position = arg->token_parser->index;
    arg->value_token->type = JSMN_STREAM_PRIMITIVE;
    arg->value_token->depth = arg->stream_parser->stack_height;
    
    // check if key was found
    if (arg->found_key == true)
    {
        // update state
        arg->token_parser->state = JSMN_STREAM_TOKEN_PARSER_STATE_COMPLETE;
    }
}

static void get_value_token_by_key_start_array_callback(void *user_arg)
{
    get_value_token_by_key_user_arg_t *arg = (get_value_token_by_key_user_arg_t *)user_arg;

    // update token
    arg->value_token->parent_position = arg->value_token->start_position;
    arg->value_token->start_position = arg->token_parser->index;
    arg->value_token->end_position = JSMN_STREAM_POSITION_UNDEFINED; // @todo
    arg->value_token->type = JSMN_STREAM_ARRAY;
    arg->value_token->depth = arg->stream_parser->stack_height;

    // check if key was found
    if (arg->found_key == true)
    {
        arg->token_parser->state = JSMN_STREAM_TOKEN_PARSER_STATE_COMPLETE;
        // @todo find the end of the array
    }

}

static void get_value_token_by_key_end_array_callback(void *user_arg)
{
   // @todo set the end of the array and parent as needed
}

int32_t jsmn_stream_utils_get_next_object_token(new_jsmn_stream_token_parser_t *token_parser, jsmn_stream_token_t *parent_token, jsmn_stream_token_t *iterator_token)
{
    // check params
    if (token_parser == NULL || parent_token == NULL || iterator_token == NULL)
    {
        return JSMN_STREAM_UTILS_ERROR_INVALID_PARAM;
    }

    // init stream parser
    jsmn_stream_parser stream_parser = {0};
    jsmn_stream_callbacks_t jsmn_stream_token_callbacks =
    {
        .start_array_callback = NULL,
        .end_array_callback = NULL,
        .start_object_callback = get_next_object_start_object_callback,
        .end_object_callback = NULL,
        .object_key_callback = NULL,
        .string_callback = NULL,
        .primitive_callback = NULL
    };

    // init user arg
    get_next_object_user_arg_t user_arg = 
    {
        .token_parser = token_parser,
        .stream_parser = &stream_parser,
        .parent_token = parent_token,
        .iterator_token = iterator_token
    };

    jsmn_stream_init(&stream_parser, &jsmn_stream_token_callbacks, &user_arg);
    token_parser->state = JSMN_STREAM_TOKEN_PARSER_STATE_INCOMPLETE;

    int32_t result = parse_json(token_parser, &stream_parser);

    if (token_parser->state == JSMN_STREAM_TOKEN_PARSER_STATE_INCOMPLETE)
    {
        return JSMN_STREAM_UTILS_ERROR_OBJECT_NOT_FOUND;
    }
    else
    {
        return result;
    }
}

static void get_next_object_start_object_callback(void *user_arg)
{
    get_next_object_user_arg_t *arg = (get_next_object_user_arg_t *)user_arg;

    // get to the right depth
    if (arg->stream_parser->stack_height == arg->parent_token->depth + 1)
    {
        // get to the right object
        if (arg->token_parser->index > arg->iterator_token->start_position)
        {        
            // update token
            arg->iterator_token->parent_position = arg->parent_token->start_position;
            arg->iterator_token->start_position = arg->token_parser->index;
            arg->iterator_token->end_position = JSMN_STREAM_POSITION_UNDEFINED;
            arg->iterator_token->type = JSMN_STREAM_OBJECT;
            arg->iterator_token->depth = arg->stream_parser->stack_height;

            arg->token_parser->state = JSMN_STREAM_TOKEN_PARSER_STATE_COMPLETE;
        }
    }
} 

int32_t jsmn_stream_utils_get_object_token_containing_kv(new_jsmn_stream_token_parser_t *token_parser, const char *key, const char *value, jsmn_stream_token_t *object_token)
{
    // check params
    if (token_parser == NULL || key == NULL || value == NULL || object_token == NULL)
    {
        return JSMN_STREAM_UTILS_ERROR_INVALID_PARAM;
    }

    // init stream parser
    jsmn_stream_parser stream_parser = {0};
    jsmn_stream_callbacks_t jsmn_stream_token_callbacks =
    {
        .start_array_callback = get_object_token_containing_kv_start_array_callback,
        .end_array_callback = NULL,
        .start_object_callback = get_object_token_containing_kv_start_object_callback,
        .end_object_callback = NULL,
        .object_key_callback = get_object_token_containing_kv_object_key_callback,
        .string_callback = get_object_token_containing_kv_string_callback,
        .primitive_callback = get_object_token_containing_kv_primitive_callback
    };

    // init user arg
    get_object_token_containing_kv_arg_t user_arg = 
    {
        .token_parser = token_parser,
        .stream_parser = &stream_parser,
        .key = (char *)key,
        .value = (char *)value,
        .object_token = object_token
    };

    jsmn_stream_init(&stream_parser, &jsmn_stream_token_callbacks, &user_arg);
    token_parser->state = JSMN_STREAM_TOKEN_PARSER_STATE_INCOMPLETE;

    int32_t result = parse_json(token_parser, &stream_parser);

    if (token_parser->state == JSMN_STREAM_TOKEN_PARSER_STATE_INCOMPLETE)
    {
        return JSMN_STREAM_UTILS_ERROR_OBJECT_NOT_FOUND;
    }
    else
    {
        return result;
    }
}

static void get_object_token_containing_kv_start_object_callback(void *user_arg)
{
    get_object_token_containing_kv_arg_t *arg = (get_object_token_containing_kv_arg_t *)user_arg;

    // update token
    arg->object_token->parent_position = arg->object_token->start_position;
    arg->object_token->start_position = arg->token_parser->index;
    arg->object_token->end_position = JSMN_STREAM_POSITION_UNDEFINED; // @todo
    arg->object_token->type = JSMN_STREAM_OBJECT;
    arg->object_token->depth = arg->stream_parser->stack_height;
}

static void get_object_token_containing_kv_start_array_callback(void *user_arg)
{
    get_object_token_containing_kv_arg_t *arg = (get_object_token_containing_kv_arg_t *)user_arg;

    // update token
    arg->object_token->parent_position = arg->object_token->start_position;
    arg->object_token->start_position = arg->token_parser->index;
    arg->object_token->end_position = JSMN_STREAM_POSITION_UNDEFINED; // @todo
    arg->object_token->type = JSMN_STREAM_ARRAY;
    arg->object_token->depth = arg->stream_parser->stack_height;
}

static void get_object_token_containing_kv_object_key_callback(const char *key, size_t key_length, void *user_arg)
{
    get_object_token_containing_kv_arg_t *arg = (get_object_token_containing_kv_arg_t *)user_arg;

    // do not update token, we want the parent object

    // check if key matches
    if (strncmp(arg->key, key, key_length) == 0)
    {
        arg->found_key = true;
    }
}

static void get_object_token_containing_kv_string_callback(const char *value, size_t length, void *user_arg)
{
    get_object_token_containing_kv_arg_t *arg = (get_object_token_containing_kv_arg_t *)user_arg;

    // do not update token, we want the parent object
    
    // check if key was found
    if (arg->found_key == true)
    {
        // check if value matches
        if (strncmp(arg->value, value, length) == 0)
        {
            arg->found_value = true;
            arg->token_parser->state = JSMN_STREAM_TOKEN_PARSER_STATE_COMPLETE;
        }
    }
}

static void get_object_token_containing_kv_primitive_callback(const char *value, size_t length, void *user_arg)
{
    get_object_token_containing_kv_arg_t *arg = (get_object_token_containing_kv_arg_t *)user_arg;

    // do not update token, we want the parent object
    
    // check if key was found
    if (arg->found_key == true)
    {
        // check if value matches
        if (strncmp(arg->value, value, length) == 0)
        {
            arg->found_value = true;
            arg->token_parser->state = JSMN_STREAM_TOKEN_PARSER_STATE_COMPLETE;
        }
    }
}

// accessors
int32_t jsmn_stream_utils_get_bool_from_token(new_jsmn_stream_token_parser_t *token_parser, jsmn_stream_token_t *token, bool *value)
{
    if (token_parser == NULL || token == NULL || value == NULL)
    {
        return JSMN_STREAM_UTILS_ERROR_INVALID_PARAM;
    }

    char value_str[6] = {0};
    if (token_parser->get_char_callback(token->start_position, token->end_position - token->start_position, token_parser->user_arg, value_str) == JSMN_STREAM_TOKEN_GET_CHAR_CB_ERROR_NONE)
    {
        if (strncmp(value_str, "true", 4) == 0)
        {
            *value = true;
            return JSMN_STREAM_UTILS_ERROR_NONE;
        }
        else if (strncmp(value_str, "false", 5) == 0)
        {
            *value = false;
            return JSMN_STREAM_UTILS_ERROR_NONE;
        }

    }

    return JSMN_STREAM_UTILS_ERROR_FAIL;

}

int32_t jsmn_stream_utils_get_bool_by_key(new_jsmn_stream_token_parser_t *token_parser, const char *key, bool *value)
{
    if (token_parser == NULL || key == NULL || value == NULL)
    {
        return JSMN_STREAM_UTILS_ERROR_INVALID_PARAM;
    }

    jsmn_stream_token_t value_token;
    jsmn_stream_utils_init_token(&value_token);

    int32_t result = jsmn_stream_utils_get_value_token_by_key(token_parser, key, &value_token);

    if (result == JSMN_STREAM_UTILS_ERROR_NONE)
    {
        return jsmn_stream_utils_get_bool_from_token(token_parser, &value_token, value);
    }
    else
    {
        return result;
    }
}

int32_t jsmn_stream_utils_get_int_from_token(new_jsmn_stream_token_parser_t *token_parser, jsmn_stream_token_t *token, int32_t *value)
{
    if (token_parser == NULL || token == NULL || value == NULL)
    {
        return JSMN_STREAM_UTILS_ERROR_INVALID_PARAM;
    }

    char value_str[11] = {0};
    if (token_parser->get_char_callback(token->start_position, token->end_position - token->start_position, token_parser->user_arg, value_str) == JSMN_STREAM_TOKEN_GET_CHAR_CB_ERROR_NONE)
    {
        *value = strtol(value_str, NULL, 10);
        return JSMN_STREAM_UTILS_ERROR_NONE;
    }

    return JSMN_STREAM_UTILS_ERROR_FAIL;
}

int32_t jsmn_stream_utils_get_int_by_key(new_jsmn_stream_token_parser_t *token_parser, const char *key, int32_t *value)
{
    if (token_parser == NULL || key == NULL || value == NULL)
    {
        return JSMN_STREAM_UTILS_ERROR_INVALID_PARAM;
    }

    jsmn_stream_token_t value_token;
    jsmn_stream_utils_init_token(&value_token);

    int32_t result = jsmn_stream_utils_get_value_token_by_key(token_parser, key, &value_token);

    if (result == JSMN_STREAM_UTILS_ERROR_NONE)
    {
        return jsmn_stream_utils_get_int_from_token(token_parser, &value_token, value);
    }
    else
    {
        return result;
    }
}
 
int32_t jsmn_stream_utils_get_uint_from_token(new_jsmn_stream_token_parser_t *token_parser, jsmn_stream_token_t *token, uint32_t *value)
{
    if (token_parser == NULL || token == NULL || value == NULL)
    {
        return JSMN_STREAM_UTILS_ERROR_INVALID_PARAM;
    }

    char value_str[11] = {0};
    if (token_parser->get_char_callback(token->start_position, token->end_position - token->start_position, token_parser->user_arg, value_str) == JSMN_STREAM_TOKEN_GET_CHAR_CB_ERROR_NONE)
    {
        *value = strtoul(value_str, NULL, 10);
        return JSMN_STREAM_UTILS_ERROR_NONE;
    }

    return JSMN_STREAM_UTILS_ERROR_FAIL;
}

int32_t jsmn_stream_utils_get_uint_by_key(new_jsmn_stream_token_parser_t *token_parser, const char *key, uint32_t *value)
{
    if (token_parser == NULL || key == NULL || value == NULL)
    {
        return JSMN_STREAM_UTILS_ERROR_INVALID_PARAM;
    }

    jsmn_stream_token_t value_token;
    jsmn_stream_utils_init_token(&value_token);

    int32_t result = jsmn_stream_utils_get_value_token_by_key(token_parser, key, &value_token);

    if (result == JSMN_STREAM_UTILS_ERROR_NONE)
    {
        return jsmn_stream_utils_get_uint_from_token(token_parser, &value_token, value);
    }
    else
    {
        return result;
    }
}

int32_t jsmn_stream_utils_get_double_from_token(new_jsmn_stream_token_parser_t *token_parser, jsmn_stream_token_t *token, double *value)
{
    if (token_parser == NULL || token == NULL || value == NULL)
    {
        return JSMN_STREAM_UTILS_ERROR_INVALID_PARAM;
    }

    char value_str[32] = {0};
    if (token_parser->get_char_callback(token->start_position, token->end_position - token->start_position, token_parser->user_arg, value_str) == JSMN_STREAM_TOKEN_GET_CHAR_CB_ERROR_NONE)
    {
        *value = strtod(value_str, NULL);
        return JSMN_STREAM_UTILS_ERROR_NONE;
    }

    return JSMN_STREAM_UTILS_ERROR_FAIL;
}

int32_t jsmn_stream_utils_get_double_by_key(new_jsmn_stream_token_parser_t *token_parser, const char *key, double *value)
{
    if (token_parser == NULL || key == NULL || value == NULL)
    {
        return JSMN_STREAM_UTILS_ERROR_INVALID_PARAM;
    }

    jsmn_stream_token_t value_token;
    jsmn_stream_utils_init_token(&value_token);

    int32_t result = jsmn_stream_utils_get_value_token_by_key(token_parser, key, &value_token);

    if (result == JSMN_STREAM_UTILS_ERROR_NONE)
    {
        return jsmn_stream_utils_get_double_from_token(token_parser, &value_token, value);
    }
    else
    {
        return result;
    }
}

int32_t jsmn_stream_utils_get_string_from_token(new_jsmn_stream_token_parser_t *token_parser, jsmn_stream_token_t *token, char *buffer, size_t buffer_size)
{
    if (token_parser == NULL || token == NULL || buffer == NULL || buffer_size == 0)
    {
        return JSMN_STREAM_UTILS_ERROR_INVALID_PARAM;
    }

    if (token->type != JSMN_STREAM_STRING)
    {
        return JSMN_STREAM_UTILS_ERROR_INVALID_PARAM;
    }

    if (token->end_position - token->start_position > buffer_size)
    {
        return JSMN_STREAM_UTILS_ERROR_INVALID_PARAM;
    }

    if (token_parser->get_char_callback(token->start_position, token->end_position - token->start_position, token_parser->user_arg, buffer) == JSMN_STREAM_TOKEN_GET_CHAR_CB_ERROR_NONE)
    {
        return JSMN_STREAM_UTILS_ERROR_NONE;
    }

    return JSMN_STREAM_UTILS_ERROR_FAIL;
}

int32_t jsmn_stream_utils_get_string_by_key(new_jsmn_stream_token_parser_t *token_parser, const char *key, char *buffer, size_t buffer_size)
{
    if (token_parser == NULL || key == NULL || buffer == NULL || buffer_size == 0)
    {
        return JSMN_STREAM_UTILS_ERROR_INVALID_PARAM;
    }

    jsmn_stream_token_t value_token;
    jsmn_stream_utils_init_token(&value_token);

    int32_t result = jsmn_stream_utils_get_value_token_by_key(token_parser, key, &value_token);

    if (result == JSMN_STREAM_UTILS_ERROR_NONE)
    {
        return jsmn_stream_utils_get_string_from_token(token_parser, &value_token, buffer, buffer_size);
    }
    else
    {
        return result;
    }
}