#include <kindex.h>
#include <kstr.h>
#include <kbuffer.h>
#include <string.h>
#include "test.h"

UNIT_TEST(kserializable) {
    kstr *str = kstr_new();
    kbuffer *buffer = kbuffer_new();
    kindex *hash = kindex_new();
    
    kbuffer *serialized_data = kbuffer_new();

    /* TRY: */
    do {
        kstr_append_cstr(str, "The string to serialize");
        kbuffer_write(buffer, (const uint8_t *)"The buffer to serialize", strlen("The buffer to serialize") + 1);
        TASSERT(kindex_add(hash, 12, (kserializable *)str) == 0);
        str = NULL;
        TASSERT(kindex_add(hash, 12, (kserializable *)buffer) != 0);
        TASSERT(kindex_add(hash, 123487, (kserializable *)buffer) == 0);
        buffer = NULL;

        kserializable_serialize((kserializable *)hash, serialized_data);
        kserializable_destroy((kserializable *)hash);
        hash = NULL;

        TASSERT(kserializable_deserialize((kserializable **)&hash, serialized_data) == 0);

        TASSERT(kindex_has(hash, 12));
        TASSERT(kindex_has(hash, 123487));
        TASSERT(!kindex_has(hash, 13));
        kindex_get(hash, 12, (kserializable **)&str);
        TASSERT(strcmp(str->data, "The string to serialize") == 0);
        kindex_get(hash, 123487, (kserializable **)&buffer);
        TASSERT(memcmp(buffer->data, "The buffer to serialize", MIN(buffer->len, strlen("The buffer to serialize"))) == 0);
    } while (0); 

    kserializable_destroy((kserializable *)hash);
    kbuffer_destroy(serialized_data);
}
