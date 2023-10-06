#include "jsmn_stream_token.h"
#include <stdbool.h>

static jsmn_streamtok_t *jsmn_stream_allocate_token(jsmn_stream_token_parser_t *jsmn_stream_parser);
static jsmn_streamtok_t *jsmn_stream_get_super_token(jsmn_stream_token_parser_t *jsmn_stream_parser);
static void jsmn_stream_parse_tokens_start_array(void *user_arg);
static void jsmn_stream_parse_tokens_end_array(void *user_arg);
static void jsmn_stream_parse_tokens_start_object(void *user_arg);
static void jsmn_stream_parse_tokens_end_object(void *user_arg);
static void jsmn_stream_parse_tokens_object_key(const char *key, size_t key_length, void *user_arg);
static void jsmn_stream_parse_tokens_string(const char *value, size_t length, void *user_arg);
static void jsmn_stream_parse_tokens_primitive(const char *value, size_t length, void *user_arg);

static jsmn_stream_callbacks_t jsmn_stream_token_callbacks = {
	.start_array_callback = jsmn_stream_parse_tokens_start_array,
	.end_array_callback = jsmn_stream_parse_tokens_end_array,
	.start_object_callback = jsmn_stream_parse_tokens_start_object,
	.end_object_callback = jsmn_stream_parse_tokens_end_object,
	.object_key_callback = jsmn_stream_parse_tokens_object_key,
	.string_callback = jsmn_stream_parse_tokens_string,
	.primitive_callback = jsmn_stream_parse_tokens_primitive
};

/**
 * @brief Initialize the jsmn_stream_token_parser_t object.
 * 
 * @param jsmn_stream_parser 
 * @param tokens 
 * @param num_tokens 
 */
void jsmn_stream_parse_tokens_init(jsmn_stream_token_parser_t *jsmn_stream_parser, jsmn_streamtok_t *tokens, int num_tokens)
{
	jsmn_stream_parser->tokens = tokens;
	jsmn_stream_parser->num_tokens = num_tokens;
	jsmn_stream_parser->next_token = 0;
	jsmn_stream_parser->char_count = 0;
	jsmn_stream_parser->super_token_id = JSMN_STREAM_TOKEN_UNDEFINED;
	jsmn_stream_parser->error = JSMN_STREAM_TOKEN_ERROR_NONE;
	jsmn_stream_init(&jsmn_stream_parser->stream_parser, &jsmn_stream_token_callbacks, jsmn_stream_parser);
}

/**
 * @brief Parse a character.
 * 
 * @param jsmn_stream_parser 
 * @param c 
 * @return int 
 */
int jsmn_stream_parse_tokens(jsmn_stream_token_parser_t *jsmn_stream_token_parser, char c)
{
	jsmn_stream_token_parser->char_count++;
	jsmn_stream_parse((jsmn_stream_parser *)&jsmn_stream_token_parser->stream_parser , c);	

	return jsmn_stream_token_parser->error;
}

/**
 * @brief Allocate a new token from the token pool.
 * 
 * @param jsmn_stream_parser 
 * @return jsmn_streamtok_t* 
 */
static jsmn_streamtok_t *jsmn_stream_allocate_token(jsmn_stream_token_parser_t *jsmn_stream_parser)
{
	jsmn_streamtok_t *token;

	// if we are out of tokens, set the error and return NULL
	if (jsmn_stream_parser->next_token >= jsmn_stream_parser->num_tokens)
	{
		jsmn_stream_parser->error = JSMN_STREAM_ERROR_NOMEM;
		return NULL;
	}

	token = &jsmn_stream_parser->tokens[jsmn_stream_parser->next_token];
	token->id = jsmn_stream_parser->next_token;
	jsmn_stream_parser->next_token++;
	token->type = JSMN_STREAM_UNDEFINED;
	token->start = JSMN_STREAM_POSITION_UNDEFINED;
	token->end = JSMN_STREAM_POSITION_UNDEFINED;
	token->size = 0;
	token->parent_id = jsmn_stream_parser->super_token_id;

	// increment the size of the super token, unless it is the root token
	if (token->parent_id != JSMN_STREAM_TOKEN_UNDEFINED)
	{
		jsmn_streamtok_t *parent = jsmn_stream_get_super_token(jsmn_stream_parser);
		if (parent != NULL)
		{
			parent->size++;
		}
	}

	return token;
}

/**
 * @brief Get the super token object or array.
 * 
 * @param jsmn_stream_parser 
 * @return jsmn_streamtok_t* 
 */
static jsmn_streamtok_t *jsmn_stream_get_super_token(jsmn_stream_token_parser_t *jsmn_stream_parser)
{
	if (jsmn_stream_parser->super_token_id == JSMN_STREAM_TOKEN_UNDEFINED)
	{
		jsmn_stream_parser->error = JSMN_STREAM_TOKEN_ERROR_INVALID;
		return NULL;
	}

	return &jsmn_stream_parser->tokens[jsmn_stream_parser->super_token_id];
}

/**
 * @brief Callback used when an array is started.
 * 
 * @param user_arg is a pointer to the jsmn_stream_token_parser_t object.
 */
static void jsmn_stream_parse_tokens_start_array(void *user_arg)
{
	jsmn_stream_token_parser_t *jsmn_stream_parser = (jsmn_stream_token_parser_t *)user_arg;
	jsmn_streamtok_t *token = jsmn_stream_allocate_token(jsmn_stream_parser);

	if (token != NULL)
	{
		token->type = JSMN_STREAM_ARRAY;
		token->start = jsmn_stream_parser->char_count - 1;

		// make this the new super token
		jsmn_stream_parser->super_token_id = jsmn_stream_parser->next_token - 1;
	}
}

