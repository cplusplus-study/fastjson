
#ifndef INCLUDE_C_JSONPARSE_H_
#define INCLUDE_C_JSONPARSE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "./json.h"


struct jsonparse_state {
    const char *json;
    int pos;
    int len;
    int depth;
    /* for handling atomic values */
    int vstart;
    int vlen;
    char vtype;
    char error;
    char stack[JSONPARSE_MAX_DEPTH];
};

/**
 * \brief      Initialize a JSON parser state.
 * \param state A pointer to a JSON parser state
 * \param json The string to parse as JSON
 * \param len  The length of the string to parse
 *
 *             This function initializes a JSON parser state for
 *             parsing a string as JSON.
 */
void jsonparse_setup(struct jsonparse_state *state, const char *json, int len);

/* move to next JSON element */
int jsonparse_next(struct jsonparse_state *state);

/* copy the current JSON value into the specified buffer */
int jsonparse_copy_value(struct jsonparse_state *state, char *buf, int buf_size);

/* get the current JSON value parsed as an int */
int jsonparse_get_value_as_int(struct jsonparse_state *state);

/* get the current JSON value parsed as a long */
long jsonparse_get_value_as_long(struct jsonparse_state *state);

/* get the length of the current JSON value */
int jsonparse_get_len(struct jsonparse_state *state);

/* get the type of the current JSON value */
int jsonparse_get_type(struct jsonparse_state *state);

/* compare the JSON value with the specified string */
int jsonparse_strcmp_value(struct jsonparse_state *state, const char *str);

#ifdef __cplusplus
}
#endif

#endif  // INCLUDE_C_JSONPARSE_H_
