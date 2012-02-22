#include <kstr.h>
#include "test.h"

UNIT_TEST(kstr) {
    kstr str1, str2, str3;
    kstr_init(&str1);
    kstr_init_cstr(&str2, "bar");
    kstr_init_kstr(&str3, &str2);

    TASSERT(kstr_equal_cstr(&str2, "bar"));
    TASSERT(kstr_equal_kstr(&str2, &str3));

    kstr_assign_cstr(&str3, "foobar");
    kstr_assign_kstr(&str1, &str3);
    kstr_sf(&str2, "%sbar", "foo");
    
    TASSERT(kstr_equal_kstr(&str3, &str2));
    TASSERT(kstr_equal_kstr(&str1, &str2));

    kstr_sf(&str2, "%s buffer to grow", "I want my");
    TASSERT(kstr_equal_cstr(&str2, "I want my buffer to grow"));

    kstr_reset(&str2);
    TASSERT(kstr_equal_cstr(&str2, ""));
    
    kstr_append_char(&str2, 'f');
    kstr_append_char(&str2, 'o');
    kstr_assign_cstr(&str1, "ob");
    kstr_append_kstr(&str2, &str1);
    kstr_append_cstr(&str2, "ar");
    TASSERT(kstr_equal_cstr(&str2, "foobar"));

    kstr_clean(&str1);
    kstr_clean(&str2);
    kstr_clean(&str3);
}
