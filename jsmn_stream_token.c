#include "jsmn_stream_token.h"
#include <stdbool.h>


static jsmn_streamtok_t *jsmn_stream_allocate_token(jsmn_stream_token_parser_t *jsmn_stream_parser)
{
	jsmn_streamtok_t *token;

	if (jsmn_stream_parser->next_token >= jsmn_stream_parser->num_tokens)
	{
		jsmn_stream_parser->error = JSMN_STREAM_ERROR_NOMEM;
		return NULL;
	}

	token = &jsmn_stream_parser->tokens[jsmn_stream_parser->next_token++];
	token->type = JSMN_STREAM_UNDEFINED;
	token->start = -1;
	token->end = -1;
	token->size = 0;
	token->parent = jsmn_stream_parser->toksuper;
	return token;
}

static jsmn_streamtok_t *jsmn_stream_get_super_token(jsmn_stream_token_parser_t *jsmn_stream_parser)
{
	return &jsmn_stream_parser->tokens[jsmn_stream_parser->toksuper];
}

static void jsmn_stream_parse_tokens_start_array(void *user_arg)
{
	jsmn_stream_token_parser_t *jsmn_stream_parser = (jsmn_stream_token_parser_t *)user_arg;
	jsmn_streamtok_t *token = jsmn_stream_allocate_token(jsmn_stream_parser);

	if (token == NULL)
	{
		return;
	}

	// update the parent size
	if (jsmn_stream_parser->toksuper != -1)
	{
		jsmn_streamtok_t *parent = jsmn_stream_get_super_token(jsmn_stream_parser);
		parent->size++;
		token->parent = jsmn_stream_parser->toksuper;
	}

	token->type = JSMN_STREAM_ARRAY;
	token->start = jsmn_stream_parser->char_count - 1;

	// make this the new super token
	jsmn_stream_parser->toksuper = jsmn_stream_parser->next_token - 1;
}


static void jsmn_stream_parse_tokens_end_array(void *user_arg)
{
	jsmn_stream_token_parser_t *jsmn_stream_parser = (jsmn_stream_token_parser_t *)user_arg;
	jsmn_streamtok_t *token = jsmn_stream_get_super_token(jsmn_stream_parser);

	if (token == NULL)
	{
		return;
	}

	// step back to the start of the array
	while (token->parent > -1 && token->type != JSMN_STREAM_ARRAY)
	{
		token = &jsmn_stream_parser->tokens[token->parent];
	}
	
	token->end = jsmn_stream_parser->char_count; 

	// make the parent the new super token
	jsmn_stream_parser->toksuper = token->parent;

}

static void jsmn_stream_parse_tokens_start_object(void *user_arg)
{
	jsmn_stream_token_parser_t *jsmn_stream_parser = (jsmn_stream_token_parser_t *)user_arg;
	jsmn_streamtok_t *token = jsmn_stream_allocate_token(jsmn_stream_parser);

	if (token == NULL)
	{
		return;
	}

	// update the parent size
	if (jsmn_stream_parser->toksuper != -1)
	{
		jsmn_streamtok_t *parent = jsmn_stream_get_super_token(jsmn_stream_parser);
		parent->size++;
		token->parent = jsmn_stream_parser->toksuper;
	}

	token->type = JSMN_STREAM_OBJECT;
	token->start = jsmn_stream_parser->char_count - 1; 

	// make this the new super token
	jsmn_stream_parser->toksuper = jsmn_stream_parser->next_token - 1;
}

static void jsmn_stream_parse_tokens_end_object(void *user_arg)
{
	jsmn_stream_token_parser_t *jsmn_stream_parser = (jsmn_stream_token_parser_t *)user_arg;
	jsmn_streamtok_t *token = jsmn_stream_get_super_token(jsmn_stream_parser);

	if (token == NULL)
	{
		return;
	}

	int super_token_id = 0;
	while ((token->parent > -1) && (token->type != JSMN_STREAM_OBJECT))
	{
		super_token_id = token->parent;
		token = &jsmn_stream_parser->tokens[token->parent];
		if (token->type == JSMN_STREAM_OBJECT)
		{
			break;
		}
	}
	token->end = jsmn_stream_parser->char_count;

	// make the parent the new super token
	jsmn_stream_parser->toksuper = super_token_id;
}

