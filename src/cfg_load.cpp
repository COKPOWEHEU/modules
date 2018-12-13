#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "cfg_load.h"

static void str2lower(char *dst, char *src, size_t n){
  while(src[0]!='\0' && n>0){
    dst[0] = tolower(src[0]);
    src++; dst++; n--;
  }
  dst[0]=0;
}

void CfgElem::deinit(){
  if(data == NULL){ data = NULL; return;}
  if(data_size == 0){ free(data); data = NULL; return;}
  for(size_t i=0; i<data_size; i++)list[i].deinit();
  free(list);
  list = NULL; data_size = 0;
}

CfgElem* CfgElem::addElem(){
  if(data != NULL && data_size == 0){
    return NULL;
  }
  CfgElem *temp = (CfgElem*)realloc((void*)list, sizeof(CfgElem)*(data_size+1));
  if(temp == NULL){ //если не можем выделить память, не имеет смысла продолжать работу
    deinit();
    return NULL;
  }
  list = temp;
  list[data_size].init();
  data_size++;
  return &list[data_size-1];
}

void CfgElem::Display(int offset){
  for(int i=0; i<offset; i++)putchar(' ');
  printf("[%s] = ", name);
  if(data_size == 0){
    if(data == NULL)printf("---\n");
    else printf("<%s>\n", data);
    return;
  }
  printf("{\n");
  for(size_t i=0; i<data_size; i++){
    list[i].Display(offset+2);
  }
  for(int i=0; i<offset; i++)putchar(' ');
  printf("}\n");
}

char* CfgElem::ReadStr(char str[]){
  if(errline >= 0)return NULL;
  while(1){
    if(data_size == 0 && list != NULL)return NULL; //ошибка записи в лист
    if(inp != NULL){
      char *res;
      res = inp->ReadStr(str);
      if(res == NULL){
        errline = inp->errline;
        if(inp->errline < 0)return NULL;
        return NULL;
      }
      if(res[0] == '}'){
        str = res+1;
        errline = inp->errline;
        inp = NULL;
      }
    }
    //ищем начало строки (имя узла/листа или закрывающую скобку)
    while(isspace(str[0])){if(str[0]==0)return NULL; if(str[0]=='\n')errline--; str++;}
    if(str[0] == '#')return NULL;
    if(str[0] == '}')return str;
    //строка является узлом/листом. Ищем конец имени
    char *dat = str;
    size_t ncnt;
    while(!isspace(dat[0]) && dat[0]!='='){if(dat[0]==0)return NULL; dat++;}
    ncnt = (dat - str);
    if(ncnt > 19)ncnt=19;
    //ищем начало данных
    while(isspace(dat[0]) || dat[0]=='='){
      if(dat[0]==0)return NULL;
      if(dat[0]=='\n'){errline=-errline; return NULL;}
      dat++;
    }
    //создаем новый элемент
    CfgElem *elem = addElem();
    if(elem == NULL){
      errline = CFGEL_RAM;
      return NULL;
    }
    strncpy(elem->name, str, ncnt);
    elem->name[ncnt] = 0; 
    
    if(dat[0] != '{'){ //новый элемент - лист
      char *edat = dat;
      size_t dcnt;
      while(edat[0]!='\n' && edat[0]!=0){edat++;}
      dcnt = (edat - dat);
      elem->data = (char*)malloc(dcnt+1);
      if(elem->data == NULL){
        errline = CFGEL_RAM;
        return NULL;
      }
      strncpy(elem->data, dat, dcnt);
      elem->data[dcnt]=0;
      str = edat;
    }else{ //новый элемент - узел
      inp = elem;
      inp->errline = errline;
      str = dat+1;
    }
  }
}

ssize_t CfgElem::ReadFile(const char filename[]){
  FILE *pf = fopen(filename, "rt");
  if(pf == NULL)return CFGEL_FILE;
  char buf[1024]={0};
  while(!feof(pf)){
    buf[1022]='#';
    if(fgets(buf, 1023, pf) == NULL)break;
    if(buf[1022] != '#'){
      fclose(pf); deinit();
      return CFGEL_FLONG;
    }
    ReadStr(buf);
  }
  fclose(pf);
  return ErrLine();
}

CfgElem* CfgElem::GetElem(const char path[]){
  if(path[0] != '/')return NULL;
  path++;
  const char *pos = strchr(path, '/');
  char isleaf = 0;
  if(pos == NULL){
    if(strncmp(name, path, (pos-path-1))==0)return this;
    isleaf = 1;
  }
  if(data_size == 0)return NULL;
  for(size_t i=0; i<data_size; i++){
    if(strncmp(list[i].name, path, (pos-path-1))==0){
      if(isleaf)return &list[i];
      else return list[i].GetElem(pos);
    }
  }
  return NULL;
}

CfgElem* CfgElem::GetCaseElem(const char path[]){
  char cpath[30];
  char cname[20];
  if(path[0] != '/')return NULL;
  str2lower(cpath, (char*)(path+1), 30);
  const char *pos = strchr(cpath, '/');
  char isleaf = 0;
  if(pos == NULL){
    str2lower(cname, name, 20);
    if(strncmp(cname, cpath, (pos-cpath-1))==0)return this;
    isleaf = 1;
  }
  if(data_size == 0)return NULL;
  for(size_t i=0; i<data_size; i++){
    str2lower(cname, list[i].name, 20);
    if(strncmp(cname, cpath, (pos-cpath-1))==0){
      if(isleaf)return &list[i];
      else{
        pos = path+(pos-cpath+1);
        return list[i].GetCaseElem(pos);
      }
    }
  }
  return NULL;
}

ssize_t CfgElem::ErrLine(){
  if(errline < 0)return CFGEL_OK;
  deinit();
  if(errline == CFGEL_RAM)return CFGEL_MEM;
  return errline;
}

//---- C compatibility ----------------
Cfg_t CfgOpen(){
  Cfg_t res;
  res.elem = new CfgElem;
  return res;
}
void CfgClose(Cfg_t cfg){
  if(cfg.elem != NULL)delete ((CfgElem*)(cfg.elem));
}
char* CfgAddStr(Cfg_t cfg, char str[]){
  return ((CfgElem*)(cfg.elem))->ReadStr(str);
}
size_t CfgLoadFile(Cfg_t cfg, char filename[]){
  return ((CfgElem*)(cfg.elem))->ReadFile(filename);
}
Cfg_t CfgGetElem(Cfg_t cfg, const char path[]){
  Cfg_t res;
  res.elem = ((CfgElem*)(cfg.elem))->GetElem(path);
  return res;
}
Cfg_t CfgGetCaseElem(Cfg_t cfg, const char path[]){
  Cfg_t res;
  res.elem = ((CfgElem*)(cfg.elem))->GetCaseElem(path);
  return res;
}
char* CfgName(Cfg_t cfg){
  if(cfg.elem == NULL)return NULL;
  return ((CfgElem*)(cfg.elem))->name;
}
char* CfgData(Cfg_t cfg){
  if(cfg.elem == NULL)return NULL;
  if(((CfgElem*)(cfg.elem))->IsLeaf())return ((CfgElem*)(cfg.elem))->data;
  return NULL;
}
void CfgDisplay(Cfg_t cfg){
  ((CfgElem*)(cfg.elem))->Display(0);
}
