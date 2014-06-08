
#include <c/json.h>
#include <c/jsonparse.h>
#include <cpp/json.hpp>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <limits>

namespace xusd{

static const int max_depth = JSONPARSE_MAX_DEPTH;

using std::string;
using std::vector;
using std::map;
using std::make_shared;
using std::initializer_list;
using std::move;

/* * * * * * * * * * * * * * * * * * * *
 * Value wrappers
 */

template <Json::Type tag, typename T>
class Value : public JsonValue {
protected:

    // Constructors
    Value(const T &value) : m_value(value) {}
    Value(T &&value)      : m_value(move(value)) {}

    // Get type tag
    Json::Type type() const {
        return tag;
    }

    // Comparisons
    bool equals(const JsonValue * other) const {
        return m_value == reinterpret_cast<const Value<tag, T> *>(other)->m_value;
    }
    bool less(const JsonValue * other) const {
        return m_value < reinterpret_cast<const Value<tag, T> *>(other)->m_value;
    }

    const T m_value;
    //void dump(string &out) const { xusd::dump(m_value, out); }
};

class JsonDouble final : public Value<Json::NUMBER, double> {
    double number_value() const { return m_value; }
    int int_value() const { return m_value; }
    bool equals(const JsonValue * other) const { return m_value == other->number_value(); }
    bool less(const JsonValue * other)   const { return m_value <  other->number_value(); }
public:
    JsonDouble(double value) : Value(value) {}
};

class JsonInt final : public Value<Json::NUMBER, int> {
    double number_value() const { return m_value; }
    int int_value() const { return m_value; }
    bool equals(const JsonValue * other) const { return m_value == other->number_value(); }
    bool less(const JsonValue * other)   const { return m_value <  other->number_value(); }
public:
    JsonInt(double value) : Value(value) {}
};

class JsonBoolean final : public Value<Json::BOOL, bool> {
    bool bool_value() const { return m_value; }
public:
    JsonBoolean(bool value) : Value(value) {}
};

class JsonString final : public Value<Json::STRING, string> {
    const string &string_value() const { return m_value; }
public:
    JsonString(const string &value) : Value(value) {}
    JsonString(string &&value)      : Value(move(value)) {}
};

class JsonArray final : public Value<Json::ARRAY, Json::array> {
    const Json::array &array_items() const { return m_value; }
    const Json & operator[](size_t i) const;
public:
    JsonArray(const Json::array &value) : Value(value) {}
    JsonArray(Json::array &&value)      : Value(move(value)) {}
};

class JsonObject final : public Value<Json::OBJECT, Json::object> {
    const Json::object &object_items() const { return m_value; }
    const Json & operator[](const string &key) const;
public:
    JsonObject(const Json::object &value) : Value(value) {}
    JsonObject(Json::object &&value)      : Value(move(value)) {}
};

class JsonNull final : public Value<Json::NUL, std::nullptr_t> {
public:
    JsonNull() : Value(nullptr) {}
};

/* * * * * * * * * * * * * * * * * * * *
 * Static globals - static-init-safe
 */
struct Statics {
    const std::shared_ptr<JsonValue> null = make_shared<JsonNull>();
    const std::shared_ptr<JsonValue> t = make_shared<JsonBoolean>(true);
    const std::shared_ptr<JsonValue> f = make_shared<JsonBoolean>(false);
    const string empty_string;
    const vector<Json> empty_vector;
    const map<string, Json> empty_map;
};

const Statics & statics() {
    static const Statics s {};
    return s;
}

const Json & static_null() {
    // This has to be separate, not in Statics, because Json() accesses statics().null.
    static const Json json_null;
    return json_null;
}

/* * * * * * * * * * * * * * * * * * * *
 * Constructors
 */

Json::Json() noexcept                  : m_ptr(statics().null) {}
Json::Json(std::nullptr_t) noexcept    : m_ptr(statics().null) {}
Json::Json(double value)               : m_ptr(make_shared<JsonDouble>(value)) {}
Json::Json(int value)                  : m_ptr(make_shared<JsonInt>(value)) {}
Json::Json(bool value)                 : m_ptr(value ? statics().t : statics().f) {}
Json::Json(const string &value)        : m_ptr(make_shared<JsonString>(value)) {}
Json::Json(string &&value)             : m_ptr(make_shared<JsonString>(move(value))) {}
Json::Json(const char * value)         : m_ptr(make_shared<JsonString>(value)) {}
Json::Json(const Json::array &values)  : m_ptr(make_shared<JsonArray>(values)) {}
Json::Json(Json::array &&values)       : m_ptr(make_shared<JsonArray>(move(values))) {}
Json::Json(const Json::object &values) : m_ptr(make_shared<JsonObject>(values)) {}
Json::Json(Json::object &&values)      : m_ptr(make_shared<JsonObject>(move(values))) {}

/* * * * * * * * * * * * * * * * * * * *
 * Accessors
 */

Json::Type Json::type()                           const { return m_ptr->type();         }
double Json::number_value()                       const { return m_ptr->number_value(); }
int Json::int_value()                             const { return m_ptr->int_value();    }
bool Json::bool_value()                           const { return m_ptr->bool_value();   }
const string & Json::string_value()               const { return m_ptr->string_value(); }
const vector<Json> & Json::array_items()          const { return m_ptr->array_items();  }
const map<string, Json> & Json::object_items()    const { return m_ptr->object_items(); }
const Json & Json::operator[] (size_t i)          const { return (*m_ptr)[i];           }
const Json & Json::operator[] (const string &key) const { return (*m_ptr)[key];         }

double                    JsonValue::number_value()              const { return 0; }
int                       JsonValue::int_value()                 const { return 0; }
bool                      JsonValue::bool_value()                const { return false; }
const string &            JsonValue::string_value()              const { return statics().empty_string; }
const vector<Json> &      JsonValue::array_items()               const { return statics().empty_vector; }
const map<string, Json> & JsonValue::object_items()              const { return statics().empty_map; }
const Json &              JsonValue::operator[] (size_t)         const { return static_null(); }
const Json &              JsonValue::operator[] (const string &) const { return static_null(); }

const Json & JsonObject::operator[] (const string &key) const {
    auto iter = m_value.find(key);
    return (iter == m_value.end()) ? static_null() : iter->second;
}
const Json & JsonArray::operator[] (size_t i) const {
    if (i >= m_value.size()) return static_null();
    else return m_value[i];
}

/* * * * * * * * * * * * * * * * * * * *
 * Comparison
 */

bool Json::operator== (const Json &other) const {
    if (m_ptr->type() != other.m_ptr->type())
        return false;

    return m_ptr->equals(other.m_ptr.get());
}

bool Json::operator< (const Json &other) const {
    if (m_ptr->type() != other.m_ptr->type())
        return m_ptr->type() < other.m_ptr->type();

    return m_ptr->less(other.m_ptr.get());
}

/* JsonParser
 *
 * Object that tracks all state of an in-progress parse.
 */
class JsonParser {
    const std::string& json_str;
public:
    JsonParser(const std::string& in):json_str(in){
        jsonparse_setup(&__state, json_str.data(), json_str.size());
    }
    /* State
     */
    struct jsonparse_state __state;

