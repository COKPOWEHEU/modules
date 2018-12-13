#include "../include/memfile.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
/*
 *  Linux
 */
#if defined(linux) || defined(__linux) || defined(__linux__) || defined(__GNU__) || defined(__GLIBC__)
#include <sys/mman.h>
typedef struct{
  int fd;
  unsigned long size;
}memfile_d;

char memfile_create(const char filename[], unsigned char flags, unsigned long size, struct memfile_t *res){
  int fd;
  void *mem=NULL;
  int i, prot=0, acc=0;
  if(res == NULL)return MEMFILE_EINP;
  res->intdata = NULL;
  res->data = NULL;

  if(flags & MEMFILE_READ){
    prot |= PROT_READ;
    acc  |= S_IRUSR | S_IRGRP | S_IROTH;
    i = O_RDONLY;
  }
  if(flags & MEMFILE_WRITE){
    i = O_CREAT;
    prot |= PROT_WRITE;
    acc  |= S_IWUSR;
    if(flags & MEMFILE_READ){
      i |= O_RDWR;
    }else{
      i |= O_WRONLY;
    }
  }

  fd = open(filename, i, acc);
  if(fd < 0)return MEMFILE_EFILE;
  if(flags & MEMFILE_WRITE){
    if(flags & MEMFILE_REWRITE){
      for(acc=0; acc<size; acc++)write(fd, " ", 1);
    }else if(flags & MEMFILE_FASTREWRITE){
      lseek(fd, size, SEEK_SET);
      write(fd, " ", 1);
    }
  }
  mem = mmap(NULL, size, prot, MAP_SHARED, fd, 0);
  if(mem == NULL){
    close(fd);
    return MEMFILE_EMEM;
  }

  res->intdata = malloc(sizeof(memfile_d));
  ((memfile_d*)(res->intdata))->fd = fd;
  ((memfile_d*)(res->intdata))->size = size;
  res->data = mem;
  
  return MEMFILE_OK;
}
void memfile_close(struct memfile_t *mem){
  int fd;
  unsigned long size;
  if(mem == NULL)return;
  if(mem->intdata == NULL)return;
  fd = ((memfile_d*)(mem->intdata))->fd;
  size = ((memfile_d*)(mem->intdata))->size;
  munmap(mem->data, size);
  close(fd);
  free(mem->intdata);
  mem->intdata = NULL;
  mem->data = NULL;
}
/*
 *  Win 32
 */
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
 //Win32
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
typedef struct{
  HANDLE fd, fm;
}memfile_d;
char memfile_create(const char filename[], unsigned char flags, unsigned long size, struct memfile_t *res){
  HANDLE fd, fm;
  void *mem;
  int cflag=0, mflag=0, vflag=0;
  if(res == NULL)return MEMFILE_EINP;
  res->intdata = NULL;
  res->data = NULL;
  
  if(flags & MEMFILE_READ){
    cflag |= GENERIC_READ;
    mflag = PAGE_READONLY;
    vflag |= FILE_MAP_READ;
  }
  if(flags & MEMFILE_WRITE){
    cflag |= GENERIC_WRITE;
    mflag = PAGE_READWRITE;
    vflag |= FILE_MAP_WRITE;
  }
  
  if(flags & MEMFILE_WRITE){
    fd = CreateFile(filename, cflag, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  }else{
    fd = CreateFile(filename, cflag, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  }
  
  
  if(fd == INVALID_HANDLE_VALUE)return MEMFILE_EFILE;
  if(fd == NULL)return MEMFILE_EFILE;

  if(flags & MEMFILE_WRITE){
    if(flags & MEMFILE_REWRITE){
      char sample = ' ';
      long unsigned int cnt1=0;
      //в документации сказано, что последние 2 параметра МОГУТ быть NULL, однако,
      //в windows это приводит к вылету
      //в wine все в порядке
      for(cflag=0; cflag<size;cflag++)WriteFile(fd, &sample, 1, &cnt1, NULL);
    }else if(flags & MEMFILE_FASTREWRITE){
      SetFilePointer(fd, size, NULL, FILE_BEGIN);
    }
  }
  
  fm = CreateFileMapping(fd, NULL, mflag, 0, size, NULL);
  if(fm == NULL){
    CloseHandle(fm);
    return MEMFILE_EFILE;
  }
  mem = MapViewOfFile(fm, vflag, 0, 0, size);
  if(mem == NULL){
    CloseHandle(fm);
    CloseHandle(fd);
    return MEMFILE_EMEM;
  }
  res->intdata = malloc(sizeof(memfile_d));
  ((memfile_d*)(res->intdata))->fd = fd;
  ((memfile_d*)(res->intdata))->fm = fm;
  res->data = mem;
  
  return MEMFILE_OK;
}
void memfile_close(struct memfile_t *mem){
  if(mem == NULL)return;
  if(mem->intdata == NULL)return;
  HANDLE fd, fm;
  fd = ((memfile_d*)(mem->intdata))->fd;
  fm = ((memfile_d*)(mem->intdata))->fm;
  UnmapViewOfFile(mem->data);
  CloseHandle(fm);
  CloseHandle(fd);
}
 /*
  *  Other systems (unsupported)
  */
#else
  #error "Unsupported platform"
#endif
