#define APP_TITLE    "Clipboard v1.0"

#define ID_CLIPBOARD 1
#define IDM_ABOUT    2
#define IDM_HELP     3
#define IDD_ABOUT    4
#define IDD_HELP     5

MRESULT EXPENTRY ClipboardWndProc(HWND, USHORT, MPARAM, MPARAM);
MRESULT EXPENTRY AboutDlgProc(HWND, USHORT, MPARAM, MPARAM);
