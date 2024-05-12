/*
 * MockStdio.h
 *
 *  Created on: Aug 7 2022
 *      Author: Liam Flaherty
 */

#ifndef _MOCK_STD_MOCKSTDIO_H_
#define _MOCK_STD_MOCKSTDIO_H_

#include <stdio.h>

#define PRINTF_BUFFER_LEN 4096
extern char printfOut[];
extern int printfOutSize;

int mockPrintf(const char * format, ...);

#define printf mockPrintf

/**
 * @brief Clears the printf mock buffer
 * 
 */
void mockClearPrintf(void);

#endif // _MOCK_STD_MOCKSTDIO_H_
