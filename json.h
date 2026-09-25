#pragma once
// Minimal JSON parser using only the C++ standard library.
// Supports the subset of JSON needed here: object, array, string, number, bool, null.

#include <string>
#include <vector>
#include <map>

enum class JsonType { Null, Bool, Number, String, Array, Object };

struct JsonValue {
    JsonType type = JsonType::Null;
    bool boolValue = false;
    double numberValue = 0.0;
    std::string stringValue;
    std::vector<JsonValue> arrayValue;
    std::map<std::string, JsonValue> objectValue;

    bool isNull() const { return type == JsonType::Null; }

    const JsonValue* find(const std::string& key) const;
};

// Throws std::runtime_error on malformed input.
JsonValue parseJson(const std::string& text);
