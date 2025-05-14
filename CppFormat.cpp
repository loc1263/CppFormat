#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0'")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")

using namespace std;

// Error messages
const char* ERRORS[] = {
    "Error: No se encontró la sección campos en el CSV",
    "Error: Formato CSV inválido",
    "Error: Formato de campo CSV inválido",
    "Error: Valor de tamaño no válido",
    "Error: No se pudo abrir el archivo",
    "Error: Error al leer/escribir archivo"
};

// Window dimensions and controls
const int WINDOW_WIDTH = 500;
const int WINDOW_HEIGHT = 350;
const int CONTROL_PADDING = 10;
const int CONTROL_WIDTH = 300;
const int BUTTON_WIDTH = 80;
const int BUTTON_HEIGHT = 25;
const int EDIT_HEIGHT = 24;
const int LABEL_HEIGHT = 20;
const int LABEL_WIDTH = 120;
const int VERTICAL_SPACING = 8;

class CSVConfig {
    vector<pair<string, int>> fields;
    
public:
    void parse(const string& csv_str) {
        istringstream csvStream(csv_str);
        string line;
        int lineNum = 0;
        
        while (getline(csvStream, line)) {
            lineNum++;
            
            // Trim whitespace from the line
            line.erase(0, line.find_first_not_of(" \t\r\n\f\v"));
            line.erase(line.find_last_not_of(" \t\r\n\f\v") + 1);
            
            if (line.empty()) continue;
            
            // Find the first comma
            size_t commaPos = line.find(',');
            if (commaPos == string::npos) {
                throw runtime_error(string(ERRORS[1]) + " on line " + to_string(lineNum));
            }
            
            // Extract field name and size
            string fieldName = line.substr(0, commaPos);
            string sizeStr = line.substr(commaPos + 1);
            
            // Trim whitespace from field name and size
            fieldName.erase(0, fieldName.find_first_not_of(" \t"));
            fieldName.erase(fieldName.find_last_not_of(" \t") + 1);
            
            sizeStr.erase(0, sizeStr.find_first_not_of(" \t"));
            sizeStr.erase(sizeStr.find_last_not_of(" \t") + 1);
            
            if (fieldName.empty() || sizeStr.empty()) {
                throw runtime_error(string(ERRORS[1]) + " on line " + to_string(lineNum));
            }
            
            try {
                int size = stoi(sizeStr);
                if (size <= 0) throw runtime_error("");
                fields.push_back({fieldName, size});
            } catch (...) {
                throw runtime_error(string(ERRORS[3]) + " on line " + to_string(lineNum));
            }
        }
    }
    
    const vector<pair<string, int>>& getFields() const { return fields; }
};

string readFile(const string& filePath) {
    ifstream file(filePath, ios::binary | ios::ate);
    if (!file) throw runtime_error(ERRORS[4]);
    
    streamsize size = file.tellg();
    file.seekg(0, ios::beg);
    
    string content(size, '\0');
    if (!file.read(&content[0], size)) {
        throw runtime_error(ERRORS[5]);
    }
    return content;
}

void writeFile(const string& filePath, const string& content) {
    ofstream file(filePath, ios::binary);
    if (!file || !(file << content)) {
        throw runtime_error(ERRORS[5]);
    }
}

// File processing function
string processFile(const string& csvFile, const string& inputFile, const string& separator) {
    // Read and parse CSV config
    CSVConfig config;
    config.parse(readFile(csvFile));
    
    // Process input file
    string inputContent = readFile(inputFile);
    stringstream result;
    istringstream lineStream(inputContent);
    string line;
    
    while (getline(lineStream, line)) {
        if (line.empty()) continue;
        
        string processedLine;
        string remaining = line;
        
        for (const auto& [name, size] : config.getFields()) {
            int currentSize = min(size, (int)remaining.length());
            string field = remaining.substr(0, currentSize);
            remaining = remaining.substr(currentSize);
            
            // Trim whitespace
            field.erase(0, field.find_first_not_of(" \t\n\r\f\v"));
            field.erase(field.find_last_not_of(" \t\n\r\f\v") + 1);
            
            if (!processedLine.empty()) processedLine += separator;
            processedLine += field;
        }
        
        result << processedLine << "\n";
    }
    
    return result.str();
}

class FileDialog {
    HWND hwndOwner;
    string filter;
    string title;
    char buffer[1024] = {0};
    
public:
    FileDialog(HWND owner, const char* f, const string& t) 
        : hwndOwner(owner), filter(f), title(t) {}
    
