#ifndef QIUX_DIRECTORY_H_
#define QIUX_DIRECTORY_H_
#include "type.h"
#define NAME_LEN 14


typedef struct DIRECTORY
{
    uint16_t ino;
    char name[NAME_LEN];    
} directory;

#endif // QIUX_DIRECTORY_H_

