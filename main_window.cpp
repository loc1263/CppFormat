#include "main_window.h"
#include "json_parser.h"
#include "file_utils.h"
#include <commctrl.h>
#include <commdlg.h>
#include <sstream>

// FileDialog implementation
class FileDialog {
public:
    FileDialog(HWND owner, const std::wstring& title, const std::wstring& filter, bool save = false)
        : owner(owner), title(title), isSave(save) {
        // Convert the filter string to the correct format
        // The format is: "Display Text\0*.ext\0All Files\0*.*\0\0"
        // We need to replace '\0' with actual null terminators
        for (size_t i = 0; i < filter.size(); ++i) {
            if (filter[i] == L'\\' && i + 1 < filter.size() && filter[i+1] == L'0') {
                filterBuffer.push_back(L'\0');
                ++i; // Skip the '0'
            } else {
                filterBuffer.push_back(filter[i]);
            }
        }
        // Ensure double null termination
        filterBuffer.push_back(L'\0');
    }
    
    std::string show() {
        wchar_t buffer[MAX_PATH] = {0};
        
        OPENFILENAMEW ofn = {0};
        ofn.lStructSize = sizeof(OPENFILENAMEW);
        ofn.hwndOwner = owner;
        ofn.lpstrFilter = filterBuffer.data();
        ofn.lpstrFile = buffer;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrTitle = title.c_str();
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER;
        
        BOOL result = isSave ? 
            GetSaveFileNameW(&ofn) : 
            GetOpenFileNameW(&ofn);
            
        if (!result) {
            return "";
        }
        
        // Convert wide string to UTF-8
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, NULL, 0, NULL, NULL);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, buffer, -1, &strTo[0], size_needed, NULL, NULL);
        strTo.pop_back(); // Remove null terminator
        
        return strTo;
    }
    
private:
    HWND owner;
    std::wstring title;
    std::wstring filterStr;  // Original filter string
    std::vector<wchar_t> filterBuffer;  // Properly formatted filter string
    bool isSave;
};

// MainWindow implementation
MainWindow::MainWindow(HINSTANCE hInstance) : hInstance(hInstance) {
    // Initialize common controls
    INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX) };
    icex.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);
    
    // Register window class
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = windowProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = WINDOW_CLASS;
    RegisterClassExW(&wc);
}

MainWindow::~MainWindow() {
    if (hwnd) {
        DestroyWindow(hwnd);
    }
    UnregisterClassW(WINDOW_CLASS, hInstance);
}

void MainWindow::create() {
    // Create main window
    hwnd = CreateWindowExW(
        0,
        WINDOW_CLASS,
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, this
    );
    
    if (!hwnd) {
        throw std::runtime_error("Failed to create main window");
    }
    
    // Initialize controls
    initializeControls();
    
    // Create file dialogs
    // Note: The filter string uses \0 as a separator
    // Format: "Display Text\0*.ext\0All Files\0*.*\0\0"
    // We use \\0 in the string to represent a single null character
    jsonFileDialog = std::make_unique<FileDialog>(
        hwnd,
        L"Select JSON Configuration File",
        L"JSON Files (*.json)\\0*.json\\0All Files (*.*)\\0*.*\\0"
    );
    
    inputFileDialog = std::make_unique<FileDialog>(
        hwnd,
        L"Select Input File",
        L"Text Files (*.txt;*.json)\\0*.txt;*.json\\0All Files (*.*)\\0*.*\\0"
    );
}

void MainWindow::show() {
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}

