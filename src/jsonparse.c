#include <c/jsonparse.h>
#include <stdlib.h>
#include <string.h>

/*--------------------------------------------------------------------*/
static void clear_value(struct jsonparse_state*state){
    state->vtype = 0;
    state->vlen = 0;
    state->vstart = 0;
}
/*--------------------------------------------------------------------*/
static int push(struct jsonparse_state *state, char c) {
    state->stack[state->depth] = c;
    state->depth++;
    clear_value(state);
    return state->depth < JSONPARSE_MAX_DEPTH;
}
/*--------------------------------------------------------------------*/
static char pop(struct jsonparse_state *state) {
    if(state->depth == 0) {
        return JSON_TYPE_ERROR;
    }
    state->depth--;
    return state->stack[state->depth];
}
/*--------------------------------------------------------------------*/
/* will pass by the value and store the start and length of the value for
   atomic types */
/*--------------------------------------------------------------------*/
static void atomic(struct jsonparse_state *state, char type) {
    char c;

    state->vstart = state->pos;
    state->vtype = type;
    if(type == JSON_TYPE_STRING || type == JSON_TYPE_PAIR_NAME) {
        while((c = state->json[state->pos++]) && c != '"') {
            if(c == '\\') {
                state->pos++;           /* skip current char */
            }
        }
        state->vlen = state->pos - state->vstart - 1;
    } else if(type == JSON_TYPE_NUMBER) {
        do {
            c = state->json[state->pos];
            if((c < '0' || c > '9') && c != '.') {
                c = 0;
            } else {
                state->pos++;
            }
        } while(c);
        /* need to back one step since first char is already gone */
        state->vstart--;
        state->vlen = state->pos - state->vstart;
    } else if(type == JSON_TYPE_NULL){
        state->vstart--;
        state->vlen = 4;
        state->pos +=3;
    } else if(type == JSON_TYPE_TRUE){
        state->vstart--;
        state->vlen = 4;
        state->pos +=3;
    } else if(type == JSON_TYPE_FALSE){
        state->vstart--;
        state->vlen = 5;
        state->pos +=4;
    }
    /* no other types for now... */
}
/*--------------------------------------------------------------------*/
static void skip_ws(struct jsonparse_state *state) {
    char c;

    while(state->pos < state->len &&
            ((c = state->json[state->pos]) == ' ' || c == '\n' || c == '\r' || c == '\t')) {
        state->pos++;
    }
}
/*--------------------------------------------------------------------*/
void jsonparse_setup(struct jsonparse_state *state, const char *json, int len) {
    state->json = json;
    state->len = len;
    state->pos = 0;
    state->depth = 0;
    state->vstart = 0;
    state->vlen = 0;
    state->vtype = 0;
    state->error = 0;
    state->stack[0] = 0;
}
/*--------------------------------------------------------------------*/
int jsonparse_next(struct jsonparse_state *state) {
    char c;
    char s;

    skip_ws(state);
    c = state->json[state->pos];
    s = jsonparse_get_type(state);
    state->pos++;

    switch(c) {
        case '{':
            push(state, c);
            return c;
        case '}':
            if(s == ':' && state->vtype != 0) {
                /*       printf("Popping vtype: '%c'\n", state->vtype); */
                pop(state);
                s = jsonparse_get_type(state);
            }
            if(s == '{') {
                pop(state);
            } else {
                state->error = JSON_ERROR_SYNTAX;
                return JSON_TYPE_ERROR;
            }
            return c;
        case ']':
            if(s == '[') {
                pop(state);
            } else {
                state->error = JSON_ERROR_UNEXPECTED_END_OF_ARRAY;
                return JSON_TYPE_ERROR;
            }
            return c;
        case ':':
            push(state, c);
            return c;
        case ',':
            /* if x:y ... , */
            if(s == ':' && state->vtype != 0) {
                pop(state);
            } else if(s == '[') {
                /* ok! */
            } else {
                state->error = JSON_ERROR_SYNTAX;
                return JSON_TYPE_ERROR;
            }
            return c;
        case '"':
            if(s == '{' || s == '[' || s == ':') {
                atomic(state, c = (s == '{' ? JSON_TYPE_PAIR_NAME : c));
            } else {
                state->error = JSON_ERROR_UNEXPECTED_STRING;
                return JSON_TYPE_ERROR;
            }
            return c;
        case '[':
            push(state, c);
            return c;
        case 'n':
            if((s == ':' || s == '[') && strncmp("null", state->json + state->pos -1, 4) == 0) {
                atomic(state, JSON_TYPE_NULL);
                return JSON_TYPE_NULL;
            }else{
                state->error = JSON_ERROR_SYNTAX;
                return JSON_TYPE_ERROR;
            }
            break;
        case 't':
            if((s == ':' || s == '[') && strncmp("true", state->json + state->pos -1, 4) == 0) {
                atomic(state, JSON_TYPE_TRUE);
                return JSON_TYPE_TRUE;
            }else{
                state->error = JSON_ERROR_SYNTAX;
                return JSON_TYPE_ERROR;
            }
            break;
        case 'f':
            if((s == ':' || s == '[') && strncmp("false", state->json + state->pos -1, 5) == 0) {
                atomic(state, JSON_TYPE_FALSE);
                return JSON_TYPE_FALSE;
            }else {
                state->error = JSON_ERROR_SYNTAX;
                return JSON_TYPE_ERROR;
            }
            break;
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            if(s == ':' || s == '[') {
                atomic(state, JSON_TYPE_NUMBER);
                return JSON_TYPE_NUMBER;
            }
            state->error = JSON_ERROR_SYNTAX;
            return JSON_TYPE_ERROR;
        default:
            state->error = JSON_ERROR_SYNTAX;
            return JSON_TYPE_ERROR;
    }
    return 0;
}
/*--------------------------------------------------------------------*/
/* get the json value of the current position
 * works only on "atomic" values such as string, number, null, false, true
 */
