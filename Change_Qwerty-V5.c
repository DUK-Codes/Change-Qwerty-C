#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <shellapi.h>
#include <shlobj.h>

#define TRAY_ICON_ID 1
#define WM_TRAYICON (WM_USER + 1)
#define IDM_ABOUT 100
#define IDM_AUTORUN 102
#define IDM_EXIT 101

// Маппинг QWERTY -> ЙЦУКЕН
wchar_t qwerty_to_cyrillic(wchar_t c) {
    switch (c) {
        case L'q': return L'й'; case L'w': return L'ц'; case L'e': return L'у';
        case L'r': return L'к'; case L't': return L'е'; case L'y': return L'н';
        case L'u': return L'г'; case L'i': return L'ш'; case L'o': return L'щ';
        case L'p': return L'з'; case L'[': return L'х'; case L']': return L'ъ';
        case L'a': return L'ф'; case L's': return L'ы'; case L'd': return L'в';
        case L'f': return L'а'; case L'g': return L'п'; case L'h': return L'р';
        case L'j': return L'о'; case L'k': return L'л'; case L'l': return L'д';
        case L';': return L'ж'; case L'\'': return L'э'; case L'z': return L'я';
        case L'x': return L'ч'; case L'c': return L'с'; case L'v': return L'м';
        case L'b': return L'и'; case L'n': return L'т'; case L'm': return L'ь';
        case L',': return L'б'; case L'.': return L'ю'; case L'/': return L'.';
        case L'`': return L'ё';
        case L'Q': return L'Й'; case L'W': return L'Ц'; case L'E': return L'У';
        case L'R': return L'К'; case L'T': return L'Е'; case L'Y': return L'Н';
        case L'U': return L'Г'; case L'I': return L'Ш'; case L'O': return L'Щ';
        case L'P': return L'З'; case L'{': return L'Х'; case L'}': return L'Ъ';
        case L'A': return L'Ф'; case L'S': return L'Ы'; case L'D': return L'В';
        case L'F': return L'А'; case L'G': return L'П'; case L'H': return L'Р';
        case L'J': return L'О'; case L'K': return L'Л'; case L'L': return L'Д';
        case L':': return L'Ж'; case L'"': return L'Э'; case L'Z': return L'Я';
        case L'X': return L'Ч'; case L'C': return L'С'; case L'V': return L'М';
        case L'B': return L'И'; case L'N': return L'Т'; case L'M': return L'Ь';
        case L'<': return L'Б'; case L'>': return L'Ю'; case L'?': return L',';
        case L'~': return L'Ё';
        default: return c;
    }
}

// Обратный маппинг ЙЦУКЕН -> QWERTY
wchar_t cyrillic_to_qwerty(wchar_t c) {
    switch (c) {
        case L'й': return L'q'; case L'ц': return L'w'; case L'у': return L'e';
        case L'к': return L'r'; case L'е': return L't'; case L'н': return L'y';
        case L'г': return L'u'; case L'ш': return L'i'; case L'щ': return L'o';
        case L'з': return L'p'; case L'х': return L'['; case L'ъ': return L']';
        case L'ф': return L'a'; case L'ы': return L's'; case L'в': return L'd';
        case L'а': return L'f'; case L'п': return L'g'; case L'р': return L'h';
        case L'о': return L'j'; case L'л': return L'k'; case L'д': return L'l';
        case L'ж': return L';'; case L'э': return L'\''; case L'я': return L'z';
        case L'ч': return L'x'; case L'с': return L'c'; case L'м': return L'v';
        case L'и': return L'b'; case L'т': return L'n'; case L'ь': return L'm';
        case L'б': return L','; case L'ю': return L'.'; case L'.': return L'/';
        case L'ё': return L'`';
        case L'Й': return L'Q'; case L'Ц': return L'W'; case L'У': return L'E';
        case L'К': return L'R'; case L'Е': return L'T'; case L'Н': return L'Y';
        case L'Г': return L'U'; case L'Ш': return L'I'; case L'Щ': return L'O';
        case L'З': return L'P'; case L'Х': return L'{'; case L'Ъ': return L'}';
        case L'Ф': return L'A'; case L'Ы': return L'S'; case L'В': return L'D';
        case L'А': return L'F'; case L'П': return L'G'; case L'Р': return L'H';
        case L'О': return L'J'; case L'Л': return L'K'; case L'Д': return L'L';
        case L'Ж': return L':'; case L'Э': return L'"'; case L'Я': return L'Z';
        case L'Ч': return L'X'; case L'С': return L'C'; case L'М': return L'V';
        case L'И': return L'B'; case L'Т': return L'N'; case L'Ь': return L'M';
        case L'Б': return L'<'; case L'Ю': return L'>'; case L',': return L'?';
        case L'Ё': return L'~';
        default: return c;
    }
}

