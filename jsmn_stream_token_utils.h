#ifndef JSMN_STREAM_TOKEN_UTILS_H_
#define JSMN_STREAM_TOKEN_UTILS_H_

#include <stdint.h>
#include "jsmn_stream.h"
#include "jsmn_stream_token.h"

#ifdef __cplusplus
extern "C"
{
#endif

enum jsmn_stream_token_get_char_cb_error
{
    JSMN_STREAM_TOKEN_GET_CHAR_CB_ERROR_NONE = 0,
    JSMN_STREAM_TOKEN_GET_CHAR_CB_ERROR_FAIL = -1,
};

enum jsmn_stream_token_utils_error
{
    JSMN_STREAM_TOKEN_UTILS_ERROR_NONE = 0,
    JSMN_STREAM_TOKEN_UTILS_ERROR_FAIL = -1,
    JSMN_STREAM_TOKEN_UTILS_ERROR_INVALID = -2,
    JSMN_STREAM_TOKEN_UTILS_ERROR_KEY_NOT_FOUND = -3,
    JSMN_STREAM_TOKEN_UTILS_ERROR_OBJECT_NOT_FOUND = -4,
};

typedef int32_t (*jsmn_stream_token_get_char_cb_t)(uint32_t index, size_t length, void *user_arg, char *ch);

int32_t jsmn_stream_token_utils_parse_with_cb(jsmn_stream_token_parser_t *parser, jsmn_stream_token_get_char_cb_t get_char_cb, size_t length, void *user_arg);
int32_t jsmn_stream_token_utils_get_value_token_by_key(jsmn_stream_token_get_char_cb_t get_char_cb, void *user_arg, jsmn_streamtok_t *parent, uint32_t num_tokens, const char *key, jsmn_streamtok_t **value_token);
int32_t jsmn_stream_token_utils_array_get_next_object_token(jsmn_stream_token_parser_t *parser, jsmn_streamtok_t *parent, jsmn_streamtok_t **iterator_token);
int32_t jsmn_stream_token_utils_get_string_from_token(jsmn_stream_token_get_char_cb_t get_char_cb, void *user_arg, jsmn_streamtok_t *token, char *buffer);
int32_t jsmn_stream_token_utils_get_int_from_token(jsmn_stream_token_get_char_cb_t get_char_cb, void *user_arg, jsmn_streamtok_t *token,  int32_t *value);
int32_t jsmn_stream_token_utils_get_double_from_token(jsmn_stream_token_get_char_cb_t get_char_cb, void *user_arg, jsmn_streamtok_t *token,  double *value);
int32_t jsmn_stream_token_utils_get_bool_from_token(jsmn_stream_token_get_char_cb_t get_char_cb, void *user_arg, jsmn_streamtok_t *token,  bool *value);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* JSMN_STREAM_TOKEN_UTILS_H_ */
