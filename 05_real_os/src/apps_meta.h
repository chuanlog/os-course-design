#ifndef APPS_META_H
#define APPS_META_H
#include <stdint.h>
struct prebuilt_app { char name[16]; uint32_t start_lba; uint32_t size; };
static struct prebuilt_app prebuilt_apps[] = {
    {"hello.bin", 5, 61},
    {"calc.bin", 6, 1179},
};
#define PREBUILT_APP_COUNT (sizeof(prebuilt_apps)/sizeof(prebuilt_apps[0]))
#endif
