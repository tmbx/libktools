#include <karray.h>
#include "test.h"

static karray array1, array2;

void test_iterator () {
    struct karray_iter array_iter;
    kiter *iter = (kiter *)&array_iter;
    int i;
    int arr[] = {0, 1, 2};
    void *v;

    karray_reset(&array1);

    for (i = 0; i < 3 ; i++)
        karray_set(&array1, i, &arr[i]);

    karray_iter_init(&array_iter, &array1);
    
    i = 0;
    while (kiter_next(iter, &v) == 0) {
        TASSERT(i++ == *(int *)v);
    }

    TASSERT(kiter_prev(iter, &v) == 0);
    TASSERT(kiter_next(iter, &v) == 1);

    while (kiter_prev(iter, &v) == 0) {
        TASSERT(--i == *(int *)v);
    }
}

UNIT_TEST(karray) {
    int i = 1, j = 2, k = 3, l = 4;
    karray_init(&array1);
    TASSERT(array1.size == 0);
    
    karray_push(&array1, &i);
    karray_push(&array1, &j);
    TASSERT(array1.size == 2);
    TASSERT(array1.data[0] == &i);
    TASSERT(array1.data[1] == &j);
    
    karray_init_karray(&array2, &array1);
    TASSERT(array2.size == 2);
    TASSERT(array2.data[1] == &j);
    
    karray_set(&array1, 1, &k);
    TASSERT(array1.data[1] == &k);
    TASSERT(array2.data[1] == &j);
    
    karray_set(&array1, 19, &l);
    TASSERT(array1.size == 20);
    TASSERT(array1.data[19] == &l);
    TASSERT(array1.data[1] == &k);
    
    karray_assign_karray(&array2, &array1);
    TASSERT(array2.size == 20);
    TASSERT(array2.data[1] == &k);

    karray_pop(&array2);
    TASSERT(array2.size == 19);
    
    test_iterator();

    karray_clean(&array1);
    karray_clean(&array2);

}
