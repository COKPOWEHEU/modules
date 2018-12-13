#ifndef __MY_SYSINFO__
#define __MY_SYSINFO__

#ifdef __cplusplus
extern "C" {
#endif

#define MSYS_OK 0
#define MSYS_CPU_NOTREADY 1
#define MSYS_CPU_CANT_OPEN 2
#define MSYS_CPU_CPUNUM_ERR 3
#define MSYS_CPU_NOT_ENOUGH_MEM 4
#define MSYS_RAM_CANT_OPEN 5

char info_get_err();
int calc_cpu();
float info_cpu_load();
double info_update_time();
float calc_ram();
float calc_swap();
int calc_temp();
float disk_load(char *diskname);

#ifdef __cplusplus
}
#endif

#endif
