#ifndef __JSMN_STREAM_TOKEN_H_
#define __JSMN_STREAM_TOKEN_H_

#include <stdbool.h>
#include "jsmn_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

enum jsmn_stream_token_error {
  JSMN_STREAM_TOKEN_ERROR_NONE = 0,
  JSMN_STREAM_TOKEN_ERROR_NOMEM = -1,
  JSMN_STREAM_TOKEN_ERROR_INVAL = -2,
  JSMN_STREAM_TOKEN_ERROR_PART = -3,
  JSMN_STREAM_TOKEN_ERROR_MAX = 0x7FFFFFFF
};

typedef struct jsmn_streamtok {
  jsmn_streamtype_t type;
  int start;
  int end;
  int size;
  int parent;
} jsmn_streamtok_t;

typedef struct {
  jsmn_stream_parser stream_parser;
  jsmn_streamtok_t *tokens;
  int next_token;
  int num_tokens;
  int char_count;
  int toksuper;
  int error;
} jsmn_stream_token_parser_t;

void jsmn_stream_parse_tokens_init(jsmn_stream_token_parser_t *parser, jsmn_streamtok_t *tokens, int num_tokens);
int jsmn_stream_parse_tokens(jsmn_stream_token_parser_t *parser, char c);

#ifdef __cplusplus
}
#endif

#endif /* __JSMN_STREAM_TOKEN_H_ */
