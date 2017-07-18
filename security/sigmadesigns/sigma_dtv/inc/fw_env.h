#ifndef KERNEL_FW_ENV_H
#define KERNEL_FW_ENV_H

int fw_env_open(void);
void fw_env_close(void);
char *fw_getenv (char *name);
void fw_env_write(char *name, char *value);
int fw_printenv(int argc, char *argv[]);

#endif
