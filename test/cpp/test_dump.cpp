#include <cpp/json.hpp>
#include <string>
#include <iostream>
class Point {
public:
    int x;
    int y;
    Point (int x, int y) : x(x), y(y) {}
    xusd::Json to_json() const { return xusd::Json::array { x, y }; }
};

int main(){
    using namespace xusd;
    Json my_json = Json::object {
        { "key1", "value1" },
        { "key2", false },
        { "key3", Json::array { 1, 2, 3 } },
    };
    std::string json_str = my_json.dump();
    std::cout<<json_str<<std::endl;

    std::vector<Point> points = { { 1, 2 }, { 10, 20 }, { 100, 200 } };
    std::string points_json = Json(points).dump();
    std::cout<<points_json<<std::endl;
    return 0;
}