void MainWindow::initializeControls() {
    // Create controls with modern spacing
    const int MARGIN = 10;
    const int CONTROL_HEIGHT = 24;
    const int BUTTON_WIDTH = 100;
    const int LABEL_WIDTH = 120;
    
    int y = MARGIN;
    
    // JSON File Selection
    CreateWindowW(L"STATIC", L"JSON Configuration:",
                 WS_VISIBLE | WS_CHILD,
                 MARGIN, y, LABEL_WIDTH, CONTROL_HEIGHT,
                 hwnd, NULL, hInstance, NULL);
    
    hwndJsonFile = CreateWindowW(L"EDIT", L"",
                              WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY,
                              MARGIN + LABEL_WIDTH, y, 300, CONTROL_HEIGHT,
                              hwnd, (HMENU)IDC_JSON_FILE, hInstance, NULL);
    
    CreateWindowW(L"BUTTON", L"Browse...",
                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 MARGIN + LABEL_WIDTH + 310, y, BUTTON_WIDTH, CONTROL_HEIGHT,
                 hwnd, (HMENU)IDC_JSON_FILE, hInstance, NULL);
    
    y += CONTROL_HEIGHT + MARGIN;
    
    // Input File Selection
    CreateWindowW(L"STATIC", L"Input File:",
                 WS_VISIBLE | WS_CHILD,
                 MARGIN, y, LABEL_WIDTH, CONTROL_HEIGHT,
                 hwnd, NULL, hInstance, NULL);
    
    hwndInputFile = CreateWindowW(L"EDIT", L"",
                                WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY,
                                MARGIN + LABEL_WIDTH, y, 300, CONTROL_HEIGHT,
                                hwnd, (HMENU)IDC_INPUT_FILE, hInstance, NULL);
    
    CreateWindowW(L"BUTTON", L"Browse...",
                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 MARGIN + LABEL_WIDTH + 310, y, BUTTON_WIDTH, CONTROL_HEIGHT,
                 hwnd, (HMENU)IDC_INPUT_FILE, hInstance, NULL);
    
    y += CONTROL_HEIGHT + MARGIN;
    
    // Separator
    CreateWindowW(L"STATIC", L"Separator:",
                 WS_VISIBLE | WS_CHILD,
                 MARGIN, y, LABEL_WIDTH, CONTROL_HEIGHT,
                 hwnd, NULL, hInstance, NULL);
    
    hwndSeparator = CreateWindowW(L"EDIT", L",",
                                 WS_VISIBLE | WS_CHILD | WS_BORDER,
                                 MARGIN + LABEL_WIDTH, y, 50, CONTROL_HEIGHT,
                                 hwnd, NULL, hInstance, NULL);
    
    y += CONTROL_HEIGHT + MARGIN * 2;
    
    // Process Button
    hwndProcessButton = CreateWindowW(
        L"BUTTON", L"Process",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        MARGIN, y, BUTTON_WIDTH - 5, CONTROL_HEIGHT * 2,  // Reduced width to fit both buttons
        hwnd, (HMENU)IDC_PROCESS_BUTTON, hInstance, NULL
    );
    
    // Close Button - positioned next to the Process button
    hwndCloseButton = CreateWindowW(
        L"BUTTON", L"Close",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        MARGIN + BUTTON_WIDTH + 5, y, BUTTON_WIDTH - 5, CONTROL_HEIGHT * 2,  // Same size as Process button
        hwnd, (HMENU)IDC_CLOSE_BUTTON, hInstance, NULL
    );
    
    // Status Bar
    hwndStatus = CreateWindowExW(
        0, STATUSCLASSNAMEW, NULL,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,
        hwnd, (HMENU)1000, hInstance, NULL
    );
    
    // Initial status
    updateStatus("Ready");
}

void MainWindow::handleCommand(WPARAM wParam) {
    // Only process button clicks (BN_CLICKED notification)
    if (HIWORD(wParam) != BN_CLICKED) {
        return;
    }
    
    // Get the control ID
    int controlId = LOWORD(wParam);
    
    // Handle the button click
    switch (controlId) {
        case IDC_JSON_FILE:
            handleCsvFileSelect();
            break;
            
        case IDC_INPUT_FILE:
            handleInputFileSelect();
            break;
            
        case IDC_PROCESS_BUTTON:
            handleProcess();
            break;
            
        case IDC_CLOSE_BUTTON:
            handleClose();
            break;
    }
}

void MainWindow::handleCsvFileSelect() {
    try {
        std::string filePath = jsonFileDialog->show();
        if (!filePath.empty()) {
            currentJsonPath = filePath;
            SetWindowTextA(GetDlgItem(hwnd, IDC_JSON_FILE), filePath.c_str());
            updateStatus("Configuration file selected");
        } else {
            DWORD error = CommDlgExtendedError();
            if (error) {
                std::stringstream ss;
                ss << "Error selecting file: " << error;
                updateStatus(ss.str());
            }
        }
    } catch (const std::exception& e) {
        updateStatus(std::string("Error: ") + e.what());
    }
}

