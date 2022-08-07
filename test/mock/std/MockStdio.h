/*
 * MockStdio.h
 *
 *  Created on: Aug 7 2022
 *      Author: Liam Flaherty
 */

#ifndef _MOCK_STD_MOCKSTDIO_H_
#define _MOCK_STD_MOCKSTDIO_H_

#define PRINTF_BUFFER_LEN 257
extern char printfOut[];
extern int printfOutSize;

#define printf(format, args...) \
    printfOutSize += snprintf(printfOut + printfOutSize, PRINTF_BUFFER_LEN, format, ## args);

/**
 * @brief Clears the printf mock buffer
 * 
 */
void mockClearPrintf(void);

#endif // _MOCK_STD_MOCKSTDIO_H_