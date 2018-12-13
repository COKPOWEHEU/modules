#include "daemon.h"
#include <stdio.h> //все равно в основной программе будет использоваться
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

char DMN_LOCK_FILE[1024];
char DMN_LOG_FILE[1024];

static volatile char dmn_run = 0;

static void dmn_def_signal(int num){
  dmn_run = 0;
}

char dmn_running(){
  return dmn_run;
}

int dmn_init_std(int argc, char **argv, void (*exitfunc)(int)){
  int res = dmn_init(argc, argv, exitfunc);
  switch( res ){
    case DMN_NOT_FILE:
      printf("daemon is not running\n");
      break;
    case DMN_INV_ARG:
      printf("usage: %s [stop]\n",argv[0]);
      break;
    case DMN_FORK:
      printf("can not create daemun process (OS error)\n");
      break;
    case DMN_DMN_PID:
      printf("daemon can not get pid (OS error)\n");
      break;
    case DMN_DMN_EXIST:
      printf("daemon already running (or can not open lock file [%s])\n",DMN_LOCK_FILE);
      break;
    case DMN_OK:
    case DMN_DAEMON:
      break;
    default:
      printf("unknown error\n");
  }
  return res;
}

/*
 *  Linux
 */
#if defined(linux) || defined(__linux) || defined(__linux__) || defined(__GNU__) || defined(__GLIBC__)

void dmn_find_files(const char progname[]){
  char *home,*name;
  home = getenv("HOME");
  name = strrchr(progname,'/');
  if(name == NULL)name=(char*)progname;

  strncpy(DMN_LOCK_FILE, home, sizeof(DMN_LOCK_FILE));
  strncat(DMN_LOCK_FILE, "/.log", sizeof(DMN_LOCK_FILE));
  mkdir(DMN_LOCK_FILE, S_IRWXU | S_IRGRP | S_IXGRP);
  strncat(DMN_LOCK_FILE, name, sizeof(DMN_LOCK_FILE));
  strncpy(DMN_LOG_FILE, DMN_LOCK_FILE, sizeof(DMN_LOG_FILE));
  strncat(DMN_LOCK_FILE, ".pid", sizeof(DMN_LOCK_FILE));
  strncat(DMN_LOG_FILE, ".log", sizeof(DMN_LOG_FILE));
}
//
int dmn_init(int argc, char **argv, void (*exitfunc)(int)){
  pid_t pid;
  int fd;
  dmn_find_files(argv[0]);
  if(argc > 1){
    if(strcmp(argv[1],"start")!=0){
      FILE *pidf;
      pidf = fopen(DMN_LOCK_FILE,"rt");
      if(pidf == NULL)return DMN_NOT_FILE;
      fscanf(pidf,"%i",&pid);
      fclose(pidf);
      if(strcmp(argv[1],"stop")==0){
        kill(pid,SIGUSR1);
        return DMN_OK;
      }else if(strcmp(argv[1],"restart")==0){
        kill(pid,SIGUSR1);
        dmn_close();
      }else return DMN_INV_ARG;
    }
  }
  if(exitfunc == NULL){
    exitfunc = dmn_def_signal;
    dmn_run = 1;
  }else{
    dmn_run = 0; //если встроенный сигнал не используется, использовать встроенную проверку тоже бессмысленно
  }
  pid = fork();
  if(pid > 0)return DMN_OK;
  if(pid < 0)return DMN_FORK;
  pid = setsid();
  if(pid < 0)return DMN_DMN_PID;
  fd = open(DMN_LOCK_FILE, O_RDWR | O_CREAT | O_EXCL, 0644);
  if(fd < 0)return DMN_DMN_EXIST;else{
    char buf[16];
    int len;
    len = sprintf(buf,"%i",pid);
    write(fd, buf, len);
    close(fd);
  }
  signal(SIGUSR1,exitfunc);//штатный выход (обычно средствами самого демона)
  signal(SIGINT, exitfunc); //штатные выходы средствами ОС
  signal(SIGTERM,exitfunc);
  signal(SIGHUP, exitfunc);
  signal(SIGQUIT,exitfunc);
  chdir("/");
  fclose(stdin);
  fclose(stdout);
  fclose(stderr);
  return DMN_DAEMON;
}

void dmn_close(){
  if(unlink(DMN_LOCK_FILE) != 0){
    FILE *log = fopen(DMN_LOG_FILE,"at");
    if(log != NULL){
      fprintf(log,"Can not remove lock file [%s]\n",DMN_LOCK_FILE);
      fclose(log);
    }
  }
}

__attribute__((__destructor__))void dmn_destructor(){
  FILE *pidf;
  pid_t dmn_pid, cur_pid;
  pidf = fopen(DMN_LOCK_FILE,"rt");
  if(!pidf)return;
  fscanf(pidf,"%i",&dmn_pid);
  fclose(pidf);
  cur_pid = getsid(0);
  if(dmn_pid != cur_pid)return;
  if(unlink(DMN_LOCK_FILE) != 0){
    pidf = fopen(DMN_LOG_FILE,"at");
    if(pidf != NULL){
      fprintf(pidf,"Can not remove lock file [%s]\n",DMN_LOCK_FILE);
      fclose(pidf);
    }
  }
}
/*
 *  Win 32
 */
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
void dmn_find_files(const char progname[]){
  char *home,*name;
  home = getenv("TEMP");
  name = strrchr(progname,'/');
  if(name == NULL){
    name = strrchr(progname,'\\');
    if(name == NULL)name=(char*)progname;
  }

  strncpy(DMN_LOCK_FILE, home, sizeof(DMN_LOCK_FILE));
  strncat(DMN_LOCK_FILE, "\\.log", sizeof(DMN_LOCK_FILE));
  mkdir(DMN_LOCK_FILE);
  strncat(DMN_LOCK_FILE, name, sizeof(DMN_LOCK_FILE));
  strncpy(DMN_LOG_FILE, DMN_LOCK_FILE, sizeof(DMN_LOG_FILE));
  strncat(DMN_LOCK_FILE, ".pid", sizeof(DMN_LOCK_FILE));
  strncat(DMN_LOG_FILE, ".log", sizeof(DMN_LOG_FILE));
}
//запуск программы в режиме демона и обычной программы одновременно. Обычную надо корректно завершить
//ВАЖНО: сбрасывает текущий путь, используйте только абсолютные пути к файлам
int dmn_init(int argc, char **argv, void (*exitfunc)(int)){
  dmn_find_files(argv[0]);
  if(exitfunc == NULL){
    exitfunc = dmn_def_signal;
    dmn_run = 1;
  }else{
    dmn_run = 0; //если встроенный сигнал не используется, использовать встроенную проверку тоже бессмысленно
  }
  signal(SIGINT, exitfunc); //штатные выходы средствами ОС
  signal(SIGTERM,exitfunc);
  chdir("/");
  fclose(stdin);
  fclose(stdout);
  fclose(stderr);
  return DMN_DAEMON;
}

//завершение работы демона вручную
//надежнее, но не обязательно
void dmn_close(){}

extern char DMN_LOCK_FILE[1024];
extern char DMN_LOG_FILE[1024];
#endif
