#pragma once

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace FileUtils {

class JsonConfig {
public:
    struct Field {
        std::string name;
        size_t width;
    };

    const std::vector<Field>& getFields() const { return fields; }
    
    void parse(const std::string& jsonContent) {
        try {
            json j = json::parse(jsonContent);
            
            if (!j.contains("fields") || !j["fields"].is_array()) {
                throw std::runtime_error("Invalid JSON format: missing or invalid 'fields' array");
            }
            
            fields.clear();
            for (const auto& field : j["fields"]) {
                if (!field.contains("name") || !field.contains("width")) {
                    throw std::runtime_error("Field must contain 'name' and 'width'");
                }
                
                Field f{
                    field["name"].get<std::string>(),
                    static_cast<size_t>(field["width"].get<int>())
                };
                fields.push_back(f);
            }
        } catch (const json::exception& e) {
            throw std::runtime_error(std::string("JSON parse error: ") + e.what());
        }
    }

private:
    std::vector<Field> fields;
};

} // namespace FileUtils
