#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <cctype>

// Error codes for CSV parsing with line numbers
const char* ERRORS[] = {
    "E100: L200 - No se encontró la sección campos en el CSV",
    "E101: L205 - Formato CSV inválido",
    "E102: L210 - Formato de campo CSV inválido",
    "E103: L220 - Valor de tamaño no válido",
    "E104: L230 - Error al procesar el archivo",
    "E105: L240 - No se pudo abrir el archivo",
    "E106: L250 - Error al obtener el tamaño del archivo",
    "E107: L260 - Error al leer el archivo",
    "E108: L270 - Error al crear el archivo de salida",
    "E109: L280 - Error al escribir en el archivo",
    "E200: L300 - Error al seleccionar archivo CSV",
    "E201: L310 - Error al seleccionar archivo de entrada",
    "E202: L320 - Error al procesar archivo de entrada",
    "E203: L330 - Error al procesar archivo de salida",
    "E204: L340 - Error en el formato del separador",
    "E205: L350 - Error al crear la ventana",
    "E206: L360 - Error al inicializar controles",
    "E207: L370 - Error al mostrar diálogo de archivo",
    "E208: L380 - Error al actualizar el estado",
    "E209: L390 - Error en la interfaz de usuario",
    "E300: L400 - Error en la lectura de archivo",
    "E301: L410 - Error en la escritura de archivo",
    "E302: L420 - Error en el procesamiento de datos",
    "E303: L430 - Error en la validación de datos",
    "E304: L440 - Error en la inicialización del sistema",
    "E305: L450 - Error en la configuración del sistema",
    "E306: L460 - Error en la ejecución del programa",
    "E307: L470 - Error en la finalización del programa",
    "E308: L480 - Error en la limpieza de recursos",
    "E309: L490 - Error en la liberación de memoria"
};

using namespace std;

// Window dimensions
const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 400;

// Control dimensions and spacing
const int CONTROL_PADDING = 10;
const int LABEL_WIDTH = 100;
const int CONTROL_WIDTH = 400;
const int BUTTON_WIDTH = 100;

#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "comctl32.lib")

// CSV parser class
class csv_config {
public:
    std::vector<std::pair<std::string, int>> fields;
    
    void parse(const std::string& csv_str) {
        std::istringstream csvStream(csv_str);
        std::string line;
        
        while (std::getline(csvStream, line)) {
            if (line.empty()) continue;
            
            std::istringstream lineStream(line);
            std::string fieldName;
            std::string sizeStr;
            
            // Get field name
            if (!std::getline(lineStream, fieldName, ',')) {
                throw runtime_error(string(ERRORS[2]) + " en parse(): Formato de línea inválido");
            }
            
            // Get size
            if (!std::getline(lineStream, sizeStr)) {
                throw runtime_error(string(ERRORS[2]) + " en parse(): Tamaño faltante");
            }
            
            int size = std::stoi(sizeStr);
            if (size <= 0) {
                throw runtime_error(string(ERRORS[3]) + " en parse(): Tamaño inválido");
            }
            
            fields.push_back({fieldName, size});
        }
    }

    int getFieldSize(const std::string& fieldName) const {
        for (const auto& field : fields) {
            if (field.first == fieldName) {
                return field.second;
            }
        }
        throw runtime_error(string(ERRORS[2]) + " en getFieldSize(): Campo no encontrado");
    }

    const std::vector<std::pair<std::string, int>>& getFields() const {
        return fields;
    }
};

// Windows API function to read file content
string readFile(const string& filePath) {
    HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        throw runtime_error(string(ERRORS[5]) + " - " + to_string(error));
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        DWORD error = GetLastError();
        CloseHandle(hFile);
        throw runtime_error(string(ERRORS[6]) + " - " + to_string(error));
    }

    string content(fileSize, '\0');
    DWORD bytesRead;
    if (!ReadFile(hFile, &content[0], fileSize, &bytesRead, NULL)) {
        DWORD error = GetLastError();
        CloseHandle(hFile);
        throw runtime_error(string(ERRORS[7]) + " - " + to_string(error));
    }

    CloseHandle(hFile);
    return content;
}

// Windows API function to write to file
void writeFile(const string& filePath, const string& content) {
    HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        throw runtime_error(string(ERRORS[8]) + " - " + to_string(error));
    }

    DWORD bytesWritten;
    if (!WriteFile(hFile, content.c_str(), content.size(), &bytesWritten, NULL)) {
        DWORD error = GetLastError();
        CloseHandle(hFile);
        throw runtime_error(string(ERRORS[9]) + " - " + to_string(error));
    }

    CloseHandle(hFile);
}

