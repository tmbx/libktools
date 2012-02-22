#ifndef __K_BASE64_H__
#define __K_BASE64_H__

#include <string.h>
#include <stdint.h>
#include <kbuffer.h>

void kbin2b64(kbuffer *bin, kbuffer *b64);

int kb642bin(kbuffer *b64, kbuffer *bin, int ignore_invalid);

#endif /*__K_BASE64_H__*/
