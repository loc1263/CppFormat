#pragma once

#include <string>
#include <stdexcept>

namespace FileUtils {
    std::string readFile(const std::string& filePath);
    void writeFile(const std::string& filePath, const std::string& content);
    std::string getFileExtension(const std::string& filePath);
    bool fileExists(const std::string& filePath);
    
    // Process file with CSV configuration and separator
    std::string procesarArchivo(const std::string& csvFile, 
                              const std::string& inputFile, 
                              const std::string& separator);
}
