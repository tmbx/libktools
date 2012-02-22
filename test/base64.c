#include <test.h>
#include <base64.h>

const char *base64 = "VGhpcyBpcyBteSB0ZXN0";
const char *text = "This is my test";

void test_b642bin() {
    kbuffer buf1, buf2;
    kbuffer_init(&buf1);
    kbuffer_init(&buf2);
    kbuffer_write(&buf1, (uint8_t *)base64, strlen(base64));
    TASSERT(kb642bin(&buf1, &buf2, 0) == 0);
    TASSERT(memcmp(buf2.data, text, strlen(text)) == 0);
    kbuffer_clean(&buf2);
    kbuffer_clean(&buf1);
}

void test_bin2b64() {
    kbuffer buf1, buf2;
    kbuffer_init(&buf1);
    kbuffer_init(&buf2);
    kbuffer_write(&buf1, (uint8_t *)text, strlen(text));
    kbin2b64(&buf1, &buf2);
    TASSERT(memcmp(buf2.data, base64, strlen(base64)) == 0);
    kbuffer_clean(&buf2);
    kbuffer_clean(&buf1);
}

UNIT_TEST(base64) {
    test_b642bin();
    test_bin2b64();
}
