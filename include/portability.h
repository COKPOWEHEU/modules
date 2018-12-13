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
  #define msleep(time_ms) usleep((time_ms)*1000)
/*
 *  Win 32
 */
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
  #ifndef WIN32
    #define WIN32 1
  #endif
  //Win32
  #define _CRT_SECURE_NO_WARNINGS
  #include <windows.h>
  
  #define DynLoad(s) LoadLibrary(s)
  #define DynFunc(lib, name)   GetProcAddress((HINSTANCE)lib, name)
  #define DynClose(lib) FreeLibrary(lib)
  #define msleep(time_ms) Sleep(time_ms)
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
