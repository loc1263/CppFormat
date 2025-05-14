#pragma once

#include <windows.h>
#include <string>
#include <memory>

class FileDialog;
class CsvConfig;

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
    void updateStatus(const std::string& message);
    
    HINSTANCE hInstance;
    HWND hwnd = nullptr;
    HWND hwndCsvFile = nullptr;
    HWND hwndInputFile = nullptr;
    HWND hwndSeparator = nullptr;  // Added this line
    HWND hwndProcessButton = nullptr;
    HWND hwndStatus = nullptr;
    
    std::unique_ptr<FileDialog> csvFileDialog;
    std::unique_ptr<FileDialog> inputFileDialog;
    std::unique_ptr<CsvConfig> csvConfig;
    
    std::string currentCsvPath;
    std::string currentInputPath;
    
    // Constants
    static constexpr const wchar_t* WINDOW_CLASS = L"FormatoProcessorClass";
    static constexpr const wchar_t* WINDOW_TITLE = L"Formato Processor";
    static constexpr int WINDOW_WIDTH = 650;
    static constexpr int WINDOW_HEIGHT = 450;
    
    // Control IDs
    enum ControlIds {
        IDC_CSV_FILE = 1001,
        IDC_INPUT_FILE,
        IDC_PROCESS_BUTTON
    };
};