    string show() {
        OPENFILENAMEA ofn = {0};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwndOwner;
        
        // Convert the filter string to the correct format
        string filterCopy = filter;
        // Replace all '|' with '\0' in the filter string
        for (size_t i = 0; i < filterCopy.size(); ++i) {
            if (filterCopy[i] == '|') filterCopy[i] = '\0';
        }
        
        ofn.lpstrFilter = filterCopy.c_str();
        ofn.nFilterIndex = 1;
        ofn.lpstrFile = buffer;
        buffer[0] = '\0';
        ofn.nMaxFile = sizeof(buffer);
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
        ofn.lpstrTitle = title.c_str();
        ofn.lpstrInitialDir = ".";
        
        return GetOpenFileNameA(&ofn) ? buffer : "";
    }
};

class StatusWindow {
    HWND hwnd;
    
public:
    StatusWindow(HWND h) : hwnd(h) {}
    void setStatus(const string& msg) { SetWindowText(hwnd, msg.c_str()); }
};

// Main window class
class MainWindow {
public:
    MainWindow(HINSTANCE hInstance);
    ~MainWindow();
    void Create();
    void Show();
    void HandleCommand(WPARAM wParam);
    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    void InitializeControls();
    void HandleCsvFileSelect();
    void HandleInputFileSelect();
    void HandleProcess();

    HINSTANCE hInstance;
    HWND hwnd;
    HWND hwndCsvFile;
    HWND hwndInputFile;
    HWND hwndSeparator;
    HWND hwndProcessButton;
    HWND hwndStatus;
    FileDialog* csvFileDialog;
    FileDialog* inputFileDialog;
};

MainWindow::MainWindow(HINSTANCE hInstance)
    : hInstance(hInstance), hwnd(NULL), hwndCsvFile(NULL), hwndInputFile(NULL),
      hwndSeparator(NULL), hwndProcessButton(NULL), hwndStatus(NULL),
      csvFileDialog(NULL), inputFileDialog(NULL) {
}

MainWindow::~MainWindow() {
    if (csvFileDialog) {
        delete csvFileDialog;
    }
    if (inputFileDialog) {
        delete inputFileDialog;
    }
    if (hwnd) {
        DestroyWindow(hwnd);
    }
}

void MainWindow::Create() {
    INITCOMMONCONTROLSEX icex = {0};
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);

    WNDCLASS wc = {0};
    wc.lpfnWndProc = MainWindow::StaticWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "FormatoProcessorClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Error al registrar la clase de ventana", "Error", MB_ICONERROR);
        return;
    }

    // Create the main window with modern style
    hwnd = CreateWindowEx(
        0,
        "FormatoProcessorClass",
        "Formato Processor",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, this
    );
    
    // Set window background color to white
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)GetStockObject(WHITE_BRUSH));

    if (!hwnd) {
        MessageBox(NULL, "Error al crear la ventana", "Error", MB_ICONERROR);
        return;
    }

    Show();
    InitializeControls();
    UpdateWindow(hwnd);
}

void MainWindow::Show() {
    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);
}

