#pragma once

#include <windows.h>
#include <string>
#include <memory>

class FileDialog;

namespace FileUtils {
    class JsonConfig;
}

class MainWindow {
public:
    MainWindow(HINSTANCE hInstance);
    ~MainWindow();
    
    void create();
    void show();
    void handleCommand(WPARAM wParam);
    
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
private:
    void initializeControls();
    void handleCsvFileSelect();
    void handleInputFileSelect();
    void handleProcess();
    void handleClose();
    void updateStatus(const std::string& message);
    
    HINSTANCE hInstance;
    HWND hwnd = nullptr;
    HWND hwndJsonFile = nullptr;
    HWND hwndInputFile = nullptr;
    HWND hwndSeparator = nullptr;
    HWND hwndProcessButton = nullptr;
    HWND hwndCloseButton = nullptr;
    HWND hwndStatus = nullptr;
    
    std::unique_ptr<FileDialog> jsonFileDialog;
    std::unique_ptr<FileDialog> inputFileDialog;
    std::unique_ptr<FileUtils::JsonConfig> jsonConfig;
    
    std::string currentJsonPath;
    std::string currentInputPath;
    
    // Constants
    static constexpr const wchar_t* WINDOW_CLASS = L"FormatoProcessorClass";
    static constexpr const wchar_t* WINDOW_TITLE = L"Formato Processor";
    static constexpr int WINDOW_WIDTH = 650;
    static constexpr int WINDOW_HEIGHT = 450;
    
    // Control IDs
    enum ControlIds {
        IDC_JSON_FILE = 1001,
        IDC_INPUT_FILE,
        IDC_PROCESS_BUTTON,
        IDC_CLOSE_BUTTON
    };
};
