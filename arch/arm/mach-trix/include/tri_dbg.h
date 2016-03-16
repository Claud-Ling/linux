#ifndef __ASM_ARCH_TRIX_TRI_DBG_H__
#define __ASM_ARCH_TRIX_TRI_DBG_H__

#ifdef CONFIG_TRIX_DBG
#define TRI_DBG(fmt,...)  	printk(fmt,##__VA_ARGS__)
#if 0
//#define TRI_DBG(fmt,...)	printk(KERN_DEBUG "[%d]%s:"fmt"\n",__LINE__, __func__, ##__VA_ARGS__); 
#endif
#else
#define TRI_DBG(fmt,...)
#endif


#endif
