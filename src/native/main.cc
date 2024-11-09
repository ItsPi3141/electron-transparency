#include "types.h"

#include <iostream>
#include <vector>

#include <node_api.h>

#include <dwmapi.h>
#include <winuser.h>

namespace electronTransparency {
    HINSTANCE dwm;
    DwmSetWinAttr SetWindowAttribute;

    struct napiArgs {
        HWND hwnd;
        int value;
    };

    void loadDwm() {
        dwm = LoadLibraryA("dwmapi.dll");
        SetWindowAttribute = (DwmSetWinAttr)GetProcAddress(dwm, "DwmSetWindowAttribute");
    }
    void unloadDwm() {
        FreeLibrary(dwm);
    }

    napiArgs parseArgs(napi_env env, napi_callback_info args) {
        // Get argc
        size_t argc;
        napi_get_cb_info(env, args, &argc, nullptr, nullptr, nullptr);

        // Get argv
        napi_value *argv = new napi_value[argc];
        napi_get_cb_info(env, args, &argc, argv, nullptr, nullptr);

        // Parse args
        napi_valuetype HWNDType;
        napi_valuetype Value;
        napi_typeof(env, argv[0], &HWNDType);
        napi_typeof(env, argv[1], &Value);

        if (HWNDType != napi_number) {
            napi_throw_error(env, nullptr, "HWND must be an integer");
            return {(HWND)(-1), -1};
        }
        if (Value != napi_number) {
            napi_throw_error(env, nullptr, "Value must be an integer");
            return {(HWND)(-1), -1};
        }

        int64_t hwndTemp;
        int32_t valueTemp;
        napi_get_value_int64(env, argv[0], &hwndTemp);
        napi_get_value_int32(env, argv[1], &valueTemp);
        HWND hwnd = (HWND)hwndTemp;
        int value = (int)valueTemp;

        return {hwnd, value};
    }

    bool isAutoTheme = false;

    // https://stackoverflow.com/questions/51334674/how-to-detect-windows-10-light-dark-mode-in-win32-application
    bool isLightTheme() {
        auto buffer = std::vector<char>(4);
        auto cbData = (DWORD)(buffer.size() * sizeof(char));
        auto res = RegGetValueW(
            HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
            L"AppsUseLightTheme",
            RRF_RT_REG_DWORD,
            nullptr,
            buffer.data(),
            &cbData);
        if (res != ERROR_SUCCESS) {
            // Default to dark theme
            return false;
        }
        // Convert bytes to int, assuming it is little-endian
        auto i = int(buffer[3] << 24 |
                     buffer[2] << 16 |
                     buffer[1] << 8 |
                     buffer[0]);

        return i == 1;
    }

    int micaType = 0x02;
    int useDarkTheme = 0x01;
    napi_value applyMica(napi_env env, napi_callback_info args) {
        napiArgs argv = parseArgs(env, args);
        HWND hwnd = argv.hwnd;
        int value = argv.value;
        if (hwnd == (HWND)-1)
            return nullptr;

        if ((value & 2) == 2) {
            // Mica
            micaType = 0x02;
        } else if ((value & 4) == 4) {
            // Tabbed mica
            micaType = 0x04;
        }

        if ((value & 8) == 8) {
            // Light theme
            useDarkTheme = 0x00;
            isAutoTheme = false;
        } else if ((value & 16) == 16) {
            // Dark theme
            useDarkTheme = 0x01;
            isAutoTheme = false;
        } else if ((value & 32) == 32) {
            // Auto theme
            useDarkTheme = isLightTheme() ? 0x00 : 0x01;
            isAutoTheme = true;
        }

        loadDwm();

        SetWindowAttribute(hwnd, DWORD(20), &useDarkTheme, sizeof(int));
        SetWindowAttribute(hwnd, DWORD(38), &micaType, sizeof(int));

        unloadDwm();

        return nullptr;
    }

