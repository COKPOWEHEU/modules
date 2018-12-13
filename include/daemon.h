#ifndef __DAEMON_H__
#define __DAEMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#define DMN_NOT_FILE  -1  //невозможно выполнить команду stop/restart
#define DMN_INV_ARG   -2  //неверный аргумент запуска
#define DMN_FORK      -4  //невозможно запустить демона (ошибка ОС)
#define DMN_DMN_PID   -5  //демон создался, но не может получить PID, лучше выйти
#define DMN_DMN_EXIST 2   //демон уже запущен (либо не получается открыть lock-file), стоит выйти
#define DMN_OK        1   //демон создан в другом потоке. Этот можно закрывать
#define DMN_DAEMON    0   //демон создан в этом потоке. Можно продолжать работу

//запуск программы в режиме демона и обычной программы одновременно. Обычную надо корректно завершить
//ВАЖНО: сбрасывает текущий путь, используйте только абсолютные пути к файлам
int dmn_init(int argc, char **argv, void (*exitfunc)(int));
//то же, но самостоятельно выводит сообщения об ошибках
int dmn_init_std(int argc, char **argv, void (*exitfunc)(int));
//завершение работы демона вручную
//надежнее, но не обязательно
void dmn_close();
//проверка, не поступил ли сигнал завершения (0-поступил, надо завершать, 1-не поступил, работаем дальше)
char dmn_running();

extern char DMN_LOCK_FILE[1024];
extern char DMN_LOG_FILE[1024];

#define dmn_log(...) \
  do{\
    FILE *log = fopen(DMN_LOG_FILE,"at");\
    if(log == NULL)break;\
    fprintf(log,__VA_ARGS__);\
    fclose(log);\
  }while(0);

#ifdef __cplusplus
}
#endif
  
#endif
