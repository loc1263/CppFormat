# C++ Format Processor GUI

A Windows GUI application for processing formats.

## Building

To compile the program, you need:
- MinGW-w64 (or any GCC compiler)
- Windows SDK

### Compile Command
```bash
g++ formato_processor_gui.cpp -o formato_processor_gui.exe -mwindows -lcomctl32 -lgdi32
```

## Running

After compiling, run the executable:
```bash
./formato_processor_gui.exe
```

## Note

This is a Windows-specific application that uses Windows API functions for GUI components.