void MainWindow::InitializeControls() {
    // Using standard Windows control heights
    const int STATUS_HEIGHT = 24;  // Standard status bar height

    int y = CONTROL_PADDING;
    
    // First row: CSV file
    CreateWindowEx(
        0, "STATIC", "Open CSV:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        CONTROL_PADDING, y,
        LABEL_WIDTH, LABEL_HEIGHT,
        hwnd, NULL, NULL, NULL
    );
    y += LABEL_HEIGHT + 4;  // Add small gap between label and control

    hwndCsvFile = CreateWindowEx(
        WS_EX_CLIENTEDGE, "EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        CONTROL_PADDING, y,
        CONTROL_WIDTH, EDIT_HEIGHT,
        hwnd, NULL, NULL, NULL
    );

    CreateWindowEx(
        0, "BUTTON", "Open CSV",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        CONTROL_PADDING + CONTROL_WIDTH + 10, y,
        BUTTON_WIDTH, BUTTON_HEIGHT,
        hwnd, (HMENU)2, NULL, NULL
    );
    
    // Set font for controls
    HFONT hControlFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessage(hwndCsvFile, WM_SETFONT, (WPARAM)hControlFont, TRUE);

    y += EDIT_HEIGHT + VERTICAL_SPACING;

    // Second row: Input file
    y += EDIT_HEIGHT + VERTICAL_SPACING;
    
    CreateWindowEx(
        0, "STATIC", "Archivo de entrada:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        CONTROL_PADDING, y,
        LABEL_WIDTH, LABEL_HEIGHT,
        hwnd, NULL, NULL, NULL
    );
    y += LABEL_HEIGHT + 4;  // Add small gap between label and control

    hwndInputFile = CreateWindowEx(
        WS_EX_CLIENTEDGE, "EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        CONTROL_PADDING, y,
        CONTROL_WIDTH, EDIT_HEIGHT,
        hwnd, NULL, NULL, NULL
    );

    CreateWindowEx(
        0, "BUTTON", "Open Input File",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        CONTROL_PADDING + CONTROL_WIDTH + 10, y,
        BUTTON_WIDTH, BUTTON_HEIGHT,
        hwnd, (HMENU)3, NULL, NULL
    );
    
    // Set font for input file controls
    SendMessage(hwndInputFile, WM_SETFONT, (WPARAM)hControlFont, TRUE);
    
    y += EDIT_HEIGHT + VERTICAL_SPACING;
    
    // Separator section
    y += EDIT_HEIGHT + VERTICAL_SPACING;
    
    CreateWindowEx(
        0, "STATIC", "Separador:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        CONTROL_PADDING, y,
        LABEL_WIDTH, LABEL_HEIGHT,
        hwnd, NULL, NULL, NULL
    );
    y += LABEL_HEIGHT + 4;  // Add small gap between label and control

    hwndSeparator = CreateWindowEx(
        WS_EX_CLIENTEDGE, "EDIT", ",",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        CONTROL_PADDING, y,
        50, EDIT_HEIGHT,  // Smaller width for separator field
        hwnd, NULL, NULL, NULL
    );
    
    // Set font for separator control
    SendMessage(hwndSeparator, WM_SETFONT, (WPARAM)hControlFont, TRUE);
    
    y += EDIT_HEIGHT + VERTICAL_SPACING;

    // Process and Close buttons row
    const int BUTTON_SPACING = 20;
    const int TOTAL_BUTTONS_WIDTH = (BUTTON_WIDTH + 20) * 2 + BUTTON_SPACING;
    int buttonsStartX = (WINDOW_WIDTH - TOTAL_BUTTONS_WIDTH) / 2;
    
    // Process button
    hwndProcessButton = CreateWindowEx(
        0, "BUTTON", "PROCESAR",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,
        buttonsStartX, y,
        BUTTON_WIDTH + 20, BUTTON_HEIGHT + 8,  // Larger button
        hwnd, (HMENU)1, NULL, NULL
    );

    CreateWindowEx(
        0, "BUTTON", "CERRAR",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        buttonsStartX + BUTTON_WIDTH + 20 + BUTTON_SPACING, y,
        BUTTON_WIDTH, BUTTON_HEIGHT,
        hwnd, (HMENU)4, NULL, NULL
    );

    hwndStatus = CreateWindowEx(
        0, STATUSCLASSNAME, "",
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,
        hwnd, (HMENU)1001, hInstance, NULL
    );

    // Note: The filter string needs to be double-null terminated
    // Each pair is "display name\0pattern\0" and the list ends with "\0\0"
    // Using | as a separator that we'll convert to null characters
    csvFileDialog = new FileDialog(
        hwnd,
        "CSV Files (*.csv)|*.csv|All Files (*.*)|*.*|",
        "Select CSV File"
    );

    inputFileDialog = new FileDialog(
        hwnd,
        "Text Files (*.txt)|*.txt|All Files (*.*)|*.*|",
        "Select Input File"
    );

    StatusWindow status(hwndStatus);
    status.setStatus("Ready");

    RECT rc = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    DWORD style = (DWORD)GetWindowLongPtr(hwnd, GWL_STYLE);
    DWORD exStyle = (DWORD)GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    AdjustWindowRectEx(&rc, style, FALSE, exStyle);
    SetWindowPos(hwnd, NULL, 0, 0, rc.right-rc.left, rc.bottom-rc.top, 
                SWP_NOMOVE | SWP_NOZORDER);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    SetWindowPos(hwnd, NULL, (screenWidth - (rc.right-rc.left))/2, 
                (screenHeight - (rc.bottom-rc.top))/2, 0, 0, 
                SWP_NOSIZE | SWP_NOZORDER);
}

