
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

static void dump(std::nullptr_t, string &out) {
    out += "null";
}

static void dump(double value, string &out) {
    char buf[32];
    snprintf(buf, sizeof buf, "%.17g", value);
    out += buf;
}

static void dump(int value, string &out) {
    char buf[32];
    snprintf(buf, sizeof buf, "%d", value);
    out += buf;
}

static void dump(bool value, string &out) {
    out += value ? "true" : "false";
}

static void dump(const string &value, string &out) {
    out += '"';
    for (size_t i = 0; i < value.length(); i++) {
        const char ch = value[i];
        if (ch == '\\') {
            out += "\\\\";
        } else if (ch == '"') {
            out += "\\\"";
        } else if (ch == '\b') {
            out += "\\b";
        } else if (ch == '\f') {
            out += "\\f";
        } else if (ch == '\n') {
            out += "\\n";
        } else if (ch == '\r') {
            out += "\\r";
        } else if (ch == '\t') {
            out += "\\t";
        } else if ((uint8_t)ch <= 0x1f) {
            char buf[8];
            snprintf(buf, sizeof buf, "\\u%04x", ch);
            out += buf;
        } else if ((uint8_t)ch == 0xe2 && (uint8_t)value[i+1] == 0x80
                   && (uint8_t)value[i+2] == 0xa8) {
            out += "\\u2028";
            i += 2;
        } else if ((uint8_t)ch == 0xe2 && (uint8_t)value[i+1] == 0x80
                   && (uint8_t)value[i+2] == 0xa9) {
            out += "\\u2029";
            i += 2;
        } else {
            out += ch;
        }
    }
    out += '"';
}

static void dump(const Json::array &values, string &out) {
    bool first = true;
    out += "[";
    for (auto &value : values) {
        if (!first)
            out += ", ";
        value.dump(out);
        first = false;
    }
    out += "]";
}

static void dump(const Json::object &values, string &out) {
    bool first = true;
    out += "{";
    for (const std::pair<string, Json> &kv : values) {
        if (!first)
            out += ", ";
        dump(kv.first, out);
        out += ": ";
        kv.second.dump(out);
        first = false;
    }
    out += "}";
}

void Json::dump(string &out) const {
    m_ptr->dump(out);
}

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
    void dump(string &out) const { xusd::dump(m_value, out); }
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


    bool isFailed(){
        return __state.error != JSON_ERROR_OK;
    }

    string& failMsg(string &err){
        err.clear();
        switch(__state.error){
            case JSON_ERROR_OK:
                break;
            case JSON_ERROR_SYNTAX:
                err = "syntax error near: \n";
                break;
            case JSON_ERROR_UNEXPECTED_ARRAY:
                err = "unexpected array near: \n";
                break;
            case JSON_ERROR_UNEXPECTED_END_OF_ARRAY:
                err = "unexpected end of array near: \n";
                break;
            case JSON_ERROR_UNEXPECTED_OBJECT:
                err = "unexpected object near: \n";
                break;
            case JSON_ERROR_UNEXPECTED_STRING:
                err = "unexpected string near: \n";
                break;
            case JSON_ERROR_MAXDEPTH:
                err = "unexpected string near: \n";
                break;
        }

        const char* s = __state.json + __state.pos;
        const char* e = __state.pos + 64 < __state.len ? __state.json + __state.pos + 64: __state.json + __state.len;
        for(int i =0; i<64; ++i){
            s = __state.json + __state.pos - i;
            if((s <= __state.json)){
                break;
            }
            if(*s == '\n'){
                if(__state.json + __state.pos - s > 32){
                    break;
                }
            }
        }
        for(int i =0; i<64; ++i){
            e = __state.json + __state.pos + i;
            if((e >= __state.json + __state.len) || (*e == '\n')){
                break;
            }
        }
        int pos = __state.json + __state.pos - s;
        err.append(s, e);
        err.append("\n");
        err.append(pos/2, ' ');
        err.append(pos/2, '_');
        err.append("^\n");
        return err;
    }

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

    /* parse_json()
     *
     * Parse a JSON object.
     */
    Json parse_json() {
        if(__state.error != JSON_ERROR_OK){
            return Json();
        }
        if (__state.depth > max_depth) {
            __state.error = JSON_ERROR_MAXDEPTH;
            return Json();
        }

        int n = 0;
        if(__state.vtype == 0){
            n = jsonparse_next(&__state);
        } else {
            n = __state.vtype;
        }
        if(n == JSON_TYPE_ERROR){
            return Json();
        }

        switch(n){
            case JSON_TYPE_NUMBER:
                return parse_number();
            case JSON_TYPE_TRUE:
                return true;
            case JSON_TYPE_FALSE:
                return false;
            case JSON_TYPE_NULL:
                return nullptr;
            case JSON_TYPE_STRING:
                return string(__state.json + __state.vstart, __state.vlen);
            case JSON_TYPE_OBJECT:
                {
                    map<string, Json> data;

                    while(true){
                        int ch = jsonparse_next(&__state);
                        if(ch == JSON_TYPE_ERROR){
                            return Json();
                        }
                        if(ch == '}'){
                            break;
                        }
                        if(ch == ','){
                            continue;
                        }
                        if(ch != JSON_TYPE_PAIR_NAME){
                            __state.error = JSON_ERROR_UNEXPECTED_OBJECT;
                            return Json();
                        }
                        string key(__state.json + __state.vstart, __state.vlen);
                        ch = jsonparse_next(&__state);
                        if(ch != ':'){
                            __state.error = JSON_ERROR_UNEXPECTED_OBJECT;
                            return Json();
                        }
                        data[key] = parse_json();
                    }
                    return data;
                }
                break;
            case JSON_TYPE_ARRAY:
                {
                    vector<Json> data;
                    while(true){
                        int ch = jsonparse_next(&__state);
                        if(ch == JSON_TYPE_ERROR){
                            return Json();
                        }
                        if(ch == ']'){
                            break;
                        }
                        if(ch == ','){
                            continue;
                        }
                        data.push_back(parse_json());
                    }
                    return data;
                }
                break;
            case JSON_TYPE_PAIR_NAME:
                return string(__state.json + __state.vstart, __state.vlen);
                break;
        }
        return Json();
    }
};

Json Json::parse(const string &in, string &err) {
    JsonParser parser (in);
    Json result = parser.parse_json();
    if(parser.isFailed()){
        parser.failMsg(err);
    }
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
