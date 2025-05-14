#pragma once

#include <string>
#include <stdexcept>
#include "json_parser.h"

namespace FileUtils {
    std::string readFile(const std::string& filePath);
    void writeFile(const std::string& filePath, const std::string& content);
    std::string getFileExtension(const std::string& filePath);
    bool fileExists(const std::string& filePath);
    
    // Process file with JSON configuration and separator
    std::string procesarArchivo(const std::string& jsonFile, 
                              const std::string& inputFile, 
                              const std::string& separator);
}
