#include "kbuffer.h"
#include "test.h"

UNIT_TEST(kbuffer) {
    kbuffer buf;
    uint64_t data;
    kbuffer_init(&buf);
    kbuffer_write64(&buf, 0x1122334455667788ll);
    kbuffer_read64(&buf, &data);
    TASSERT(data == 0x1122334455667788ll);

    kbuffer_clean(&buf);
};
