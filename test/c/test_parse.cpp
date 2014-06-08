
#include <gtest/gtest.h>
#include <c/jsonparse.h>
#include <cpp/json.hpp>
#include <algorithm>
#include <iterator>
#include <iostream>

TEST(JsonParse, testsetup){
    struct jsonparse_state state1 = {
        (const char*)0,/*const char *json;*/
        1,/*int pos;*/
        1,/*int len;*/
        1,/*int depth;*/
        1,/*int vstart;*/
        1,/*int vlen;*/
        1,/*char vtype;*/
        1,/*char error;*/
        {1}/*char stack[JSONPARSE_MAX_DEPTH];*/
    };
    std::fill(state1.stack, state1.stack+JSONPARSE_MAX_DEPTH, 1);
    const char* json1 = "";
    jsonparse_setup(&state1, json1, strlen(json1));

    EXPECT_EQ(json1, state1.json);
    EXPECT_EQ(0, state1.pos);
    EXPECT_EQ(strlen(json1), state1.len);
    EXPECT_EQ(0, state1.depth);
    EXPECT_EQ(0, state1.vstart);
    EXPECT_EQ(0, state1.vlen);
    EXPECT_EQ(0, state1.vtype);
    EXPECT_EQ(0, state1.error);
    EXPECT_EQ(0, state1.stack[0]);
    EXPECT_TRUE((std::all_of(state1.stack+1, state1.stack+JSONPARSE_MAX_DEPTH, [](char v)->bool{ return v == 1; })));
}

TEST(JsonParse, testnext){
    struct jsonparse_state state1;
    const char* json1 = R"({"name": "xusd-null", "email": "xyz_kankan@126.com" })";
    jsonparse_setup(&state1, json1, strlen(json1));

    EXPECT_EQ('{', (jsonparse_next(&state1)));
    EXPECT_EQ(1, state1.depth);
    EXPECT_EQ(0, state1.vtype);

    EXPECT_EQ('N', (jsonparse_next(&state1)));
    EXPECT_EQ(1, state1.depth);
    EXPECT_EQ(JSON_TYPE_PAIR_NAME, state1.vtype);
    EXPECT_EQ(4, state1.vlen);
    char buf[32];
    jsonparse_copy_value(&state1, buf, sizeof(buf));
    EXPECT_STREQ("name", buf);

    EXPECT_EQ(':', (jsonparse_next(&state1)));
    EXPECT_EQ(2, state1.depth);
    EXPECT_EQ(0, state1.vtype);


    EXPECT_EQ('"', (jsonparse_next(&state1)));
    EXPECT_EQ(2, state1.depth);
    EXPECT_EQ(JSON_TYPE_STRING, state1.vtype);
    EXPECT_EQ(9, state1.vlen);
    jsonparse_copy_value(&state1, buf, sizeof(buf));
    EXPECT_STREQ("xusd-null", buf);


    EXPECT_EQ(',', (jsonparse_next(&state1)));
    EXPECT_EQ(1, state1.depth);


    EXPECT_EQ('N', (jsonparse_next(&state1)));
    EXPECT_EQ(1, state1.depth);
    EXPECT_EQ(JSON_TYPE_PAIR_NAME, state1.vtype);
    EXPECT_EQ(5, state1.vlen);
    jsonparse_copy_value(&state1, buf, sizeof(buf));
    EXPECT_STREQ("email", buf);

    EXPECT_EQ(':', (jsonparse_next(&state1)));
    EXPECT_EQ(2, state1.depth);
    EXPECT_EQ(0, state1.vtype);

    EXPECT_EQ('"', (jsonparse_next(&state1)));
    EXPECT_EQ(2, state1.depth);
    EXPECT_EQ(JSON_TYPE_STRING, state1.vtype);
    EXPECT_EQ(strlen("xyz_kankan@126.com"), state1.vlen);
    jsonparse_copy_value(&state1, buf, sizeof(buf));
    EXPECT_STREQ("xyz_kankan@126.com", buf);

    EXPECT_EQ('}', (jsonparse_next(&state1)));
    EXPECT_EQ(0, state1.depth);
    //EXPECT_EQ(0, state1.vtype);

    EXPECT_EQ(0, (jsonparse_next(&state1)));
}