void MainWindow::HandleCsvFileSelect() {
    try {
        string filePath = csvFileDialog->show();
        if (!filePath.empty()) {
            SetWindowText(hwndCsvFile, filePath.c_str());
            StatusWindow(hwndStatus).setStatus("CSV file selected");
        }
    } catch (const exception& e) {
        StatusWindow(hwndStatus).setStatus("Error selecting CSV file");
        MessageBoxA(hwnd, e.what(), "Error", MB_ICONERROR);
    }
}

void MainWindow::HandleInputFileSelect() {
    try {
        string filePath = inputFileDialog->show();
        if (!filePath.empty()) {
            SetWindowText(hwndInputFile, filePath.c_str());
            StatusWindow(hwndStatus).setStatus("Input file selected");
        }
    } catch (const exception& e) {
        StatusWindow(hwndStatus).setStatus("Error selecting input file");
        MessageBoxA(hwnd, e.what(), "Error", MB_ICONERROR);
    }
}

void MainWindow::HandleCommand(WPARAM wParam) {
    switch (LOWORD(wParam)) {
        case 1: HandleProcess(); break;
        case 2: HandleCsvFileSelect(); break;
        case 3: HandleInputFileSelect(); break;
        case 4: PostQuitMessage(0); break;
    }
}

void MainWindow::HandleProcess() {
    try {
        char csvPath[MAX_PATH] = {0};
        char inputPath[MAX_PATH] = {0};
        char separator[10] = {0};
        
        GetWindowText(hwndCsvFile, csvPath, MAX_PATH);
        GetWindowText(hwndInputFile, inputPath, MAX_PATH);
        GetWindowText(hwndSeparator, separator, 10);
        
        if (!*csvPath) {
            MessageBoxA(hwnd, "Please select a CSV file", "Error", MB_ICONERROR);
            return;
        }
        
        if (!*inputPath) {
            MessageBoxA(hwnd, "Please select an input file", "Error", MB_ICONERROR);
            return;
        }
        
        if (!*separator) strcpy_s(separator, ",");
        
        try {
            // Read CSV file first to check for errors
            string csvContent = readFile(csvPath);
            if (csvContent.empty()) {
                throw runtime_error("CSV file is empty");
            }
            
            // Read input file to check for errors
            string inputContent = readFile(inputPath);
            if (inputContent.empty()) {
                throw runtime_error("Input file is empty");
            }
            
            string outputPath = string(inputPath) + ".processed";
            string result = processFile(csvPath, inputPath, separator);
            writeFile(outputPath, result);
            
            StatusWindow(hwndStatus).setStatus("File processed successfully");
            string msg = "File processed successfully.\n\nOutput: " + outputPath;
            MessageBoxA(hwnd, msg.c_str(), "Success", MB_ICONINFORMATION);
            
        } catch (const exception& e) {
            string errorMsg = string("Error processing files. ") + e.what();
            StatusWindow(hwndStatus).setStatus("Error processing file");
            MessageBoxA(hwnd, errorMsg.c_str(), "Error", MB_ICONERROR);
        }
        
    } catch (const exception& e) {
        string errorMsg = string("Unexpected error: ") + e.what();
        StatusWindow(hwndStatus).setStatus("Error processing file");
        MessageBoxA(hwnd, errorMsg.c_str(), "Error", MB_ICONERROR);
    }
}

LRESULT CALLBACK MainWindow::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static MainWindow* pThis = nullptr;
    
    if (msg == WM_CREATE) {
        pThis = (MainWindow*)((LPCREATESTRUCT)lParam)->lpCreateParams;
        return 0;
    }
    
    if (!pThis) return DefWindowProc(hwnd, msg, wParam, lParam);
    
    switch (msg) {
        case WM_COMMAND: pThis->HandleCommand(wParam); break;
        case WM_DESTROY: PostQuitMessage(0); break;
        default: return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    try {
        INITCOMMONCONTROLSEX icex = {sizeof(icex), ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES};
        InitCommonControlsEx(&icex);
        
        MainWindow mainWindow(hInstance);
        mainWindow.Create();
        
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            if (!IsDialogMessage(GetAncestor(msg.hwnd, GA_ROOT), &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        
        return (int)msg.wParam;
        
    } catch (const exception& e) {
        MessageBoxA(NULL, e.what(), "Error", MB_ICONERROR);
        return 1;
    }
}
