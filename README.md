# Fixed-Width File Processor

A Windows GUI application for processing fixed-width text files. The application parses input files based on a CSV configuration that defines field widths and outputs the data with custom field separators.

## Features

- Parse fixed-width text files with configurable field definitions
- JSON-based configuration for field widths and metadata
- Customizable output separators (comma, pipe, tab, etc.)
- Automatic field trimming and padding
- Modern Windows 10 style GUI
- Uses nlohmann/json for JSON parsing

## Configuration

### Field Definition

Create a JSON file (e.g., `formato.json`) that defines your fixed-width fields:

```json
{
  "fields": [
    {"name": "Fecha", "width": 8},
    {"name": "Tid", "width": 6},
    {"name": "Codigo_Auth", "width": 8},
    {"name": "Cod_Comercio", "width": 9},
    {"name": "Monto_Inicial", "width": 11},
    {"name": "Monto_Procesamiento", "width": 11},
    {"name": "trx", "width": 4},
    {"name": "cuotas", "width": 3},
    {"name": "fecha_proc", "width": 8},
    {"name": "Flag", "width": 1},
    {"name": "Extra", "width": 1},
    {"name": "Code", "width": 3}
  ]
}
```

### Output Formatting

- **Field Separator**: Choose any character (default: comma `,`)
- Common separators: `|` (pipe), `\t` (tab), `;` (semicolon)

## Project Structure

```
CppFormat/
├── main.cpp              # Application entry point and WinMain
├── main_window.h         # Main window class declaration
├── main_window.cpp       # Main window implementation and UI logic
├── file_utils.h          # File utility functions declaration
├── file_utils.cpp        # File I/O and processing implementation
├── csv_parser.h          # CSV configuration parser (header-only)
├── CMakeLists.txt        # CMake build configuration
└── README.md             # This documentation file
```

### File Descriptions

- **main.cpp**: Contains the Windows entry point (`WinMain`) and initializes the application.
- **main_window.h/cpp**: Implements the main application window, user interface, and event handling.
- **file_utils.h/cpp**: Provides file I/O operations and the core fixed-width file processing logic.
- **csv_parser.h**: Header-only implementation for parsing CSV configuration files.
- **CMakeLists.txt**: Configuration for building the project with CMake.

## Building from Source

### Prerequisites
- MinGW-w64 with GCC (or compatible C++17 compiler)
- Windows SDK
- CMake 3.15 or higher
- Git (for downloading nlohmann/json)

### Compilation

#### Using CMake (recommended):
```bash
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

The output executable will be named `FormatoProcessor.exe` in the `build/bin` directory.

#### Using g++ directly:
First, download and install nlohmann/json, then:
```bash
g++ -I<path_to_nlohmann_json_include> main.cpp main_window.cpp file_utils.cpp -o CppFormat.exe -mwindows -lcomctl32 -lcomdlg32 -lgdi32 -lole32 -lshell32 -loleaut32 -luuid -std=c++17
```

## Usage

1. **Launch** the application
2. **Configure**:
   - Click "Browse..." next to "CSV Configuration" to select your field definition file
   - Click "Browse..." next to "Input File" to select the fixed-width file to process
   - (Optional) Set the output separator (default is comma)
3. **Process**: Click the "Process" button
4. **Output**: The processed file will be saved with a "_processed" suffix

### Example

1. **Input File** (`input.txt`):
   ```
   20250511123456ABCD1234CM1234567890000012345000000678900000000012305022
   20250512111111EFGH5678CM9876543210000004321000000543200000000042305032
   ```

2. **CSV Configuration** (`formato.csv`):
   ```
   Fecha,8
   Tid,6
   Codigo_Auth,8
   Cod_Comercio,9
   Monto_Inicial,11
   Monto_Procesamiento,11
   trx,4
   cuotas,3
   fecha_proc,8
   Flag,1
   Extra,1
   ```

3. **Output** (with comma separator):
   ```
   20250511,123456,ABCD1234,CM12345678,90000012345,00000678900,0000,000,01230502,2
   20250512,111111,EFGH5678,CM98765432,10000004321,00000543200,0000,000,04230503,2
   ```

## Error Handling

The application provides detailed error messages for common issues:
- File not found or inaccessible
- Invalid CSV configuration format
- Field width mismatches
- Output file write errors

### Troubleshooting

- **CSV Format Errors**: Ensure your CSV file follows the `field_name,width` format
- **Field Mismatches**: Verify that the total width in your CSV matches your input file's format
- **File Access**: Check file permissions and ensure files aren't in use by other programs
- **Memory**: The application can handle large files, but very large files may require more memory

## Contributing

Contributions are welcome! Please ensure your code follows the existing style and includes appropriate documentation.

## License

This project is licensed under the MIT License - see the LICENSE file for details.
