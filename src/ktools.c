#include "ktools.h"
#include "kserializable.h"
#include "kerror.h"

#define __BUILD_ID(ID) #ID
#define _BUILD_ID(ID) __BUILD_ID(ID)

char *build_id = _BUILD_ID(BUILD_ID);

void ktools_initialize() {
    kerror_initialize();
    kserializable_initialize();
}

void ktools_finalize() {
    kerror_finalize();
    kserializable_finalize();
}
