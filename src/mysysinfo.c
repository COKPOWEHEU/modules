#include <unistd.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/time.h>
#include "mysysinfo.h"
#include <stdio.h>

static char numcpu=0;
static char lasterr=0;
struct cpu_load_t{
  float load;
  unsigned long long active_last;
  unsigned long long total_last;
};
struct cpu_load_t *cpu_load = NULL;

struct meminfo_t{
  unsigned long long memmax;
  unsigned long long memfree;
  unsigned long long swapmax;
  unsigned long long swapfree;
  unsigned long long buffers;
  unsigned long long cached;
};
struct meminfo_t mem;

char info_get_err(){return lasterr;}

int calc_cpu(){
 int res;
 FILE *pf=fopen("/proc/cpuinfo","rt");
 if(pf==NULL){printf("E (mysysinfo): can't open /proc/cpuinfo\n"); numcpu=-1; return -1;}
 do{
  char str[100];
  res=fscanf(pf,"%s",str);
  if(strcmp(str,"processor")==0)numcpu++;
 }while(res!=EOF);
 fclose(pf);
 return numcpu;
}

double get_time(){
  struct timeval t;
  gettimeofday (&t, 0);
  return (double)t.tv_sec+(double)t.tv_usec*1e-6;
}

float disk_load(char *diskname){
  static double ltime;
  double ctime;
  static unsigned lbusyr_ms;//, lbusyw_ms;
  unsigned int cbusyr_ms=0, cbusyw_ms=0;
  char defdisk[]="sda";
  char *disk = defdisk;
  float res;
  FILE *pf;
  char buf[256];
  if(diskname != NULL)disk=diskname;
  pf = fopen("/proc/diskstats","rt");
  if(!pf)return -1;
  while(!feof(pf)){
    fscanf(pf,"%*u%*u%s",buf);
    if(strcmp(buf,disk)!=0){
      fgets(buf, sizeof(buf), pf);
      continue;
    }
    fscanf(pf,"%*u%*u%*u%u %*u%*u%*u%u",&cbusyr_ms, &cbusyw_ms);
    fgets(buf, sizeof(buf), pf);
    break;
  }
  fclose(pf);
  ctime = get_time();
  res = cbusyr_ms - lbusyr_ms;
  res /= (ctime - ltime);
  lbusyr_ms = cbusyr_ms;
  //lbusyw_ms = cbusyw_ms;
  ltime = ctime;
  return res;
}
char UpdateStat(double time_abs){
  FILE *stat;
  unsigned long long active,total;
  unsigned long long cpu_user,cpu_nice,cpu_system,cpu_idle,cpu_iowait,cpu_irq,cpu_softirq,cpu_steal;
  unsigned char num;
  char buf[255];
  static double oldtime=0;
  if(time_abs-oldtime < 0.01)return MSYS_CPU_NOTREADY;
  oldtime = time_abs;
  stat = fopen("/proc/stat","rt");
  if(stat == NULL)return MSYS_CPU_CANT_OPEN;
  if(cpu_load == NULL || numcpu<=0){
    if(calc_cpu() <= 0)return MSYS_CPU_CPUNUM_ERR;
    cpu_load = (struct cpu_load_t*)malloc(sizeof(struct cpu_load_t)*(numcpu+1));
    if(cpu_load == NULL)return MSYS_CPU_NOT_ENOUGH_MEM; //маловероятно, что не найдется свободной полусотни байт, но всякое бывает
  }
  while(!feof(stat)){
    if(fgets(buf,255,stat)==NULL)break;
    if(strncmp(buf,"cpu",3)==0){
      if(isdigit(buf[3]))num = atoi(&buf[3])+1; else num=0;
      if(num > numcpu){printf("Err: %i / %i\n",num,numcpu); break;}
      sscanf(buf,"%*s%llu%llu%llu%llu%llu%llu%llu%llu",&cpu_user, &cpu_nice, &cpu_system, &cpu_idle,
             &cpu_iowait, &cpu_irq, &cpu_softirq, &cpu_steal);
      total = cpu_user + cpu_nice + cpu_system + cpu_idle + cpu_iowait + cpu_irq + cpu_softirq + cpu_steal;
      active = total - cpu_idle - cpu_iowait;
      
      if(cpu_load[num].total_last == total)break;
      cpu_load[num].load = (active - cpu_load[num].active_last);
      cpu_load[num].load /= (total - cpu_load[num].total_last);
      
      cpu_load[num].active_last = active;
      cpu_load[num].total_last = total;
    }
  }
  fclose(stat);
  return MSYS_OK;
}
__attribute__((__destructor__)) void delcpu(){
  if(cpu_load != NULL)free(cpu_load);
}

float info_cpu_load(){
  if(cpu_load == NULL)return -1;
  return cpu_load[0].load;
}

char parse_meminfo(){
  FILE *meminfo;
  char buf[255];
  meminfo = fopen("/proc/meminfo","rt");
  if(meminfo == NULL)return MSYS_RAM_CANT_OPEN;
  while(!feof(meminfo)){
    if(fgets(buf,255,meminfo)==NULL)break;
    if(strncmp(buf,"MemTotal:",9)==0)
      sscanf(buf,"%*s%llu",&mem.memmax);
    if(strncmp(buf,"MemFree:",8)==0)
      sscanf(buf,"%*s%llu",&mem.memfree);
    if(strncmp(buf,"SwapTotal:",10)==0)
      sscanf(buf,"%*s%llu",&mem.swapmax);
    if(strncmp(buf,"SwapFree:",9)==0)
      sscanf(buf,"%*s%llu",&mem.swapfree);
    if(strncmp(buf,"Buffers:",8)==0)
      sscanf(buf,"%*s%llu",&mem.buffers);
    if(strncmp(buf,"Cached:",7)==0)
      sscanf(buf,"%*s%llu",&mem.cached);
  }
  fclose(meminfo);
  return 0;
}

double info_update_time(){
  char res;
  double time = get_time();
  res = UpdateStat(time);
  if(res != MSYS_OK && res != MSYS_CPU_NOTREADY){
    lasterr = res;
  }
  res = parse_meminfo();
  if(res != MSYS_OK){
    lasterr = res;
  }
  return time;
}

float calc_ram(){
  float res = mem.memfree+mem.buffers+mem.cached;
  if(mem.memmax <= 0)return -100;
  res /= mem.memmax;
  return res;
}

float calc_swap(){
  float res = mem.swapfree;
  if(mem.swapmax <= 0)return -100;
  res /= mem.swapmax;
  return res;
}

int calc_temp(){
 FILE *pf=fopen("/sys/class/thermal/thermal_zone0/temp","rt");
 int res;
 if(pf==NULL){printf("E (mysysinfo): can't open /sys/class/thermal/thermal_zone0/temp\n"); return -1;}
 fscanf(pf,"%i",&res);
 fclose(pf);
 return res;
}
