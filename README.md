# Formato Processor GUI

A Windows GUI application for processing text files based on CSV configuration. This application allows you to format input files according to a specified CSV template, ensuring data consistency and proper formatting.

## Features

- **CSV Configuration**: Define output format using a simple CSV file
- **File Processing**: Process text files according to the specified format
- **User-Friendly Interface**: Modern Windows 10 style GUI with intuitive controls
- **Error Handling**: Comprehensive error reporting with detailed messages
- **Status Updates**: Real-time status updates during file processing

## CSV Format Specification

The CSV configuration file should define the output format with the following structure:
```
field_name1,size1
field_name2,size2
...
```

Where:
- `field_name`: Name of the field (for reference)
- `size`: Number of characters to allocate for this field

## Building

### Prerequisites
- MinGW-w64 (or any GCC compiler)
- Windows SDK
- Standard C++17 compatible compiler

### Compilation

```bash
g++ formato_processor_gui.cpp -o FormatoProcessor.exe -mwindows -lcomctl32 -lcomdlg32 -lgdi32 -static-libgcc -static-libstdc++
```

## Usage

1. Launch the application
2. Click "Seleccionar CSV" to choose your CSV configuration file
3. Click "Seleccionar Archivo" to choose the input file to process
4. Click "Procesar" to process the file
5. The processed file will be saved with a "_processed" suffix

## Error Codes

The application uses the following error code format:
- `E1XX`: File operation errors
- `E2XX`: User interface errors
- `E3XX`: System and processing errors

## Troubleshooting

- **CSV Format Errors**: Ensure your CSV file follows the specified format
- **File Access Issues**: Check file permissions and ensure files are not in use by other programs
- **Memory Errors**: The application has a file size limit (configurable in code)

## Contributing

Contributions are welcome! Please ensure your code follows the existing style and includes appropriate documentation.

## License

This project is licensed under the MIT License - see the LICENSE file for details.
