#include <iostream>
#include<variant>
#include<vector>
#include<map>
#include<optional>
#include<string>
#include<fstream>
#include <sstream>
namespace json {

    class Json;
    using J_Null = std::monostate;
    using Bool = bool;
    using Int = int64_t;
    using Float = double;
    using String = std::string;
    using Array = std::vector<Json>;
    using Dict = std::map<std::string, Json>;
    using Data= std::variant<J_Null, Bool, Int, Float, String, Array, Dict>;

    struct Json {
        Data data;
        Json(Data data) :data(data) {}
        Json(const char* str) {
            data = std::string(str);
        }
        Json(const J_Null& p) {
            data = p;
        }
        Json(const Bool& p) {
            data = p;
        }
        Json(const Int& p) {
            data = p;
        }
        Json(const Float& p) {
            data = p;
        }

        Json& operator [](const String& p) {
            if (std::holds_alternative<Dict>(data))
            {
                return std::get<Dict>(data).at(p);
            }
            throw std::runtime_error("object do not extist");
        }
        Json& operator [](const int& p) {
            if (std::holds_alternative<Array>(data))
            {
                return std::get<Array>(data)[p];
            }
            throw std::runtime_error("object do not extist");
        }

        Json& operator =(const Json& p) {
            if (std::holds_alternative<Dict>(data)||std::holds_alternative<Array>(data)) {
                throw std::runtime_error("cannot change dict");
            }
            else
            {
                data = p.data;
                return *this;
            }
        }
    };

    class JsonParser {
    private:
        std::string_view json_str;
        size_t pos = 0;
    public:
        JsonParser(const std::string& str) :json_str(str){}
        void parse_whitespace() {
            while (pos <json_str.size() && std::isspace(json_str[pos]))
            {
                pos++;
            }
        }

        auto parse_null() -> std::optional<Json> {
            std::string_view str = json_str.substr(pos, 4);
            while (str=="null") {
                pos = pos + 4;
                return Json(J_Null());
            }
            return {};
        }

        auto parse_true() -> std::optional<Json> {
            std::string_view str = json_str.substr(pos, 4);
            while (str == "true") {
                pos = pos + 4;
                return Json(true);
            }
            return {};
        }

        auto parse_false() -> std::optional<Json> {
            std::string_view str = json_str.substr(pos, 5);
            while (str == "false") {
                pos = pos + 5;
                return Json(false);
            }
            return {};
        }

        auto parse_number() -> std::optional<Json> {
            std::string str;
            while ((json_str[pos] == '.' || json_str[pos]=='e'||std::isdigit(json_str[pos]))&&pos<json_str.size())
            {
                str.push_back(json_str[pos]);
                pos++;
            }
   
            auto is_float = [](const std::string& number)->bool {
                if (number.find('.')==number.npos||number.find('e')== number.npos)
                {
                    return true;
                }
                return false;
            };
            if (is_float(str))
            {
                return Json(Float (std::stod(str)));
            }
            return Json(Int(std::stoi(str)));
        }

        auto parse_string() -> std::optional<Json> {
           pos++;
           std::string str;
           while (json_str[pos] != '"') {
               str.push_back(json_str[pos]);
               pos++;
           }
           pos++;
           return Json(String(str));
        }

        auto parse_array() -> std::optional<Json> {
            pos++;
            Array arr;
            while (json_str[pos] != ']') {
                auto value = parse_value();
                if (value.has_value())
                    arr.push_back(value.value().data);
                parse_whitespace();
                if (json_str[pos] == ',')
                    pos++;
            }
            pos++;
            return Json(arr);
        }

        auto parse_object() -> std::optional<Json> {
            pos++;
            Dict dict;
            String str;
            while (json_str[pos]!='}') {
                parse_whitespace();
                if (json_str[pos] == '"')
                {
                    auto key = parse_value();
                    if(key.has_value())
                        str=std::get<String>(key.value().data);
                }
                if (json_str[pos] == ':')
                    pos++;
                parse_whitespace();
                auto value= parse_value();
                parse_whitespace();
                while ((json_str[pos] == ','|| json_str[pos] == '£¬')&& pos<json_str.size()) {
                    pos++;
                }
                parse_whitespace();
                dict.emplace(str, value.value());
            }
            pos++;
            return Json(dict);
        }

        auto parse_value() -> std::optional<Json> {
            parse_whitespace();
            switch (json_str[pos]) {
            case 'n':
                return parse_null();
            case 't':
                return parse_true();
            case 'f':
                return parse_false();
            case '"':
                return parse_string();
            case '[':
                return parse_array();
            case '{':
                return parse_object();
            default:
                return parse_number();
            }
        }

        auto parse() -> std::optional<Json> {
            auto value=parse_value();
            if (value)
                return value;
            return {};
        }
    };


    auto parser(std::string json_str) ->std::optional<Json>{
        JsonParser p(json_str);
        return p.parse();
    }
    auto  operator << (std::ostream& out, const Json& t) ->std::ostream& {
        if  (std::holds_alternative<Int>(t.data)) {
            out<< std::get<Int>(t.data);
        }
        else if  (std::holds_alternative<String>(t.data)) {
            out<<"\""<<std::get<String>(t.data)<<"\"";
        }
        else if (std::holds_alternative<Float>(t.data)) {
            out << std::get<Float>(t.data);
        }
        else if  (std::holds_alternative<Array>(t.data)) {
            out<<"[";
            auto array = std::get<Array>(t.data);
            for (int i = 0; i < array.size(); i++)
            {
                out << array[i];
                if (i != array.size() - 1)
                {
                    out << ",";
                }
            }
            out << "]";
        }
        else if (std::holds_alternative<Dict>(t.data)) {
            out << "{" << std::endl;
            auto dict = std::get<Dict>(t.data);
            for (auto it = dict.begin(); it != dict.end(); it++)
            {
                if (dict.size() != 1)
                {
                    out <<"\"" << it->first << "\"" << ":" << it->second;
                    out << "," << std::endl;
                }
                else
                {
                    out << "\"" << it->first << "\"" << ":" << it->second;
                }
            }
            out << "}";
        }
        else if(std::holds_alternative<J_Null>(t.data)) {
            out<<"null";
            out << "," ;
        }
        else if (std::holds_alternative<Bool>(t.data)) {
            if(std::get<Bool>(t.data))
                out<<"true";
            else
                out<<"false";
            out << "," ;
        }
        return out;
    }
    
}
using namespace json;


int main() {
    std::ifstream fin("test.txt");
    std::stringstream ss; 
    ss << fin.rdbuf();
    std::string s{ ss.str() };
    auto x = parser(s).value();
    std::cout << x << "\n";
    std::cout<<x["person"]["name"];
    std::cout <<x["person"]["hobby"];
    std::cout <<x["person"]["hobby"][0];
    x["person"]["name"] = "me";
    std::cout << x << "\n";

   /* x["version"] = { 114514LL };
    std::cout << x << "\n";*/
}