#ifndef JSMN_STREAM_UTILS_H_
#define JSMN_STREAM_UTILS_H_

#include <stdint.h>
#include <stdbool.h>
#include "jsmn_stream.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define JSMN_STREAM_UTILS_PARSE_BUFFER_SIZE 512
#define JSMN_STREAM_POSITION_UNDEFINED (-1)

enum get_char_callback_error
{
    JSMN_STREAM_GET_CHAR_CB_ERROR_NONE = 0,
    JSMN_STREAM_GET_CHAR_CB_ERROR_FAIL = -1,
};

enum jsmn_stream_utils_error
{
    JSMN_STREAM_UTILS_ERROR_NONE = 0,
    JSMN_STREAM_UTILS_ERROR_FAIL = -1,
    JSMN_STREAM_UTILS_ERROR_INVALID_PARAM = -2,
    JSMN_STREAM_UTILS_ERROR_KEY_NOT_FOUND = -3,
    JSMN_STREAM_UTILS_ERROR_OBJECT_NOT_FOUND = -4,
};

enum jsmm_stream_token_parser_state
{
    JSMN_STREAM_TOKEN_PARSER_STATE_ERROR = -1,
    JSMN_STREAM_TOKEN_PARSER_STATE_INCOMPLETE = 0,
    JSMN_STREAM_TOKEN_PARSER_STATE_COMPLETE = 1
};

typedef struct jsmn_stream_token
{
  jsmn_streamtype_t type;
  int32_t start_position; // start position in the JSON data string
  int32_t end_position; // end position in the JSON data string
  int32_t parent_position; // parent position in the JSON data string
  int32_t depth; // object depth
} jsmn_stream_token_t;

typedef int32_t (*get_char_callback_t)(uint32_t index, size_t length, void *user_arg, char *ch);

typedef struct new_jsmn_stream_token_parser
{
  get_char_callback_t get_char_callback;
  size_t json_length;
  void *user_arg;
  int32_t state;
  uint32_t index;
} new_jsmn_stream_token_parser_t;

int32_t jsmn_stream_utils_init_parser(new_jsmn_stream_token_parser_t *parser, get_char_callback_t get_char_callback, size_t json_length, void *user_arg);
int32_t jsmn_stream_utils_init_token(jsmn_stream_token_t *token);
int32_t jsmn_stream_utils_get_value_token_by_key(new_jsmn_stream_token_parser_t *token_parser, int32_t start_index, const char *key, jsmn_stream_token_t *value_token);
int32_t jsmn_stream_utils_array_get_size(new_jsmn_stream_token_parser_t *token_parser, jsmn_stream_token_t *token, uint32_t *size);
int32_t jsmn_stream_utils_array_get_next_object_token(new_jsmn_stream_token_parser_t *token_parser, jsmn_stream_token_t *parent_token, jsmn_stream_token_t *iterator_token);
int32_t jsmn_stream_utils_object_get_size(new_jsmn_stream_token_parser_t *token_parser, jsmn_stream_token_t *token, uint32_t *size);
int32_t jsmn_stream_utils_object_get_next_kv_tokens(new_jsmn_stream_token_parser_t *token_parser, jsmn_stream_token_t *parent_token, jsmn_stream_token_t *key_iterator_token, jsmn_stream_token_t *value_iterator_token);
int32_t jsmn_stream_utils_get_object_token_containing_kv(new_jsmn_stream_token_parser_t *token_parser, int32_t start_index, const char *key, const char *value, jsmn_stream_token_t *object_token);

// accessors
int32_t jsmn_stream_utils_get_bool_from_token(new_jsmn_stream_token_parser_t *token_parser, jsmn_stream_token_t *token, bool *value);
int32_t jsmn_stream_utils_get_bool_by_key(new_jsmn_stream_token_parser_t *token_parser, int32_t start_index, const char *key, bool *value);
int32_t jsmn_stream_utils_get_int_from_token(new_jsmn_stream_token_parser_t *token_parser, jsmn_stream_token_t *token, int32_t *value);
int32_t jsmn_stream_utils_get_int_by_key(new_jsmn_stream_token_parser_t *token_parser, int32_t start_index, const char *key, int32_t *value);
int32_t jsmn_stream_utils_get_uint_from_token(new_jsmn_stream_token_parser_t *token_parser, jsmn_stream_token_t *token, uint32_t *value);
int32_t jsmn_stream_utils_get_uint_by_key(new_jsmn_stream_token_parser_t *token_parser, int32_t start_index, const char *key, uint32_t *value);
int32_t jsmn_stream_utils_get_double_from_token(new_jsmn_stream_token_parser_t *token_parser, jsmn_stream_token_t *token, double *value);
int32_t jsmn_stream_utils_get_double_by_key(new_jsmn_stream_token_parser_t *token_parser, int32_t start_index, const char *key, double *value);
int32_t jsmn_stream_utils_get_string_from_token(new_jsmn_stream_token_parser_t *token_parser, jsmn_stream_token_t *token, char *buffer, size_t buffer_size);
int32_t jsmn_stream_utils_get_string_by_key(new_jsmn_stream_token_parser_t *token_parser, int32_t start_index, const char *key, char *buffer, size_t buffer_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* JSMN_STREAM_UTILS_H_ */