// Переключение раскладки строки
void switch_layout(const wchar_t* src, wchar_t* dst, int from_qwerty) {
    for (int i = 0; src[i]; i++) {
        dst[i] = from_qwerty ? qwerty_to_cyrillic(src[i]) : cyrillic_to_qwerty(src[i]);
    }
    dst[wcslen(src)] = L'\0';
}

// Получить текст из буфера обмена
int get_clipboard(wchar_t** output) {
    *output = NULL;
    if (!OpenClipboard(NULL)) return 0;
    
    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (!hData) {
        CloseClipboard();
        return 0;
    }

    wchar_t* pszText = (wchar_t*)GlobalLock(hData);
    if (!pszText) {
        CloseClipboard();
        return 0;
    }

    size_t len = wcslen(pszText) + 1;
    *output = (wchar_t*)malloc(len * sizeof(wchar_t));
    if (*output) {
        wcscpy_s(*output, len, pszText);
    }

    GlobalUnlock(hData);
    CloseClipboard();
    return (*output != NULL);
}

// Поместить текст в буфер обмена
int set_clipboard(const wchar_t* buffer) {
    if (!OpenClipboard(NULL)) return 0;
    EmptyClipboard();
    
    size_t size = (wcslen(buffer) + 1) * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
    if (!hMem) {
        CloseClipboard();
        return 0;
    }

    wchar_t* pMem = (wchar_t*)GlobalLock(hMem);
    wcscpy_s(pMem, wcslen(buffer) + 1, buffer);
    GlobalUnlock(hMem);

    SetClipboardData(CF_UNICODETEXT, hMem);
    CloseClipboard();
    return 1;
}

// Определить раскладку
int detect_layout(const wchar_t* text) {
    int qwerty_chars = 0;
    int cyrillic_chars = 0;
    
    for (int i = 0; text[i]; i++) {
        wchar_t c = text[i];
        if ((c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z')) {
            qwerty_chars++;
        } else if (c >= 0x0400 && c <= 0x04FF) { // Unicode Cyrillic range
            cyrillic_chars++;
        }
    }
    
    if (cyrillic_chars > qwerty_chars) return 0; // Cyrillic
    if (qwerty_chars > cyrillic_chars) return 1; // QWERTY
    return -1; // Undetermined
}

// Ожидание отпускания клавиш
void wait_for_key_release(UINT vk1, UINT vk2) {
    while (1) {
        SHORT state1 = GetAsyncKeyState(vk1);
        SHORT state2 = GetAsyncKeyState(vk2);
        
        // Проверяем старший бит (флаг нажатия)
        if (!(state1 & 0x8000) && !(state2 & 0x8000)) {
            break;
        }
        Sleep(10);
    }
}

// Эмуляция копирования (Ctrl+C)
void send_copy_command() {
    INPUT inputs[4] = {0};
    
    // Ctrl down
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;
    
    // C down
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'C';
    
    // C up
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'C';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    
    // Ctrl up
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
    
    SendInput(4, inputs, sizeof(INPUT));
}

// Эмуляция вставки текста (Ctrl+V)
void send_paste_command() {
    INPUT inputs[4] = {0};
    
    // Ctrl down
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;
    
    // V down
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'V';
    
    // V up
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'V';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    
    // Ctrl up
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
    
    SendInput(4, inputs, sizeof(INPUT));
}

// Получить выделенный текст
wchar_t* get_selected_text() {
    // Сохраняем текущий буфер обмена
    wchar_t* old_clipboard = NULL;
    int had_clipboard = get_clipboard(&old_clipboard);
    
    // Очищаем буфер обмена
    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        CloseClipboard();
    }
    
    // Копируем выделенный текст
    send_copy_command();
    Sleep(100); // Даем время на копирование
    
    // Получаем новый текст из буфера
    wchar_t* selected_text = NULL;
    get_clipboard(&selected_text);
    
    // Восстанавливаем старый буфер обмена
    if (had_clipboard && old_clipboard) {
        set_clipboard(old_clipboard);
    } else if (OpenClipboard(NULL)) {
        EmptyClipboard();
        CloseClipboard();
    }
    
    if (old_clipboard) free(old_clipboard);
    return selected_text;
}

// Обработчик горячей клавиши
void process_hotkey() {
    // Ждем отпускания Alt и F12
    wait_for_key_release(VK_MENU, VK_F12);
    
    // Получаем выделенный текст
    wchar_t* clipboard_text = get_selected_text();
    if (!clipboard_text || !*clipboard_text) {
        if (clipboard_text) free(clipboard_text);
        return;
    }

    int layout = detect_layout(clipboard_text);
    wchar_t* converted = (wchar_t*)malloc((wcslen(clipboard_text) + 1) * sizeof(wchar_t));
    
    if (converted) {
        switch (layout) {
            case 1: // QWERTY -> Cyrillic
                switch_layout(clipboard_text, converted, 1);
                break;
            case 0: // Cyrillic -> QWERTY
                switch_layout(clipboard_text, converted, 0);
                break;
            default: // Неопределенная раскладка
                free(clipboard_text);
                free(converted);
                return;
        }
        
        if (set_clipboard(converted)) {
            Sleep(50); // Даем время на обновление буфера
            send_paste_command();
        }
        free(converted);
    }
    free(clipboard_text);
}