/**
 * @brief Callback used when an array is ended.
 * 
 * @param user_arg is a pointer to the jsmn_stream_token_parser_t object.
 */
static void jsmn_stream_parse_tokens_end_array(void *user_arg)
{
	jsmn_stream_token_parser_t *jsmn_stream_parser = (jsmn_stream_token_parser_t *)user_arg;
	jsmn_streamtok_t *token = jsmn_stream_get_super_token(jsmn_stream_parser);

	if (token != NULL)
	{
		int super_token_id = token->parent_id;
		// If the current super token is not pointing at the start of the array
		// step back until we find the start of the array
		while ((token->parent_id > JSMN_STREAM_TOKEN_UNDEFINED) && (token->type != JSMN_STREAM_ARRAY))
		{
			super_token_id = token->parent_id;
			token = &jsmn_stream_parser->tokens[token->parent_id];
		}
		
		token->end = jsmn_stream_parser->char_count; 

		// make the array the new super token
		jsmn_stream_parser->super_token_id = super_token_id;
	}
}

/**
 * @brief Callback used when an object is started.
 * 
 * @param user_arg is a pointer to the jsmn_stream_token_parser_t object.
 */
static void jsmn_stream_parse_tokens_start_object(void *user_arg)
{
	jsmn_stream_token_parser_t *jsmn_stream_parser = (jsmn_stream_token_parser_t *)user_arg;
	jsmn_streamtok_t *token = jsmn_stream_allocate_token(jsmn_stream_parser);

	if (token != NULL)
	{
		token->type = JSMN_STREAM_OBJECT;
		token->start = jsmn_stream_parser->char_count - 1; 

		// make this the new super token
		jsmn_stream_parser->super_token_id = jsmn_stream_parser->next_token - 1;
	}
}

/**
 * @brief Callback used when an object is ended.
 * 
 * @param user_arg is a pointer to the jsmn_stream_token_parser_t object.
 */
static void jsmn_stream_parse_tokens_end_object(void *user_arg)
{
	jsmn_stream_token_parser_t *jsmn_stream_parser = (jsmn_stream_token_parser_t *)user_arg;
	jsmn_streamtok_t *token = jsmn_stream_get_super_token(jsmn_stream_parser);

	if (token != NULL)
	{
		int super_token_id = token->parent_id;

		// If the current super token is not pointing at the start of the object
		// step back until we find it
		while ((token->parent_id > JSMN_STREAM_TOKEN_UNDEFINED) && (token->type != JSMN_STREAM_OBJECT))
		{
			super_token_id = token->parent_id;
			token = &jsmn_stream_parser->tokens[token->parent_id];
		}

		token->end = jsmn_stream_parser->char_count;

		// make the object the new super token
		jsmn_stream_parser->super_token_id = super_token_id;
	}
}

/**
 * @brief Callback used when an object key is parsed.
 * 
 * @param key is a pointer to the key string.
 * @param key_length is the length of the key string.
 * @param user_arg is a pointer to the jsmn_stream_token_parser_t object.
 */
static void jsmn_stream_parse_tokens_object_key(const char *key, size_t key_length, void *user_arg)
{
	jsmn_stream_token_parser_t *jsmn_stream_parser = (jsmn_stream_token_parser_t *)user_arg;
	jsmn_streamtok_t *token = jsmn_stream_allocate_token(jsmn_stream_parser);

	if (token != NULL)
	{
		token->type = JSMN_STREAM_KEY;
		token->start = jsmn_stream_parser->char_count - key_length - 1;
		token->end = jsmn_stream_parser->char_count - 1;
		token->size = 0;

		// make this the new super token
		jsmn_stream_parser->super_token_id = jsmn_stream_parser->next_token - 1; 
	}
}

/**
 * @brief Callback used when a string is parsed.
 * 
 * @param value is a pointer to the string.
 * @param length is the length of the string.
 * @param user_arg is a pointer to the jsmn_stream_token_parser_t object.
 */
static void jsmn_stream_parse_tokens_string(const char *value, size_t length, void *user_arg)
{
	jsmn_stream_token_parser_t *jsmn_stream_parser = (jsmn_stream_token_parser_t *)user_arg;
	jsmn_streamtok_t *token = jsmn_stream_allocate_token(jsmn_stream_parser);

	if (token != NULL)
	{
		token->type = JSMN_STREAM_STRING;
		token->start = jsmn_stream_parser->char_count - length - 1;
		token->end = jsmn_stream_parser->char_count - 1;
		token->size = 0;

		// make the parent's (a key) parent (object or array) the new super token
		jsmn_stream_parser->super_token_id = jsmn_stream_get_super_token(jsmn_stream_parser)->parent_id;
	}
}

/**
 * @brief Callback used when a primitive (number, bool, null) is parsed.
 * 
 * @param value is a pointer to the primitive.
 * @param length is the length of the primitive.
 * @param user_arg is a pointer to the jsmn_stream_token_parser_t object.
 */
static void jsmn_stream_parse_tokens_primitive(const char *value, size_t length, void *user_arg)
{
	jsmn_stream_token_parser_t *jsmn_stream_parser = (jsmn_stream_token_parser_t *)user_arg;
	jsmn_streamtok_t *token = jsmn_stream_allocate_token(jsmn_stream_parser);

	if (token != NULL)
	{
		token->type = JSMN_STREAM_PRIMITIVE;
		token->start = jsmn_stream_parser->char_count - length - 1;
		token->end = jsmn_stream_parser->char_count - 1;
		token->size = length;

		// make the parent's (a key) parent (object or array) the new super token
		jsmn_stream_parser->super_token_id = jsmn_stream_get_super_token(jsmn_stream_parser)->parent_id;
	}
}

