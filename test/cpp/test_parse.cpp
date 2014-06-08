
#include <gtest/gtest.h>
#include <c/jsonparse.h>
#include <cpp/json.hpp>
#include <algorithm>
#include <iterator>
#include <iostream>


TEST(JsonParse, testnext){
    const std::string json_str= R"({"name": "xusd-null", "email": "xyz_kankan@126.com" })";
    std::string err;
    xusd::Json json = xusd::Json::parse(json_str, err);
    EXPECT_TRUE((json.is_object()));
    EXPECT_TRUE((json["name"].is_string()));
    EXPECT_TRUE((json["email"].is_string()));
    EXPECT_EQ("xusd-null", json["name"].string_value());
    EXPECT_EQ("xyz_kankan@126.com", json["email"].string_value());
}


TEST(JsonParse, parseArray){
    const std::string json_str = R"(["1", "2", "3", "4"])";
    std::string err;
    xusd::Json json = xusd::Json::parse(json_str, err);
    EXPECT_TRUE((json.is_array()));
    EXPECT_TRUE((json[0].is_string()));
    EXPECT_TRUE((json[1].is_string()));
    EXPECT_TRUE((json[2].is_string()));
    EXPECT_TRUE((json[3].is_string()));
    EXPECT_EQ((json[0].string_value()), std::string("1"));
    EXPECT_EQ((json[1].string_value()), std::string("2"));
    EXPECT_EQ((json[2].string_value()), std::string("3"));
    EXPECT_EQ((json[3].string_value()), std::string("4"));
}


TEST(JsonParse, number1){
    const std::string json_str = R"([1, 2, 3, 4])";
    std::string err;
    xusd::Json json = xusd::Json::parse(json_str, err);
    EXPECT_TRUE((json.is_array()));
    EXPECT_TRUE((json[0].is_number()));
    EXPECT_TRUE((json[1].is_number()));
    EXPECT_TRUE((json[2].is_number()));
    EXPECT_TRUE((json[3].is_number()));
    EXPECT_EQ((json[0].number_value()), 1);
    EXPECT_EQ((json[1].number_value()), 2);
    EXPECT_EQ((json[2].number_value()), 3);
    EXPECT_EQ((json[3].number_value()), 4);
}

TEST(JsonParse, number2){
    const std::string json_str = R"([1.1, 2.2, 3.3, -4.4])";
    std::string err;
    xusd::Json json = xusd::Json::parse(json_str, err);
    EXPECT_TRUE((json.is_array()));
    EXPECT_TRUE((json[0].is_number()));
    EXPECT_TRUE((json[1].is_number()));
    EXPECT_TRUE((json[2].is_number()));
    EXPECT_TRUE((json[3].is_number()));
    EXPECT_EQ((json[0].number_value()), 1.1);
    EXPECT_EQ((json[1].number_value()), 2.2);
    EXPECT_EQ((json[2].number_value()), 3.3);
    EXPECT_EQ((json[3].number_value()), -4.4);
}
TEST(JsonParse, other){
    const std::string json_str = R"([true, false, null, -1000.1999])";
    std::string err;
    xusd::Json json = xusd::Json::parse(json_str, err);
    EXPECT_TRUE((json.is_array()));
    EXPECT_TRUE((json[0].is_bool()));
    EXPECT_TRUE((json[1].is_bool()));
    EXPECT_TRUE((json[2].is_null()));
    EXPECT_TRUE((json[3].is_number()));
    EXPECT_EQ((json[0].bool_value()), true);
    EXPECT_EQ((json[1].bool_value()), false);
    EXPECT_EQ((json[3].number_value()), -1000.1999);
}

int main(int argc, char* argv[]){
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