// File processing function
string procesarArchivo(const string& csvFile, const string& inputFile, const string& separador) {
    try {
        // Read CSV configuration
        string csvContent = readFile(csvFile);
        
        // Parse CSV
        csv_config config;
        config.parse(csvContent);
        
        // Read input file
        string inputContent = readFile(inputFile);
        
        // Process file
        stringstream result;
        istringstream lineStream(inputContent);
        string line;
        
        while (getline(lineStream, line)) {
            if (line.empty()) continue;
            
            string processedLine;
            const auto& fields = config.getFields();
            
            // Process each field
            for (const auto& field : fields) {
                const std::string& fieldName = field.first;
                int size = field.second;
                
                if (size > line.length()) {
                    throw runtime_error(string(ERRORS[4]) + " - Tamaño de campo excede el tamaño de la línea");
                }
                
                // Extract field value
                std::string fieldValue = line.substr(0, size);
                line = line.substr(size);
                
                // Add to result with separator
                if (!processedLine.empty()) {
                    processedLine += separador;
                }
                processedLine += fieldValue;
            }
            
            result << processedLine << "\n";
        }
        
        return result.str();
    } catch (const exception& e) {
        throw runtime_error(string(ERRORS[5]) + " - " + e.what());
    }
}

// File dialog class
class FileDialog {
public:
    FileDialog(HWND hwndOwner, const std::string& filter, const std::string& title)
        : hwndOwner(hwndOwner), filter(filter), title(title) {
        buffer = new char[1024];
    }

    ~FileDialog() {
        delete[] buffer;
    }

    std::string ShowDialog() {
        OPENFILENAME ofn = {0};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwndOwner;
        ofn.lpstrFilter = filter.c_str();
        ofn.nFilterIndex = 1;
        ofn.lpstrFile = buffer;
        ofn.nMaxFile = 1024;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        ofn.lpstrTitle = title.c_str();

        if (GetOpenFileName(&ofn)) {
            return std::string(buffer);
        }
        return "";
    }

private:
    HWND hwndOwner;
    std::string filter;
    std::string title;
    char* buffer;
};

// Status window class
class StatusWindow {
public:
    StatusWindow(HWND hwnd) : hwnd(hwnd) {}
    void SetStatus(const std::string& message) {
        SetWindowText(hwnd, message.c_str());
    }

private:
    HWND hwnd;
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

    hwnd = CreateWindowEx(
        0, "FormatoProcessorClass", "Procesador de Formato",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, this
    );

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
    const int VERTICAL_SPACING = 30;
    const int LABEL_HEIGHT = 25;
    const int EDIT_HEIGHT = 25;
    const int BUTTON_HEIGHT = 25;
    const int STATUS_HEIGHT = 30;

    int y = CONTROL_PADDING;
    CreateWindowEx(
        0, "STATIC", "Open CSV:",
        WS_CHILD | WS_VISIBLE,
        CONTROL_PADDING, y,
        LABEL_WIDTH, LABEL_HEIGHT,
        hwnd, NULL, NULL, NULL
    );

    hwndCsvFile = CreateWindowEx(
        0, "EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        CONTROL_PADDING + LABEL_WIDTH + 10, y,
        CONTROL_WIDTH, EDIT_HEIGHT,
        hwnd, NULL, NULL, NULL
    );

    CreateWindowEx(
        0, "BUTTON", "Open CSV",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CONTROL_PADDING + LABEL_WIDTH + CONTROL_WIDTH + 20, y,
        BUTTON_WIDTH, BUTTON_HEIGHT,
        hwnd, (HMENU)2, NULL, NULL
    );

    y += VERTICAL_SPACING;

    CreateWindowEx(
        0, "STATIC", "Archivo de entrada:",
        WS_CHILD | WS_VISIBLE,
        CONTROL_PADDING, y,
        LABEL_WIDTH, LABEL_HEIGHT,
        hwnd, NULL, NULL, NULL
    );

    hwndInputFile = CreateWindowEx(
        0, "EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        CONTROL_PADDING + LABEL_WIDTH + 10, y,
        CONTROL_WIDTH, EDIT_HEIGHT,
        hwnd, NULL, NULL, NULL
    );

    CreateWindowEx(
        0, "BUTTON", "Open Input File",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CONTROL_PADDING + LABEL_WIDTH + CONTROL_WIDTH + 20, y,
        BUTTON_WIDTH, BUTTON_HEIGHT,
        hwnd, (HMENU)3, NULL, NULL
    );

    y += VERTICAL_SPACING;

    CreateWindowEx(
        0, "STATIC", "Separador:",
        WS_CHILD | WS_VISIBLE,
        CONTROL_PADDING, y,
        LABEL_WIDTH, LABEL_HEIGHT,
        hwnd, NULL, NULL, NULL
    );

