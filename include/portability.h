/*
 *  Linux
 */
#if defined(linux) || defined(__linux) || defined(__linux__) || defined(__GNU__) || defined(__GLIBC__)
  #ifndef linux
    #define linux 1 //чтобы была всего одна константа, а не десяток
  #endif
  //Linux
  #include <dlfcn.h>
  #include <unistd.h>
  
  #define DynLoad(s) dlopen(s, RTLD_LAZY)
  #define DynFunc(lib, name)  dlsym(lib,name)
  #define DynClose(lib) dlclose(lib)
  #define DynSuffix ".so"
  #define msleep(time_ms) usleep((time_ms)*1000)
  
  #define my_setlocale(category, locale) setlocale(category, locale)
/*
 *  Win 32
 */
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
  #ifndef WIN32
    #define WIN32 1
  #endif
  //Win32
  #define _CRT_SECURE_NO_WARNINGS
  #define _USE_MATH_DEFINES
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  
  #define DynLoad(s) LoadLibrary(s)
  #define DynFunc(lib, name)   GetProcAddress((HINSTANCE)lib, name)
  #define DynClose(lib) FreeLibrary(lib)
  #define DynSuffix ".dll"
  #define msleep(time_ms) Sleep(time_ms)
  
  #include <io.h>
  #include "fcntl.h"
  
  #define my_setlocale(category, locale) (\
    SetConsoleCP(CP_UTF8), SetConsoleOutputCP(CP_UTF8),\
    _setmode(_fileno(stdout), _O_U8TEXT),\
    _setmode(_fileno(stdin), _O_U8TEXT),\
    setlocale(category, locale) )
  
 /*
  *  Other systems (unsupported)
  */
#else
  #error "Unsupported platform"
#endif

#if 0 //Linux: ввод символа в неканоничном режиме (без необходимости enter'а)
#include <termios.h>
static int getch(){
  struct termios oldt, newt;
  int ch;
  tcgetattr( STDIN_FILENO, &oldt );
  newt = oldt;
  newt.c_lflag &= ~( ICANON | ECHO );
  tcsetattr( STDIN_FILENO, TCSANOW, &newt );
  ch = getchar();
  tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
  return ch;
}
#endif