/*--------------------------------------------------------------------*/
int jsonparse_copy_value(struct jsonparse_state *state, char *str, int size) {
    int i;

    if(state->vtype == 0) {
        return 0;
    }
    size = size <= state->vlen ? (size - 1) : state->vlen;
    for(i = 0; i < size; i++) {
        str[i] = state->json[state->vstart + i];
    }
    str[i] = 0;
    return state->vtype;
}
/*--------------------------------------------------------------------*/
int jsonparse_get_value_as_int(struct jsonparse_state *state) {
    if(state->vtype != JSON_TYPE_NUMBER) {
        return 0;
    }
    return atoi(&state->json[state->vstart]);
}
/*--------------------------------------------------------------------*/
long jsonparse_get_value_as_long(struct jsonparse_state *state) {
    if(state->vtype != JSON_TYPE_NUMBER) {
        return 0;
    }
    return atol(&state->json[state->vstart]);
}
/*--------------------------------------------------------------------*/
/* strcmp - assume no strange chars that needs to be stuffed in string... */
/*--------------------------------------------------------------------*/
int jsonparse_strcmp_value(struct jsonparse_state *state, const char *str) {
    if(state->vtype == 0) {
        return -1;
    }
    return strncmp(str, &state->json[state->vstart], state->vlen);
}
/*--------------------------------------------------------------------*/
int jsonparse_get_len(struct jsonparse_state *state) {
    return state->vlen;
}
/*--------------------------------------------------------------------*/
int jsonparse_get_type(struct jsonparse_state *state) {
    if(state->depth == 0) {
        return 0;
    }
    return state->stack[state->depth - 1];
}
/*--------------------------------------------------------------------*/
int jsonparse_has_next(struct jsonparse_state *state) {
    return state->pos < state->len;
}
/*--------------------------------------------------------------------*/
