#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <commctrl.h>

#include "main_window.h"
#include <stdexcept>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    try {
        // Initialize COM
        if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE))) {
            MessageBoxW(NULL, L"Failed to initialize COM", L"Error", MB_ICONERROR);
            return 1;
        }
        
        // Create and show main window
        MainWindow mainWindow(hInstance);
        mainWindow.create();
        mainWindow.show();
        
        // Message loop
        MSG msg = {0};
        while (GetMessage(&msg, NULL, 0, 0) > 0) {
            if (!IsDialogMessage(GetAncestor(msg.hwnd, GA_ROOT), &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        
        // Cleanup
        CoUninitialize();
        return (int)msg.wParam;
        
    } catch (const std::exception& e) {
        MessageBoxA(NULL, e.what(), "Error", MB_ICONERROR);
        return 1;
    }
}
