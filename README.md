a fast json c/c++ lib.

### 1. easy initializer and serialization
```cpp
using namespace xusd;
Json my_json = Json::object {
	{ "key1", "value1" },
		{ "key2", false },
		{ "key3", Json::array { 1, 2, 3 } },
};
std::string json_str = my_json.dump();


// dump ==>

{"key1": "value1", "key2": false, "key3": [1, 2, 3]}
```

### 2. support user-defined type

```
class Point {
public:
    int x;
    int y;
    Point (int x, int y) : x(x), y(y) {}
    xusd::Json to_json() const { return xusd::Json::array { x, y }; }
};

int main(){
    std::vector<Point> points = { { 1, 2 }, { 10, 20 }, { 100, 200 } };
    std::string points_json = Json(points).dump();
    std::cout<<points_json<<std::endl;
}

// dump ==>

[[1, 2], [10, 20], [100, 200]]
```

```cpp
Json json = Json::array { Json::object { { "k", "v" } } };
std::string str = json[0]["k"].string_value();
```

### 3. support c/c++ language

```
#include <c/jsonparse.h> // ==> c lang
#include <cpp/json.hpp> // ==> c++ lang
```


### 4. very fast


### 5. lightweigh