    /* encode_utf8(pt, out)
     *
     * Encode pt as UTF-8 and add it to out.
     */
    void encode_utf8(long pt, string & out) {
        if (pt < 0)
            return;

        if (pt < 0x80) {
            out += pt;
        } else if (pt < 0x800) {
            out += (pt >> 6) | 0xC0;
            out += (pt & 0x3F) | 0x80;
        } else if (pt < 0x10000) {
            out += (pt >> 12) | 0xE0;
            out += ((pt >> 6) & 0x3F) | 0x80;
            out += (pt & 0x3F) | 0x80;
        } else {
            out += (pt >> 18) | 0xF0;
            out += ((pt >> 12) & 0x3F) | 0x80;
            out += ((pt >> 6) & 0x3F) | 0x80;
            out += (pt & 0x3F) | 0x80;
        }
    }

    /* parse_string()
     *
     * Parse a string, starting at the current position.
     */
    string parse_string() {
        return std::string(__state.json + __state.vstart, __state.vlen);
    }

    /* parse_number()
     *
     * Parse a double.
     */
    Json parse_number() {
        return std::atof(__state.json + __state.vstart);
    }

    /* expect(str, res)
     *
     * Expect that 'str' starts at the character that was just read. If it does, advance
     * the input and return res. If not, flag an error.
     */
    Json expect(const string &expected, Json res) {
        //return fail("parse error: expected " + expected + ", got " + "");
        return Json();
    }

    /* parse_json()
     *
     * Parse a JSON object.
     */
    Json parse_json(int depth) {
        if (depth > max_depth) {
            return Json();
            //return fail("exceeded maximum nesting depth");
        }

        int n = jsonparse_next(&__state);
        if(n == JSON_TYPE_ERROR){

            return Json();
            //return fail("exceeded maximum nesting depth");
        }

        switch(n){
            case JSON_TYPE_NUMBER:
                return parse_number();
            case JSON_TYPE_TRUE:
                return expect("true", true);
            case JSON_TYPE_FALSE:
                return expect("false", false);
            case JSON_TYPE_NULL:
                return expect("null", nullptr);
            case JSON_TYPE_OBJECT:
                break;
            case JSON_TYPE_ARRAY:
                break;
            case JSON_TYPE_PAIR_NAME:
                break;
            case JSON_TYPE_PAIR:
                break;
            case ',':
                break;
        }

        return Json();
    }
};

Json Json::parse(const string &in, string &err) {
    JsonParser parser (in);
    Json result = parser.parse_json(0);

    return result;
}

vector<Json> Json::parse_multi(const string &in, string &err) {
    JsonParser parser { in };
    vector<Json> json_vec;
    return json_vec;
}

/* * * * * * * * * * * * * * * * * * * *
 * Shape-checking
 */

bool Json::has_shape(const shape & types, string & err) const {
    return true;
}

}