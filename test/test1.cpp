
#include <gtest/gtest.h>
#include <jsonparse.h>
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

}

int main(int argc, char* argv[]){
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