    napi_value restoreControls(napi_env env, napi_callback_info args) {
        napiArgs argv = parseArgs(env, args);
        HWND hwnd = argv.hwnd;
        int value = argv.value;
        if (hwnd == (HWND)-1)
            return nullptr;

        HMENU menu = GetSystemMenu(hwnd, false);
        EnableMenuItem(menu, SC_MAXIMIZE, MF_ENABLED);
        EnableMenuItem(menu, SC_RESTORE, MF_ENABLED);
        EnableMenuItem(menu, SC_SIZE, MF_ENABLED);
        EnableMenuItem(menu, SC_MOVE, MF_ENABLED);
        SetWindowLong(hwnd, GWL_STYLE,
                      GetWindowLong(hwnd, GWL_STYLE) | WS_MAXIMIZEBOX | WS_SIZEBOX);

        return nullptr;
    }

    napi_value maximize(napi_env env, napi_callback_info args) {
        napiArgs argv = parseArgs(env, args);
        HWND hwnd = argv.hwnd;
        int value = argv.value;
        if (hwnd == (HWND)-1)
            return nullptr;

        ShowWindow(hwnd, SW_MAXIMIZE);

        return nullptr;
    }

    bool setOldWndProc = false;
    WNDPROC OldWndProc;
    HWND childHwnd;
    LRESULT CALLBACK WndProc(HWND hwnd, unsigned int msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
            case WM_NCHITTEST: {
                LRESULT hit = DefWindowProc(hwnd, msg, wParam, lParam);
                switch (hit) {
                    case HTNOWHERE:
                    case HTCLIENT:
                        return HTCAPTION;
                }
            }
            case WM_SYSCOMMAND: {
                switch (wParam) {
                    case SC_MAXIMIZE: {
                        ShowWindow(hwnd, SW_SHOWMAXIMIZED);
                    }
                }
            }
            case WM_NCLBUTTONDBLCLK: {
                switch (wParam) {
                    case HTCAPTION: {
                        ShowWindow(hwnd, SW_SHOWMAXIMIZED);
                    }
                }
            }
            case WM_WININICHANGE: {
                if (lParam == (LPARAM)("ImmersiveColorSet") && isAutoTheme) {
                    SetWindowAttribute(hwnd, DWORD(20), &useDarkTheme, sizeof(int));
                    SetWindowAttribute(hwnd, DWORD(38), &micaType, sizeof(int));
                }
            }
        }
        return CallWindowProc(OldWndProc, hwnd, msg, wParam, lParam);
    }

    napi_value removeFrame(napi_env env, napi_callback_info args) {
        napiArgs argv = parseArgs(env, args);
        HWND hwnd = argv.hwnd;
        if (hwnd == (HWND)-1)
            return nullptr;

        if (!setOldWndProc) {
            setOldWndProc = true;
            OldWndProc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_WNDPROC);
        }
        SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

        SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_CAPTION);

        return nullptr;
    }

    napi_value redrawWindow(napi_env env, napi_callback_info args) {
        napiArgs argv = parseArgs(env, args);
        HWND hwnd = argv.hwnd;
        if (hwnd == (HWND)-1)
            return nullptr;

        SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE);

        return nullptr;
    }

    napi_value init(napi_env env, napi_value exports) {
        napi_value _applyMica;
        napi_value _restoreControls;
        napi_value _maximize;
        napi_value _removeFrame;
        napi_value _redrawWindow;

        napi_create_function(env, nullptr, 0, applyMica, nullptr, &_applyMica);
        napi_set_named_property(env, exports, "applyMica", _applyMica);

        napi_create_function(env, nullptr, 0, restoreControls, nullptr, &_restoreControls);
        napi_set_named_property(env, exports, "restoreControls", _restoreControls);

        napi_create_function(env, nullptr, 0, maximize, nullptr, &_maximize);
        napi_set_named_property(env, exports, "maximize", _maximize);

        napi_create_function(env, nullptr, 0, removeFrame, nullptr, &_removeFrame);
        napi_set_named_property(env, exports, "removeFrame", _removeFrame);

        napi_create_function(env, nullptr, 0, redrawWindow, nullptr, &_redrawWindow);
        napi_set_named_property(env, exports, "redrawWindow", _redrawWindow);

        return exports;
    }

    NAPI_MODULE(NODE_GYP_MODULE_NAME, init)
}