// Проверка наличия автозагрузки
int is_autorun_enabled() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, 
                     L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 
                     0, 
                     KEY_READ, 
                     &hKey) != ERROR_SUCCESS) {
        return 0;
    }

    wchar_t appPath[MAX_PATH];
    DWORD bufSize = sizeof(appPath);
    LSTATUS status = RegGetValueW(hKey, 
                                NULL, 
                                L"LayoutSwitcher", 
                                RRF_RT_REG_SZ, 
                                NULL, 
                                appPath, 
                                &bufSize);
    
    RegCloseKey(hKey);
    return (status == ERROR_SUCCESS);
}

// Включить/выключить автозагрузку
void set_autorun(int enable) {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, 
                     L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 
                     0, 
                     KEY_WRITE, 
                     &hKey) != ERROR_SUCCESS) {
        return;
    }

    if (enable) {
        wchar_t appPath[MAX_PATH];
        GetModuleFileNameW(NULL, appPath, MAX_PATH);
        RegSetValueExW(hKey, 
                      L"LayoutSwitcher", 
                      0, 
                      REG_SZ, 
                      (BYTE*)appPath, 
                      (wcslen(appPath) + 1) * sizeof(wchar_t));
    } else {
        RegDeleteValueW(hKey, L"LayoutSwitcher");
    }

    RegCloseKey(hKey);
}

// Добавить иконку в системный трей
void AddTrayIcon(HWND hwnd) {
    NOTIFYICONDATA nid = {0};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = TRAY_ICON_ID;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    
    // Загрузка иконки из ресурсов
    nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(1));
    
    // Если иконка не загружена, используем стандартную
    if (nid.hIcon == NULL) {
        nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }
    
    wcscpy_s(nid.szTip, sizeof(nid.szTip)/sizeof(WCHAR), L"Раскладка текста");
    Shell_NotifyIcon(NIM_ADD, &nid);
}

// Удалить иконку из системного трея
void RemoveTrayIcon(HWND hwnd) {
    NOTIFYICONDATA nid = {0};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = TRAY_ICON_ID;
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

// Обработчик сообщений окна
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            AddTrayIcon(hwnd);
            if (!RegisterHotKey(hwnd, 1, MOD_ALT, VK_F12)) {
                // Ошибка регистрации горячей клавиши
            }
            break;
            
        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP) {
                POINT pt;
                GetCursorPos(&pt);
                
                HMENU hMenu = CreatePopupMenu();
                AppendMenuW(hMenu, MF_STRING, IDM_ABOUT, L"О программе");
                
                // Пункт автозагрузки с галочкой
                if (is_autorun_enabled()) {
                    AppendMenuW(hMenu, MF_STRING | MF_CHECKED, IDM_AUTORUN, L"Автозагрузка");
                } else {
                    AppendMenuW(hMenu, MF_STRING, IDM_AUTORUN, L"Автозагрузка");
                }
                
                AppendMenuW(hMenu, MF_STRING, IDM_EXIT, L"Закрыть");
                
                SetForegroundWindow(hwnd);
                TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
                DestroyMenu(hMenu);
            }
            break;
            
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDM_ABOUT:
                    MessageBoxW(hwnd, 
                        L"Переключатель раскладки\n\n"
                        L"Alt+F12 - автоматическое переключение раскладки выделенного текста\n\n"
                        L"Автоматически определяет раскладку и преобразует текст",
                        L"О программе", MB_OK | MB_ICONINFORMATION);
                    break;
                    
                case IDM_AUTORUN:
                    if (is_autorun_enabled()) {
                        set_autorun(0);
                    } else {
                        set_autorun(1);
                    }
                    break;
                    
                case IDM_EXIT:
                    DestroyWindow(hwnd);
                    break;
            }
            break;
            
        case WM_HOTKEY:
            if (wParam == 1) {
                process_hotkey();
            }
            break;
            
        case WM_DESTROY:
            RemoveTrayIcon(hwnd);
            UnregisterHotKey(hwnd, 1);
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Точка входа
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    setlocale(LC_ALL, "");
    
    // Регистрация класса окна
    const wchar_t CLASS_NAME[] = L"LayoutSwitcherClass";
    
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    
    if (!RegisterClass(&wc)) {
        MessageBox(NULL, L"Ошибка регистрации класса окна", L"Ошибка", MB_ICONERROR);
        return 1;
    }
    
    // Создание невидимого окна
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Переключатель раскладки",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);
    
    if (!hwnd) {
        MessageBox(NULL, L"Ошибка создания окна", L"Ошибка", MB_ICONERROR);
        return 1;
    }
    
    // Основной цикл сообщений
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int) msg.wParam;
}