TEST(JsonParse, parseArray){
    struct jsonparse_state state1;
    const char* json1 = R"(["1", "2", "3", "4"])";
    jsonparse_setup(&state1, json1, strlen(json1));

    EXPECT_EQ(JSON_TYPE_ARRAY, jsonparse_next(&state1));
    EXPECT_EQ(JSON_TYPE_STRING, jsonparse_next(&state1));
    EXPECT_EQ(',', jsonparse_next(&state1));
    EXPECT_EQ(JSON_TYPE_STRING, jsonparse_next(&state1));
    EXPECT_EQ(',', jsonparse_next(&state1));
    EXPECT_EQ(JSON_TYPE_STRING, jsonparse_next(&state1));
    EXPECT_EQ(',', jsonparse_next(&state1));
    EXPECT_EQ(JSON_TYPE_STRING, jsonparse_next(&state1));
    EXPECT_EQ(']', jsonparse_next(&state1));
    EXPECT_EQ(0, (jsonparse_next(&state1)));
}


TEST(JsonParse, number1){
    struct jsonparse_state state1;
    const char* json1 = R"([1, 2, 3, 4])";
    jsonparse_setup(&state1, json1, strlen(json1));

    EXPECT_EQ(JSON_TYPE_ARRAY, jsonparse_next(&state1));
    EXPECT_EQ(JSON_TYPE_NUMBER, jsonparse_next(&state1));
    EXPECT_EQ(1, atoi(state1.json + state1.vstart));
    EXPECT_EQ(',', jsonparse_next(&state1));
    EXPECT_EQ(JSON_TYPE_NUMBER, jsonparse_next(&state1));
    EXPECT_EQ(2, atoi(state1.json + state1.vstart));
    EXPECT_EQ(',', jsonparse_next(&state1));
    EXPECT_EQ(JSON_TYPE_NUMBER, jsonparse_next(&state1));
    EXPECT_EQ(3, atoi(state1.json + state1.vstart));
    EXPECT_EQ(',', jsonparse_next(&state1));
    EXPECT_EQ(JSON_TYPE_NUMBER, jsonparse_next(&state1));
    EXPECT_EQ(4, atoi(state1.json + state1.vstart));
    EXPECT_EQ(']', jsonparse_next(&state1));
    EXPECT_EQ(0, (jsonparse_next(&state1)));
}

TEST(JsonParse, number2){
    struct jsonparse_state state1;
    const char* json1 = R"([1.1, 2.2, 3.3, -4.4])";
    jsonparse_setup(&state1, json1, strlen(json1));

    EXPECT_EQ(JSON_TYPE_ARRAY, jsonparse_next(&state1));
    EXPECT_EQ(JSON_TYPE_NUMBER, jsonparse_next(&state1));
    EXPECT_EQ(1.1, atof(state1.json + state1.vstart));
    EXPECT_EQ(',', jsonparse_next(&state1));
    EXPECT_EQ(JSON_TYPE_NUMBER, jsonparse_next(&state1));
    EXPECT_EQ(2.2, atof(state1.json + state1.vstart));
    EXPECT_EQ(',', jsonparse_next(&state1));
    EXPECT_EQ(JSON_TYPE_NUMBER, jsonparse_next(&state1));
    EXPECT_EQ(3.3, atof(state1.json + state1.vstart));
    EXPECT_EQ(',', jsonparse_next(&state1));
    EXPECT_EQ(JSON_TYPE_NUMBER, jsonparse_next(&state1));
    EXPECT_EQ(-4.4, atof(state1.json + state1.vstart));
    EXPECT_EQ(']', jsonparse_next(&state1));
    EXPECT_EQ(0, (jsonparse_next(&state1)));
}
TEST(JsonParse, other){
    struct jsonparse_state state1;
    const char* json1 = R"([true, false, null, -1000.1999])";
    jsonparse_setup(&state1, json1, strlen(json1));

    EXPECT_EQ(JSON_TYPE_ARRAY, jsonparse_next(&state1));
    EXPECT_EQ(JSON_TYPE_TRUE, jsonparse_next(&state1));
    EXPECT_TRUE((strncmp(state1.json + state1.vstart, "true", 4) == 0));
    EXPECT_EQ(',', jsonparse_next(&state1));
    EXPECT_EQ(JSON_TYPE_FALSE, jsonparse_next(&state1));
    EXPECT_TRUE((strncmp(state1.json + state1.vstart, "false", 5) == 0));
    EXPECT_EQ(',', jsonparse_next(&state1));
    EXPECT_EQ(JSON_TYPE_NULL, jsonparse_next(&state1));
    EXPECT_TRUE((strncmp(state1.json + state1.vstart, "null", 4) == 0));
    EXPECT_EQ(',', jsonparse_next(&state1));
    EXPECT_EQ(JSON_TYPE_NUMBER, jsonparse_next(&state1));
    EXPECT_EQ(-1000.1999, atof(state1.json + state1.vstart));
    EXPECT_EQ(']', jsonparse_next(&state1));
    EXPECT_EQ(0, (jsonparse_next(&state1)));
}

int main(int argc, char* argv[]){
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
