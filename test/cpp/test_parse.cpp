
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
    std::cout<<json.dump()<<std::endl;
}


TEST(JsonParse, parseArray){
    const std::string json_str = R"(["1", "2", "3", "4"])";
    std::string err;
    xusd::Json json = xusd::Json::parse(json_str, err);
    std::cout<<json.dump()<<std::endl;
}


TEST(JsonParse, number1){
    const std::string json_str = R"([1, 2, 3, 4])";
    std::string err;
    xusd::Json json = xusd::Json::parse(json_str, err);
    std::cout<<json.dump()<<std::endl;
}

TEST(JsonParse, number2){
    const std::string json_str = R"([1.1, 2.2, 3.3, -4.4])";
    std::string err;
    xusd::Json json = xusd::Json::parse(json_str, err);
    std::cout<<json.dump()<<std::endl;
}
TEST(JsonParse, other){
    const std::string json_str = R"([true, false, null, -1000.1999])";
    std::string err;
    xusd::Json json = xusd::Json::parse(json_str, err);
    std::cout<<json.dump()<<std::endl;
}

int main(int argc, char* argv[]){
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
