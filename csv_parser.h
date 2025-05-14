#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <cctype>

// Error codes for CSV parsing
namespace ErrorCodes {
    constexpr const char* CSV_SECTION_NOT_FOUND = "E100: L200 - CSV section not found";
    constexpr const char* INVALID_CSV_FORMAT = "E101: L205 - Invalid CSV format";
    constexpr const char* INVALID_FIELD_FORMAT = "E102: L210 - Invalid field format";
    constexpr const char* INVALID_SIZE_VALUE = "E103: L220 - Invalid size value";
}

class CsvConfig {
public:
    struct Field {
        std::string name;
        int size;
    };

    void parse(const std::string& csvContent) {
        std::istringstream stream(csvContent);
        std::string line;
        
        fields.clear();
        
        while (std::getline(stream, line)) {
            trim(line);
            if (line.empty() || line[0] == '#') {
                continue; // Skip empty lines and comments
            }
            parseLine(line);
        }
        
        if (fields.empty()) {
            throw std::runtime_error(ErrorCodes::CSV_SECTION_NOT_FOUND);
        }
    }

    const std::vector<Field>& getFields() const { 
        return fields; 
    }
    
private:
    std::vector<Field> fields;
    
    void parseLine(const std::string& line) {
        std::istringstream lineStream(line);
        std::string fieldName, sizeStr;
        
        if (!std::getline(lineStream, fieldName, ',')) {
            throw std::runtime_error(ErrorCodes::INVALID_FIELD_FORMAT);
        }
        
        trim(fieldName);
        if (fieldName.empty()) {
            throw std::runtime_error(ErrorCodes::INVALID_FIELD_FORMAT);
        }
        
        if (!std::getline(lineStream, sizeStr, ',')) {
            throw std::runtime_error(ErrorCodes::INVALID_FIELD_FORMAT);
        }
        
        trim(sizeStr);
        try {
            int size = std::stoi(sizeStr);
            if (size <= 0) {
                throw std::runtime_error(ErrorCodes::INVALID_SIZE_VALUE);
            }
            fields.push_back({fieldName, size});
        } catch (const std::exception&) {
            throw std::runtime_error(ErrorCodes::INVALID_SIZE_VALUE);
        }
    }
    
    static void trim(std::string& str) {
        // Trim leading whitespace
        str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        
        // Trim trailing whitespace
        str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), str.end());
    }
};
