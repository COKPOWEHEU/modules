#ifndef __CFG_LOAD_H__
#define __CFG_LOAD_H__

#include <stdlib.h>

#define CFGEL_OK 0
#define CFGEL_MEM -1
#define CFGEL_FILE -2
#define CFGEL_FLONG -3

#ifdef __cplusplus
//data_size == 0 -> leaf
//data_size > 0 -> array (struct)
class CfgElem{
private:
  const ssize_t CFGEL_RAM = 0;
  size_t data_size;
  CfgElem *inp;
  ssize_t errline;
  
  void init(){
    name[0]=0; data_size=0; data=NULL; inp=NULL; errline=-1;
  }
  void deinit();
  CfgElem* addElem();
public:
  char name[20];
  union{
    char *data;
    CfgElem *list;
  };
  
  CfgElem(){init();}
  ~CfgElem(){deinit();}
  void Display(int offset);
  char IsLeaf(){return (data_size==0 && data != NULL);}
  char* ReadStr(char str[]);
  ssize_t ReadFile(const char filename[]);
  CfgElem* GetElem(const char path[]);
  CfgElem* GetCaseElem(const char path[]);
  ssize_t ErrLine();
};

extern "C" {
#endif

typedef union{void *elem;}Cfg_t;

Cfg_t CfgOpen();
void CfgClose(Cfg_t cfg);
char* CfgAddStr(Cfg_t cfg, char str[]);
size_t CfgLoadFile(Cfg_t cfg, char filename[]);
Cfg_t CfgGetElem(Cfg_t cfg, const char path[]);
Cfg_t CfgGetCaseElem(Cfg_t cfg, const char path[]);
char* CfgName(Cfg_t cfg);
char* CfgData(Cfg_t cfg);
void CfgDisplay(Cfg_t cfg);

#ifdef __cplusplus
}
#endif

#endif
