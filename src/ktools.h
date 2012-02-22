#ifndef __K_TOOLS_H__
#define __K_TOOLS_H__

#include "base64.h"
#include "karray.h"
#include "kbuffer.h"
#include "kerror.h"
#include "kthread.h"
#include "kfs.h"
#include "khash.h"
#include "kindex.h"
#include "kiter.h"
#include "klist.h"
#include "kmem.h"
#include "kpath.h"
#include "krb_tree.h"
#include "kserializable.h"
#include "ksock.h"
#include "kstr.h"
#include "ktime.h"
#include "kutils.h"

void ktools_initialize();
void ktools_finalize();

extern char *build_id;

#endif /*__K_TOOLS_H__*/