    hwndSeparator = CreateWindowEx(
        0, "EDIT", ",",
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        CONTROL_PADDING + LABEL_WIDTH + 10, y,
        CONTROL_WIDTH, EDIT_HEIGHT,
        hwnd, NULL, NULL, NULL
    );

    y += VERTICAL_SPACING;

    hwndProcessButton = CreateWindowEx(
        0, "BUTTON", "Procesar",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CONTROL_PADDING + LABEL_WIDTH + 10, y,
        CONTROL_WIDTH / 2 - 10, BUTTON_HEIGHT,
        hwnd, (HMENU)1, NULL, NULL
    );

    CreateWindowEx(
        0, "BUTTON", "Cerrar",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CONTROL_PADDING + LABEL_WIDTH + CONTROL_WIDTH / 2 + 10, y,
        CONTROL_WIDTH / 2 - 10, BUTTON_HEIGHT,
        hwnd, (HMENU)4, NULL, NULL
    );

    y += VERTICAL_SPACING;

    hwndStatus = CreateWindowEx(
        0, "STATIC", "",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        CONTROL_PADDING, y,
        WINDOW_WIDTH - 2 * CONTROL_PADDING, STATUS_HEIGHT,
        hwnd, NULL, NULL, NULL
    );
}



void MainWindow::HandleCommand(WPARAM wParam) {
    switch (LOWORD(wParam)) {
        case 1:  // Process button
            HandleProcess();
            break;
        case 2:  // CSV file button
            HandleCsvFileSelect();
            break;
        case 3:  // Input file button
            HandleInputFileSelect();
            break;
        case 4:  // Close button
            PostQuitMessage(0);
            break;
    }
}

void MainWindow::HandleCsvFileSelect() {
    try {
        OPENFILENAME ofn = {0};
        char filePath[MAX_PATH] = {0};

        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFile = filePath;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = "CSV Files (*.csv)\0*.csv\0All Files (*.*)\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        ofn.lpstrTitle = "Seleccionar archivo CSV";

        if (GetOpenFileName(&ofn)) {
            SetWindowText(hwndCsvFile, ofn.lpstrFile);
        }
    } catch (const std::exception& e) {
        MessageBox(hwnd, e.what(), "Error", MB_ICONERROR);
    }
}

void MainWindow::HandleInputFileSelect() {
    try {
        OPENFILENAME ofn = {0};
        char filePath[MAX_PATH] = {0};

        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFile = filePath;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        ofn.lpstrTitle = "Seleccionar archivo de entrada";

        if (GetOpenFileName(&ofn)) {
            SetWindowText(hwndInputFile, ofn.lpstrFile);
        }
    } catch (const std::exception& e) {
        MessageBox(hwnd, e.what(), "Error", MB_ICONERROR);
    }
}

void MainWindow::HandleProcess() {
    char csvPath[MAX_PATH];
    GetWindowText(hwndCsvFile, csvPath, MAX_PATH);
    if (csvPath[0] == '\0') {
        MessageBox(hwnd, "Por favor seleccione un archivo CSV", "Error", MB_ICONERROR);
        return;
    }

    char inputPath[MAX_PATH];
    GetWindowText(hwndInputFile, inputPath, MAX_PATH);
    if (inputPath[0] == '\0') {
        MessageBox(hwnd, "Por favor seleccione un archivo de entrada", "Error", MB_ICONERROR);
        return;
    }

    char separator[2];
    GetWindowText(hwndSeparator, separator, 2);
    if (separator[0] == '\0') {
        MessageBox(hwnd, "Por favor ingrese un separador", "Error", MB_ICONERROR);
        return;
    }

    try {
        // Process the file
        string result = procesarArchivo(csvPath, inputPath, separator);
        
        // Get output file path (add _processed to input file name)
        string outputPath = inputPath;
        size_t lastDot = outputPath.find_last_of('.');
        if (lastDot != string::npos) {
            outputPath.insert(lastDot, "_processed");
        } else {
            outputPath += "_processed";
        }
        
        // Save processed file
        writeFile(outputPath, result);
        
        MessageBox(hwnd, ("Archivo procesado y guardado como: " + outputPath).c_str(), "Éxito", MB_ICONINFORMATION);
    } catch (const exception& e) {
        MessageBox(hwnd, e.what(), "Error", MB_ICONERROR);
    }
}

LRESULT CALLBACK MainWindow::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis = (MainWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (msg) {
        case WM_CREATE: {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            pThis = (MainWindow*)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
            pThis->hwnd = hwnd;
            break;
        }
        case WM_COMMAND:
            if (pThis) {
                pThis->HandleCommand(wParam);
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Initialize common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);

    MainWindow app(hInstance);
    app.Create();
    app.Show();

    // Message loop
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