void MainWindow::handleInputFileSelect() {
    try {
        std::string filePath = inputFileDialog->show();
        if (!filePath.empty()) {
            currentInputPath = filePath;
            SetWindowTextA(GetDlgItem(hwnd, IDC_INPUT_FILE), filePath.c_str());
            updateStatus("Input file selected");
        }
    } catch (const std::exception& e) {
        updateStatus(std::string("Error: ") + e.what());
    }
}

void MainWindow::handleProcess() {
    try {
        if (currentJsonPath.empty()) {
            updateStatus("Error: No JSON configuration file selected");
            return;
        }
        
        if (currentInputPath.empty()) {
            updateStatus("Error: No input file selected");
            return;
        }
        
        // Get separator from edit control
        char separator[2] = {0};
        GetWindowTextA(hwndSeparator, separator, 2);
        std::string sepStr(separator);
        
        if (sepStr.empty()) {
            sepStr = ",";  // Default separator
        }
        
        updateStatus("Processing file...");
        
        // Process the file
        std::string result = FileUtils::procesarArchivo(
            currentJsonPath,
            currentInputPath,
            sepStr
        );
        
        // Generate output path
        std::string outputPath = currentInputPath;
        size_t dotPos = outputPath.find_last_of('.');
        if (dotPos != std::string::npos) {
            outputPath.insert(dotPos, "_processed");
        } else {
            outputPath += "_processed";
        }
        
        // Save the result
        FileUtils::writeFile(outputPath, result);
        
        std::string successMsg = "File processed successfully!\n\n"
                              "Input: " + currentInputPath + "\n"
                              "Output: " + outputPath + "\n"
                              "Records processed: " + std::to_string(std::count(result.begin(), result.end(), '\n'));
        
        updateStatus("File processed successfully: " + outputPath);
        
        // Show success message box
        MessageBoxA(hwnd, successMsg.c_str(), "Processing Complete", MB_OK | MB_ICONINFORMATION);
        
    } catch (const std::exception& e) {
        std::string errorMsg = "Error processing file:\n" + std::string(e.what());
        updateStatus(errorMsg);
        MessageBoxA(hwnd, errorMsg.c_str(), "Processing Error", MB_OK | MB_ICONERROR);
    } catch (...) {
        std::string errorMsg = "An unknown error occurred while processing the file";
        updateStatus(errorMsg);
        MessageBoxA(hwnd, errorMsg.c_str(), "Processing Error", MB_OK | MB_ICONERROR);
    }
    
    // Re-enable UI
    if (hwndJsonFile) EnableWindow(hwndJsonFile, TRUE);
    if (hwndInputFile) EnableWindow(hwndInputFile, TRUE);
    if (hwndProcessButton) EnableWindow(hwndProcessButton, TRUE);
}

void MainWindow::updateStatus(const std::string& message) {
    SetWindowTextA(hwndStatus, message.c_str());
}

void MainWindow::handleClose() {
    // Show confirmation dialog
    int result = MessageBoxW(hwnd, 
        L"Are you sure you want to exit?", 
        L"Confirm Exit", 
        MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
    
    if (result == IDYES) {
        // Close the application
        PostMessage(hwnd, WM_CLOSE, 0, 0);
    }
}

LRESULT CALLBACK MainWindow::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis = nullptr;
    
    if (msg == WM_NCCREATE) {
        pThis = static_cast<MainWindow*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        pThis->hwnd = hwnd;
    } else {
        pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }
    
    if (pThis) {
        switch (msg) {
            case WM_COMMAND:
                pThis->handleCommand(wParam);
                break;
                
            case WM_SIZE: {
                // Resize status bar
                SendMessage(pThis->hwndStatus, WM_SIZE, 0, 0);
                
                // Get status bar height
                RECT rcStatus;
                GetWindowRect(pThis->hwndStatus, &rcStatus);
                int statusHeight = rcStatus.bottom - rcStatus.top;
                
                // Resize main window content
                RECT rcClient;
                GetClientRect(hwnd, &rcClient);
                SetWindowPos(pThis->hwndStatus, NULL,
                            0, rcClient.bottom - statusHeight,
                            rcClient.right, statusHeight,
                            SWP_NOZORDER);
                break;
            }
                
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
