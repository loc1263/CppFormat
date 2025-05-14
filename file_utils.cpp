#include "file_utils.h"
#include "json_parser.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

namespace FileUtils {

std::string readFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }
    
    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

void writeFile(const std::string& filePath, const std::string& content) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to create file: " + filePath);
    }
    
    file << content;
    if (!file.good()) {
        throw std::runtime_error("Failed to write to file: " + filePath);
    }
}

std::string getFileExtension(const std::string& filePath) {
    size_t dotPos = filePath.find_last_of(".");
    if (dotPos == std::string::npos) {
        return "";
    }
    std::string ext = filePath.substr(dotPos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), 
                  [](unsigned char c){ return std::tolower(c); });
    return ext;
}

bool fileExists(const std::string& filePath) {
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(filePath.c_str());
    return (attrs != INVALID_FILE_ATTRIBUTES && 
           !(attrs & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat buffer;   
    return (stat(filePath.c_str(), &buffer) == 0 && 
           !S_ISDIR(buffer.st_mode));
#endif
}

std::string procesarArchivo(const std::string& jsonFile, 
                         const std::string& inputFile, 
                         const std::string& separator) {
    // Read JSON configuration
    std::string jsonContent = readFile(jsonFile);
    if (jsonContent.empty()) {
        throw std::runtime_error("JSON file is empty");
    }
    
    // Parse JSON configuration
    JsonConfig config;
    try {
        config.parse(jsonContent);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Error parsing JSON config: ") + e.what());
    }
    
    // Read input file
    std::string inputContent = readFile(inputFile);
    if (inputContent.empty()) {
        throw std::runtime_error("Input file is empty");
    }
    
    // Process the file line by line
    std::istringstream inputStream(inputContent);
    std::string line;
    std::string result;
    
    while (std::getline(inputStream, line)) {
        // Skip empty lines
        if (line.empty()) {
            result += "\r\n";
            continue;
        }
        
        std::string formattedLine;
        size_t currentPos = 0;
        const auto& fields = config.getFields();
        
        // Process each field according to the CSV configuration
        for (const auto& field : fields) {
            // Extract the field value from the fixed-width input
            std::string fieldValue;
            if (currentPos < line.length()) {
                size_t endPos = std::min(currentPos + field.width, line.length());
                fieldValue = line.substr(currentPos, endPos - currentPos);
                currentPos = endPos;
            }
            
            // Trim whitespace
            fieldValue.erase(0, fieldValue.find_first_not_of(" \t\n\r\f\v"));
            fieldValue.erase(fieldValue.find_last_not_of(" \t\n\r\f\v") + 1);
            
            // Format the field to the specified width
            if (fieldValue.length() > field.width) {
                // Truncate if too long
                fieldValue = fieldValue.substr(0, field.width);
            } else if (fieldValue.length() < field.width) {
                // Pad with spaces if too short
                fieldValue.append(field.width - fieldValue.length(), ' ');
            }
            
            formattedLine += fieldValue;
            
            // Add separator between fields (except after the last one)
            if (&field != &fields.back()) {
                formattedLine += separator;
            }
        }
        
        result += formattedLine + "\r\n";
    }
    
    return result;
}

} // namespace FileUtils
