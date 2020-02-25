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

/*Прочие подсказки*/
#if 0
g++:
  //запрещает new бросать exception'ы и упрощает проверку
  #include <new>
  a = new (std::nothrow)int[10000000];
    noexcept(a = new int[10000000]); - хуже
  //стандартный способ С++ поймать ошибку:
  try{
    int *p = new int[10000000000];
  }catch(std::bad_alloc){
    printf("Err\n");
  }

  //обертка вокруг С++ функций чтобы избежать декорирования
  #ifdef __cplusplus
  extern "C" {
  #endif
  ...
  #ifdef __cplusplus
  }
  #endif
  
  //упаковка структур
  #pragma pack(push, 1)
  struct __attribute__((__packed__)) StructName{
  ...
  };
  #pragma pack(pop)

  //запрещает игнорировать возвращаемое значение (вроде только с С++17)
  [[nodiscard]] int func(int)
  
  //проверка существования заголовка (вроде только с С++17, но gcc вполне понимает)
  #if __has_include("myinclude.h")
  #include "myinclude.h"
  #endif

  //конструкторы - деструкторы (gcc-специфичное)
  __attribute__((constructor))

gcc/g++
  //включает распараллеливание по стандарту openmp
  -fopenmp
  //TODO: добавить пример кода

  //сохранение метки в переменную
  void *p = &&lab2;
  goto *p;
  lab2:;
  
  //форматные макроконстанты для ввода-вывода переменных фиксированного размера вроде int8_t:
  Использование: scanf("%" SCNxFAST16, &f16_var);
  PRI(t)(modif)(size)
  SCN(t)(modif)(size)
  t - обычные print-scan-овские модификаторы вроде i, d, x и т.п.
  modif - ничего, FAST, LEAST
  size - размер переменной: 8, 16 и т.п.
  Например, PRIdLEAST32, PRId8
  Кроме того:
  PRIdMAX PRIdPTR PRIiMAX PRIiPTR
  
  //добавление бинарных ресурсов в программу:
  $ gcc main.o -Wl,--format=binary -Wl,some.res -Wl,--format=default -o a.out
  extern const uint8_t my_res_start[]   asm("_binary_some_res_start");
  extern const uint8_t my_res_end[]     asm("_binary_some_res_end");
  
UTF-8:
  0xxxxxxx
  110xxxxx 10xxxxxx
  1110xxxx 10xxxxxx 10xxxxxx
  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  
Linux computer name
  /sys/devices/virtual/dmi/id/product_name
  /sys/devices/virtual/dmi/id/board_name
  
Microsoft compiler workaround to use standard functions
  #define _CRT_SECURE_NO_WARNINGS

  
UTF-8 text to console:
  #include <stdio.h>
  #include <wchar.h>
  #include <locale.h>
  #ifdef WIN32
    #include <windows.h>
    #include "fcntl.h"
    __attribute__((constructor)) void coninit(){
      SetConsoleCP(CP_UTF8); SetConsoleOutputCP(CP_UTF8);
      _setmode(_fileno(stdout), _O_U8TEXT);
      _setmode(_fileno(stdin), _O_U8TEXT);
    }
  #endif
 
  int main(){
    setlocale(LC_ALL, "");
    wprintf(L"x\u00B2+7x+4\n");
    return 0;
  }

  
GLUT:
  cflags += -DFREEGLUT_STATIC
  ldflags +=  -lfreeglut_static -static-libgcc -static-libstdc++ -lmingw32 -mconsole -mwindows -lopengl32 -lglu32 -lwinmm
  #Именно в таком порядке!
  
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
