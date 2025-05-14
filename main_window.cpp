#include "main_window.h"
#include "csv_parser.h"
#include "file_utils.h"
#include <commctrl.h>
#include <commdlg.h>
#include <sstream>

// FileDialog implementation
class FileDialog {
public:
    FileDialog(HWND owner, const std::wstring& title, const std::wstring& filter, bool save = false)
        : owner(owner), title(title), filter(filter), isSave(save) {}
    
    std::string show() {
        wchar_t buffer[MAX_PATH] = {0};
        
        OPENFILENAMEW ofn = {0};
        ofn.lStructSize = sizeof(OPENFILENAMEW);
        ofn.hwndOwner = owner;
        ofn.lpstrFilter = filter.c_str();
        ofn.lpstrFile = buffer;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrTitle = title.c_str();
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        
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
    std::wstring filter;
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
    csvFileDialog = std::make_unique<FileDialog>(
        hwnd,
        L"Select CSV Configuration File",
        L"CSV Files (*.csv)\0*.csv\0All Files (*.*)\0*.*\0"
    );
    
    inputFileDialog = std::make_unique<FileDialog>(
        hwnd,
        L"Select Input File",
        L"All Files (*.*)\0*.*\0"
    );
    
    // Create CSV config
    csvConfig = std::make_unique<CsvConfig>();
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
    
    // CSV File Selection
    CreateWindowW(L"STATIC", L"CSV Configuration:", 
                 WS_VISIBLE | WS_CHILD,
                 MARGIN, y, LABEL_WIDTH, CONTROL_HEIGHT,
                 hwnd, NULL, hInstance, NULL);
    
    hwndCsvFile = CreateWindowW(L"EDIT", L"",
                              WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY,
                              MARGIN + LABEL_WIDTH, y, 300, CONTROL_HEIGHT,
                              hwnd, (HMENU)IDC_CSV_FILE, hInstance, NULL);
    
    CreateWindowW(L"BUTTON", L"Browse...",
                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 MARGIN + LABEL_WIDTH + 310, y, BUTTON_WIDTH, CONTROL_HEIGHT,
                 hwnd, (HMENU)IDC_CSV_FILE, hInstance, NULL);
    
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
        MARGIN, y, BUTTON_WIDTH, CONTROL_HEIGHT * 2,
        hwnd, (HMENU)IDC_PROCESS_BUTTON, hInstance, NULL
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
        case IDC_CSV_FILE:
            handleCsvFileSelect();
            break;
            
        case IDC_INPUT_FILE:
            handleInputFileSelect();
            break;
            
        case IDC_PROCESS_BUTTON: {
            // Disable the process button to prevent multiple clicks
            EnableWindow(hwndProcessButton, FALSE);
            
            // Process the file
            handleProcess();
            
            // Re-enable the button
            EnableWindow(hwndProcessButton, TRUE);
            break;
        }
    }
}

void MainWindow::handleCsvFileSelect() {
    try {
        std::string filePath = csvFileDialog->show();
        if (!filePath.empty()) {
            currentCsvPath = filePath;
            SetWindowTextA(GetDlgItem(hwnd, IDC_CSV_FILE), filePath.c_str());
            updateStatus("CSV file selected");
        }
    } catch (const std::exception& e) {
        MessageBoxA(hwnd, e.what(), "Error", MB_ICONERROR);
        updateStatus("Error selecting CSV file");
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
        MessageBoxA(hwnd, e.what(), "Error", MB_ICONERROR);
        updateStatus("Error selecting input file");
    }
}

void MainWindow::handleProcess() {
    // Check if both files are selected
    if (currentCsvPath.empty() || currentInputPath.empty()) {
        MessageBoxA(hwnd, "Please select both CSV and input files", "Error", MB_ICONWARNING);
        return;
    }
    
    // Get separator (default to comma if empty)
    wchar_t separator[2] = {0};
    GetWindowTextW(hwndSeparator, separator, 2);
    if (separator[0] == L'\0') {
        separator[0] = L',';  // Default separator
    }
    
    // Convert wide char to UTF-8
    char separator_utf8[8] = {0};
    WideCharToMultiByte(CP_UTF8, 0, separator, 1, separator_utf8, sizeof(separator_utf8), NULL, NULL);
    
    // Check if files exist
    if (!FileUtils::fileExists(currentCsvPath) || !FileUtils::fileExists(currentInputPath)) {
        MessageBoxA(hwnd, "One or both of the selected files do not exist", "Error", MB_ICONERROR);
        return;
    }
    
    // Disable UI during processing
    EnableWindow(hwndCsvFile, FALSE);
    EnableWindow(hwndInputFile, FALSE);
    
    try {
        updateStatus("Reading CSV configuration...");
        
        // Read and parse CSV
        std::string csvContent = FileUtils::readFile(currentCsvPath);
        if (csvContent.empty()) {
            throw std::runtime_error("CSV file is empty");
        }
        
        csvConfig->parse(csvContent);
        
        updateStatus("Reading input file...");
        
        // Read input file
        std::string inputContent = FileUtils::readFile(currentInputPath);
        if (inputContent.empty()) {
            throw std::runtime_error("Input file is empty");
        }
        
        updateStatus("Processing...");
        
        // Process the file with the specified separator
        std::string result = FileUtils::procesarArchivo(currentCsvPath, currentInputPath, separator_utf8);
        
        // Generate output path (add _processed before extension)
        std::string outputPath = currentInputPath;
        size_t lastDot = outputPath.find_last_of('.');
        if (lastDot != std::string::npos) {
            outputPath.insert(lastDot, "_processed");
        } else {
            outputPath += "_processed";
        }
        
        // Save output file
        FileUtils::writeFile(outputPath, result);
        
        updateStatus("Processing complete");
        
        // Show success message
        std::string successMsg = "File processed successfully!\n\n";
        successMsg += "Output saved to:\n" + outputPath;
        
        MessageBoxA(hwnd, successMsg.c_str(), "Success", MB_ICONINFORMATION);
        
    } catch (const std::exception& e) {
        // Show error message
        std::string error = "An error occurred during processing:\n\n";
        error += e.what();
        
        MessageBoxA(hwnd, error.c_str(), "Error", MB_ICONERROR);
        updateStatus("Processing failed");
    }
    
    // Re-enable UI
    EnableWindow(hwndCsvFile, TRUE);
    EnableWindow(hwndInputFile, TRUE);
}

void MainWindow::updateStatus(const std::string& message) {
    SetWindowTextA(hwndStatus, message.c_str());
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