static void jsmn_stream_parse_tokens_object_key(const char *key, size_t key_length, void *user_arg)
{
	jsmn_stream_token_parser_t *jsmn_stream_parser = (jsmn_stream_token_parser_t *)user_arg;
	jsmn_streamtok_t *token = jsmn_stream_allocate_token(jsmn_stream_parser);

	if (token == NULL)
	{
		return;
	}

	// update the parent size
	if (jsmn_stream_parser->toksuper != -1)
	{
		jsmn_streamtok_t *parent = jsmn_stream_get_super_token(jsmn_stream_parser);
		parent->size++;
		token->parent = jsmn_stream_parser->toksuper;
	}

	token->type = JSMN_STREAM_KEY;
	token->start = jsmn_stream_parser->char_count - key_length - 1;
	token->end = jsmn_stream_parser->char_count - 1;
	token->size = 0;

	// make this the new super token
	jsmn_stream_parser->toksuper = jsmn_stream_parser->next_token - 1;
}

static void jsmn_stream_parse_tokens_string(const char *value, size_t length, void *user_arg)
{
	jsmn_stream_token_parser_t *jsmn_stream_parser = (jsmn_stream_token_parser_t *)user_arg;
	jsmn_streamtok_t *token = jsmn_stream_allocate_token(jsmn_stream_parser);

	if (token == NULL)
	{
		return;
	}

	// update the parent size
	if (jsmn_stream_parser->toksuper != -1)
	{
		jsmn_streamtok_t *parent = jsmn_stream_get_super_token(jsmn_stream_parser);
		parent->size++;
		token->parent = jsmn_stream_parser->toksuper;
	}

	token->type = JSMN_STREAM_STRING;
	token->start = jsmn_stream_parser->char_count - length - 1;
	token->end = jsmn_stream_parser->char_count - 1;
	token->size = 0;

	// make the super token the last object or array
	while (token->type != JSMN_STREAM_OBJECT && token->type != JSMN_STREAM_ARRAY)
	{
		token = &jsmn_stream_parser->tokens[token->parent];
	}
	jsmn_stream_parser->toksuper = jsmn_stream_get_super_token(jsmn_stream_parser)->parent;
}

static void jsmn_stream_parse_tokens_primitive(const char *value, size_t length, void *user_arg)
{
	jsmn_stream_token_parser_t *jsmn_stream_parser = (jsmn_stream_token_parser_t *)user_arg;
	jsmn_streamtok_t *token = jsmn_stream_allocate_token(jsmn_stream_parser);

	if (token == NULL)
	{
		return;
	}

	// update the parent size
	if (jsmn_stream_parser->toksuper != -1)
	{
		jsmn_streamtok_t *parent = jsmn_stream_get_super_token(jsmn_stream_parser);
		parent->size++;
		token->parent = jsmn_stream_parser->toksuper;
	}

	token->type = JSMN_STREAM_PRIMITIVE;
	token->start = jsmn_stream_parser->char_count - length - 1;
	token->end = jsmn_stream_parser->char_count - 1;
	token->size = length;

	// make the parent's parent the new super token
	jsmn_stream_parser->toksuper = jsmn_stream_get_super_token(jsmn_stream_parser)->parent;
}

static jsmn_stream_callbacks_t jsmn_stream_token_callbacks = {
	.start_array_callback = jsmn_stream_parse_tokens_start_array,
	.end_array_callback = jsmn_stream_parse_tokens_end_array,
	.start_object_callback = jsmn_stream_parse_tokens_start_object,
	.end_object_callback = jsmn_stream_parse_tokens_end_object,
	.object_key_callback = jsmn_stream_parse_tokens_object_key,
	.string_callback = jsmn_stream_parse_tokens_string,
	.primitive_callback = jsmn_stream_parse_tokens_primitive
};

void jsmn_stream_parse_tokens_init(jsmn_stream_token_parser_t *jsmn_stream_parser, jsmn_streamtok_t *tokens, int num_tokens)
{
	jsmn_stream_parser->tokens = tokens;
	jsmn_stream_parser->num_tokens = num_tokens;
	jsmn_stream_parser->next_token = 0;
	jsmn_stream_parser->char_count = 0;
	jsmn_stream_parser->toksuper = -1;
	jsmn_stream_parser->error = JSMN_STREAM_TOKEN_ERROR_NONE;
	jsmn_stream_init(&jsmn_stream_parser->stream_parser, &jsmn_stream_token_callbacks, jsmn_stream_parser);
}

int jsmn_stream_parse_tokens(jsmn_stream_token_parser_t *jsmn_stream_token_parser, char c)
{
	jsmn_stream_token_parser->char_count++;
	jsmn_stream_parse((jsmn_stream_parser *)&jsmn_stream_token_parser->stream_parser , c);	

	if (jsmn_stream_token_parser->error != JSMN_STREAM_TOKEN_ERROR_NONE)
	{
		return jsmn_stream_token_parser->error;
	}

	return 0;
}


