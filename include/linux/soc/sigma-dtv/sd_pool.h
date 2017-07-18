#ifndef __SD_POOL_H__
#define __SD_POOL_H__


/*
 * @fn		void *sd_pool_alloc(int sz);
 * @brief	allocate memory from sd memory pool
 * @param[in]	<sz> - buffer length need be allocated
		should be less than 2048 bytes

 * @return	NULL failed; valid ptr success
 */
void *sd_pool_alloc(int sz);

/*
 * @fn		void sd_pool_free(void *ptr);
 * @brief	allocate memory from sd memory pool
 * @param[in]	<ptr> - buffer ptr want to free

 * @return	N/A
 */
void sd_pool_free(void *ptr);
#endif
