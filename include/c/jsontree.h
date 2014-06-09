/**
 *  JSON output generation
 */

#ifndef INCLUDE_C_JSONTREE_H_
#define INCLUDE_C_JSONTREE_H_

#include "./json.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef JSONTREE_CONF_MAX_DEPTH
#define JSONTREE_MAX_DEPTH JSONTREE_CONF_MAX_DEPTH
#else
#define JSONTREE_MAX_DEPTH 32
#endif /* JSONTREE_CONF_MAX_DEPTH */

struct jsontree_context {
  struct jsontree_value *values[JSONTREE_MAX_DEPTH];
  uint16_t index[JSONTREE_MAX_DEPTH];
  int (*putchar)(int);
  uint8_t depth;
  uint8_t path;
  int callback_state;
};

struct jsontree_value {
  uint8_t type;
  /* followed by a value */
};

struct jsontree_string {
  uint8_t type;
  const char *value;
};

struct jsontree_int {
  uint8_t type;
  int value;
};

/* NOTE: the jsontree_callback set will receive a jsonparse state */
struct jsonparse_state;
struct jsontree_callback {
  uint8_t type;
  int (* output)(struct jsontree_context *js_ctx);
  int (* set)(struct jsontree_context *js_ctx, struct jsonparse_state *parser);
};

struct jsontree_pair {
  const char *name;
  struct jsontree_value *value;
};

struct jsontree_object {
  uint8_t type;
  uint8_t count;
  struct jsontree_pair *pairs;
};

struct jsontree_array {
  uint8_t type;
  uint8_t count;
  struct jsontree_value **values;
};

#define JSONTREE_STRING(text) {JSON_TYPE_STRING, (text)}
#define JSONTREE_PAIR(name, value) {(name), (struct jsontree_value *)(value)}
#define JSONTREE_CALLBACK(output, set) {JSON_TYPE_CALLBACK, (output), (set)}

#define JSONTREE_OBJECT(name, ...)                                      \
  static struct jsontree_pair jsontree_pair_##name[] = {__VA_ARGS__};   \
  static struct jsontree_object name = {                                \
    JSON_TYPE_OBJECT,							\
    sizeof(jsontree_pair_##name)/sizeof(struct jsontree_pair),          \
    jsontree_pair_##name }

#define JSONTREE_OBJECT_EXT(name, ...)                                  \
  static struct jsontree_pair jsontree_pair_##name[] = {__VA_ARGS__};   \
  struct jsontree_object name = {                                       \
    JSON_TYPE_OBJECT,							\
    sizeof(jsontree_pair_##name)/sizeof(struct jsontree_pair),          \
    jsontree_pair_##name }
    
#define JSONTREE_ARRAY(name, count)                                     \
  static struct jsontree_value *jsontree_value##name[count];            \
  static struct jsontree_array name = {                                 \
    JSON_TYPE_ARRAY,              \
    count,                        \
    jsontree_value##name }         

void jsontree_setup(struct jsontree_context *js_ctx,
                    struct jsontree_value *root, int (* putchar)(int));
void jsontree_reset(struct jsontree_context *js_ctx);

const char *jsontree_path_name(const struct jsontree_context *js_ctx,
                               int depth);

void jsontree_write_int(const struct jsontree_context *js_ctx, int value);
void jsontree_write_atom(const struct jsontree_context *js_ctx,
                         const char *text);
void jsontree_write_string(const struct jsontree_context *js_ctx,
                           const char *text);
int jsontree_print_next(struct jsontree_context *js_ctx);
struct jsontree_value *jsontree_find_next(struct jsontree_context *js_ctx,
                                          int type);

#ifdef __cplusplus
}
#endif

#endif  // INCLUDE_C_JSONTREE_H_
