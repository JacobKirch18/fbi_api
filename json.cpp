#include "json.h"
#include <cctype>
#include <stdexcept>

const JsonValue* JsonValue::find(const std::string& key) const {
    if (type != JsonType::Object) return nullptr;
    auto it = objectValue.find(key);
    if (it == objectValue.end()) return nullptr;
    return &it->second;
}

class JsonParser {
public:
    explicit JsonParser(const std::string& text) : m_text(text), m_pos(0) {}

    JsonValue parse() {
        JsonValue value = parseValue();
        return value;
    }

private:
    const std::string& m_text;
    size_t m_pos;

    char peek() const {
        if (m_pos >= m_text.size()) throw std::runtime_error("Unexpected end of JSON");
        return m_text[m_pos];
    }

    char next() {
        if (m_pos >= m_text.size()) throw std::runtime_error("Unexpected end of JSON");
        return m_text[m_pos++];
    }

    void skipWhitespace() {
        while (m_pos < m_text.size() && std::isspace(static_cast<unsigned char>(m_text[m_pos]))) {
            ++m_pos;
        }
    }

    JsonValue parseValue() {
        skipWhitespace();
        switch (peek()) {
        case '{': return parseObject();
        case '[': return parseArray();
        case '"': return parseString();
        case 't':
        case 'f': return parseBool();
        case 'n': return parseNull();
        default:  return parseNumber();
        }
    }

    JsonValue parseObject() {
        JsonValue value;
        value.type = JsonType::Object;
        next(); // '{'
        skipWhitespace();
        if (peek() == '}') { next(); return value; }

        while (true) {
            skipWhitespace();
            JsonValue key = parseString();
            skipWhitespace();
            if (next() != ':') throw std::runtime_error("Expected ':' in object");
            JsonValue val = parseValue();
            value.objectValue.emplace(std::move(key.stringValue), std::move(val));
            skipWhitespace();
            char c = next();
            if (c == ',') continue;
            if (c == '}') break;
            throw std::runtime_error("Expected ',' or '}' in object");
        }
        return value;
    }

    JsonValue parseArray() {
        JsonValue value;
        value.type = JsonType::Array;
        next(); // '['
        skipWhitespace();
        if (peek() == ']') { next(); return value; }

        while (true) {
            value.arrayValue.push_back(parseValue());
            skipWhitespace();
            char c = next();
            if (c == ',') continue;
            if (c == ']') break;
            throw std::runtime_error("Expected ',' or ']' in array");
        }
        return value;
    }

    unsigned int parseHex4() {
        unsigned int value = 0;
        for (int i = 0; i < 4; ++i) {
            char c = next();
            value <<= 4;
            if (c >= '0' && c <= '9') value |= static_cast<unsigned int>(c - '0');
            else if (c >= 'a' && c <= 'f') value |= static_cast<unsigned int>(c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') value |= static_cast<unsigned int>(c - 'A' + 10);
            else throw std::runtime_error("Invalid \\u escape");
        }
        return value;
    }

    static void appendUtf8(std::string& out, unsigned int codepoint) {
        if (codepoint <= 0x7F) {
            out += static_cast<char>(codepoint);
        }
        else if (codepoint <= 0x7FF) {
            out += static_cast<char>(0xC0 | (codepoint >> 6));
            out += static_cast<char>(0x80 | (codepoint & 0x3F));
        }
        else if (codepoint <= 0xFFFF) {
            out += static_cast<char>(0xE0 | (codepoint >> 12));
            out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (codepoint & 0x3F));
        }
        else {
            out += static_cast<char>(0xF0 | (codepoint >> 18));
            out += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
            out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (codepoint & 0x3F));
        }
    }

    JsonValue parseString() {
        JsonValue value;
        value.type = JsonType::String;
        if (next() != '"') throw std::runtime_error("Expected '\"'");

        std::string result;
        while (true) {
            char c = next();
            if (c == '"') break;
            if (c == '\\') {
                char esc = next();
                switch (esc) {
                case '"':  result += '"';  break;
                case '\\': result += '\\'; break;
                case '/':  result += '/';  break;
                case 'b':  result += '\b'; break;
                case 'f':  result += '\f'; break;
                case 'n':  result += '\n'; break;
                case 'r':  result += '\r'; break;
                case 't':  result += '\t'; break;
                case 'u': {
                    unsigned int codepoint = parseHex4();
                    if (codepoint >= 0xD800 && codepoint <= 0xDBFF &&
                        m_pos + 1 < m_text.size() && m_text[m_pos] == '\\' && m_text[m_pos + 1] == 'u') {
                        m_pos += 2;
                        unsigned int low = parseHex4();
                        codepoint = 0x10000 + ((codepoint - 0xD800) << 10) + (low - 0xDC00);
                    }
                    appendUtf8(result, codepoint);
                    break;
                }
                default: result += esc; break;
                }
            }
            else {
                result += c;
            }
        }
        value.stringValue = std::move(result);
        return value;
    }

    JsonValue parseBool() {
        JsonValue value;
        value.type = JsonType::Bool;
        if (m_text.compare(m_pos, 4, "true") == 0) {
            value.boolValue = true;
            m_pos += 4;
        }
        else if (m_text.compare(m_pos, 5, "false") == 0) {
            value.boolValue = false;
            m_pos += 5;
        }
        else {
            throw std::runtime_error("Invalid literal");
        }
        return value;
    }

    JsonValue parseNull() {
        if (m_text.compare(m_pos, 4, "null") != 0) throw std::runtime_error("Invalid literal");
        m_pos += 4;
        return JsonValue{}; // default-constructed is JsonType::Null
    }

    JsonValue parseNumber() {
        size_t start = m_pos;
        if (peek() == '-') next();
        while (m_pos < m_text.size() && std::isdigit(static_cast<unsigned char>(m_text[m_pos]))) next();
        if (m_pos < m_text.size() && m_text[m_pos] == '.') {
            next();
            while (m_pos < m_text.size() && std::isdigit(static_cast<unsigned char>(m_text[m_pos]))) next();
        }
        if (m_pos < m_text.size() && (m_text[m_pos] == 'e' || m_text[m_pos] == 'E')) {
            next();
            if (m_pos < m_text.size() && (m_text[m_pos] == '+' || m_text[m_pos] == '-')) next();
            while (m_pos < m_text.size() && std::isdigit(static_cast<unsigned char>(m_text[m_pos]))) next();
        }
        JsonValue value;
        value.type = JsonType::Number;
        value.numberValue = std::stod(m_text.substr(start, m_pos - start));
        return value;
    }
};

JsonValue parseJson(const std::string& text) {
    JsonParser parser(text);
    return parser.parse();
}
