#include <windows.h>
#include <sapi.h>
#include <string>

HWND hwndEdit;
HWND hwndTranslateButton;
HWND hwndHighlightButton;

ISpVoice* pVoice = nullptr;
HRESULT hr;

void PresentPlease(const wchar_t* text)
{
    if (pVoice)
        pVoice->Speak(text, 0, NULL);
}

std::wstring TranslateText(const std::wstring& text, bool toEnglish)
{
    if (toEnglish)
    {
        std::wstring translatedText = L"English: " + text;
        PresentPlease(translatedText.c_str());
        return translatedText;
    }
    else
    {
        std::wstring translatedText = L"¡Presente en español, por favor!";
        PresentPlease(translatedText.c_str());
        return translatedText;
    }
}

void HighlightText()
{
    int start, end;
    SendMessage(hwndEdit, EM_GETSEL, reinterpret_cast<WPARAM>(&start), reinterpret_cast<LPARAM>(&end));

    if (start != end)
    {
        wchar_t* buffer = new wchar_t[end - start + 1];
        SendMessage(hwndEdit, EM_GETSELTEXT, 0, reinterpret_cast<LPARAM>(buffer));

        SendMessage(hwndEdit, EM_SETSEL, start, end);
        SendMessage(hwndEdit, EM_SETBKGNDCOLOR, TRUE, RGB(255, 255, 0));

        PresentPlease(buffer);

        SendMessage(hwndEdit, EM_SETSEL, end, end);
        SendMessage(hwndEdit, EM_SETBKGNDCOLOR, FALSE, RGB(255, 255, 255));

        delete[] buffer;
    }
}

void OnButtonClick(HWND hwndButton)
{
    if (hwndButton == hwndTranslateButton)
    {
        int selectedLanguage = SendMessage(hwndTranslateButton, BM_GETCHECK, 0, 0);
        bool toEnglish = (selectedLanguage == BST_CHECKED);

        int textLength = GetWindowTextLength(hwndEdit) + 1;
        wchar_t* textBuffer = new wchar_t[textLength];
        GetWindowText(hwndEdit, textBuffer, textLength);

        TranslateText(textBuffer, toEnglish);

        SetWindowText(hwndEdit, textBuffer);

        delete[] textBuffer;
    }
    else if (hwndButton == hwndHighlightButton)
    {
        HighlightText();
    }
}

void InitializeSAPI()
{
    hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, reinterpret_cast<void**>(&pVoice));
    if (FAILED(hr))
    {
        MessageBox(NULL, L"Failed to initialize SAPI", L"Error", MB_OK | MB_ICONERROR);
    }
}

void ReleaseSAPI()
{
    if (pVoice)
    {
        pVoice->Release();
        pVoice = nullptr;
    }
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        hwndEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL, 10, 10, 300, 200, hwnd, NULL, NULL, NULL);

        hwndTranslateButton = CreateWindow(L"BUTTON", L"Translate", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 10, 220, 100, 30, hwnd, reinterpret_cast<HMENU>(1), NULL, NULL);
        hwndHighlightButton = CreateWindow(L"BUTTON", L"Highlight", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 120, 220, 100, 30, hwnd, reinterpret_cast<HMENU>(2), NULL, NULL);

        SendMessage(hwndTranslateButton, BM_SETCHECK, BST_CHECKED, 0);

        InitializeSAPI();
        break;

    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED)
        {
            HWND hwndButton = reinterpret_cast<HWND>(lParam);
            OnButtonClick(hwndButton);
        }
        break;

    case WM_CLOSE:
        ReleaseSAPI();
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"TranslatorApp";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProcedure;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc))
    {
        MessageBox(NULL, L"Window Registration Failed", L"Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"Translator App", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 300, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL)
    {
        MessageBox(NULL, L"Window Creation Failed!", L"Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

