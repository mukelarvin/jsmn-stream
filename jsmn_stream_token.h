#ifndef __JSMN_STREAM_TOKEN_H_
#define __JSMN_STREAM_TOKEN_H_

#include <stdbool.h>
#include "jsmn_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

#define JSMN_STREAM_TOKEN_UNDEFINED -1
#define JSMN_STREAM_POSITION_UNDEFINED -1

enum jsmn_stream_token_error {
  JSMN_STREAM_TOKEN_ERROR_NONE = 0,
  JSMN_STREAM_TOKEN_ERROR_NOMEM = -1,
  JSMN_STREAM_TOKEN_ERROR_INVALID = -2,
};

/**
 * @brief
 * 
 */
typedef struct jsmn_streamtok {
  int id; // token id. Useful when using pointers instead of array indexes
  jsmn_streamtype_t type;
  int start; // start position in the JSON data string
  int end; // end position in the JSON data string
  int size; // number of child (nested) tokens
  int parent_id; // parent token id in the JSON data string
} jsmn_streamtok_t;

typedef struct {
  jsmn_stream_parser stream_parser;
  jsmn_streamtok_t *tokens;
  int next_token;
  int num_tokens;
  int char_count;
  int super_token_id;
  int error;
} jsmn_stream_token_parser_t;

void jsmn_stream_parse_tokens_init(jsmn_stream_token_parser_t *parser, jsmn_streamtok_t *tokens, int num_tokens);
int jsmn_stream_parse_tokens(jsmn_stream_token_parser_t *parser, char c);

#ifdef __cplusplus
}
#endif

#endif /* __JSMN_STREAM_TOKEN_H_ */
