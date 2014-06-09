a fast json c/c++ lib. (请把好的想法, 以代码的形式提交到该项目, 共同学习, 共同维护.)

### 1. easy to use
```cpp
const std::string json_str = R"({
    "core": { "editor": "vim", "quotepath":false },
        "user":{ "name":"null" , "email":"xyz_kankan@126.com" },
        "color":{ "ui":true },
        "alias":{
            "ci":"commit",
            "co":"checkout",
            "st":"status",
            "br":"branch",
            "hist":"log --pretty=format:\"%h %ad | %s%d [%an]\" --graph --date=short",
            "dump":"cat-file -p",
            "type":"cat-file -t"
        },
        "diff":{ "tool":"vimdiff" },
        "difftool":{ "prompt":false },
        "merge":{ "tool":"whatever_you_want" }
})";
std::string err;
xusd::Json json = xusd::Json::parse(json_str, err);

//===================================================

json["core"]["editor"] ==> "vim"
json["user"]["email"] ==> "xyz_kankan@126.com"
```
### 2. error log

```cpp
const std::string json_str = R"({
"core": { "editor": "vim", "quotepath":false },
"user":{ "name":"null" , "email":"xyz_kankan@126.com" },
"color":{ "ui":true },
"alias":{
	"ci":"commit",
	"co":"checkout",
	"st":"status",
	"br":"branch",
	"hist":"log --pretty=format:\"%h %ad | %s%d [%an]\" --graph --date=short",
	"dump":"cat-file -p",
	"type":"cat-file -t"
},
"diff":{ "tool":"vimdiff" },
"difftool":{ "prompt":false },
"merge":{ "tool":"whatever_you_want" } true
xxyy})";

xusd::Json json = xusd::Json::parse(json_str, err);
EXPECT_FALSE((err.empty()))<<err;
std::cout<<err<<std::endl;

//===================================

"merge":{ "tool":"whatever_you_want" } true
		   ____________________________^

```

### 3. easy initializer and serialization

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

### 4. support user-defined type

```cpp
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

### 5. support c/c++ language

```cpp
#include <c/jsonparse.h> // ==> c lang
#include <cpp/json.hpp> // ==> c++ lang
```


### 6. very fast


### 7. lightweigh


