#ifndef __KERNEL_KLIB_H__
#define __KERNEL_KLIB_H__

extern struct file *klib_fopen(const char *filename, int flags, int mode) ;
extern void klib_fclose(struct file *filp);
extern int klib_fread(char *buf, int len, struct file *filp);
extern int klib_fwrite(char *buf, int len, struct file *filp);
extern int klib_fseek(struct file *filp, int offset, int whence);
extern int klib_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#endif
