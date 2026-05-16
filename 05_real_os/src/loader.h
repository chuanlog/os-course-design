#ifndef LOADER_H
#define LOADER_H

#include <stdint.h>

/* 从文件系统中加载指定的文件，并将其作为程序运行 */
void loader_exec(const char* filename);

#endif