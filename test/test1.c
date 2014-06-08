#include <string.h>
#include <stdio.h>
#include <json.h>
#include <jsonparse.h>
#include <jsontree.h>

int main(){
    const char* json = "{ \"items\": [ { \"name\": \"command\", \"index\": \"X\", \"optional\": \"0\" }, { \"name\": \"status\", \"index\": \"X\", \"optional\": \"0\" } ] }";
    struct jsonparse_state state;

    jsonparse_setup(&state, json,strlen(json));
    char c;
    while((c = jsonparse_next(&state)) != 0){
        switch(c){
            case '[':
                break;
            case '{':
                break;
            case ':':
                break;
            case 'N':
                break;
            case '"':
                break;
            case 'I':
                break;
            case '0':
                break;
            case 0:
            break;
        }
    }
    return 0;
}
