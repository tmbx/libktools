#include "string.h"
#include "klist.h"
#include "test.h"

static const char item1[] = "Item 1";
static const char item2[] = "Item 2";
static const char item3[] = "Item 3";

static klist *l1;

void test_klist_append () {
    TASSERT(l1 != NULL);
    klist_append(l1, (void *)item2);
    klist_append(l1, (void *)item3);
    klist_prepend(l1, (void *)item1);
    TASSERT(l1->length == 3);
}

void test_klist_index () {
    char * item_ptr;
    klist_get(l1, 0, (void **) &item_ptr);	
    TASSERT(item_ptr == item1);
    klist_get(l1, 2, (void **) &item_ptr);
    TASSERT(item_ptr == item3);
}

void test_klist_iterator_iter() {
    char * item_ptr;
    struct klist_iter list_iter;
    kiter *iter = (kiter *)&list_iter;
    klist_iter_init(&list_iter, l1);

    kiter_next(iter, (void **) &item_ptr);
    TASSERT(item_ptr == item1);
    kiter_next(iter, (void **) &item_ptr);
    TASSERT(item_ptr == item2);
    kiter_next(iter, (void **) &item_ptr);
    TASSERT(item_ptr == item3);

    TASSERT(kiter_next(iter, (void **) &item_ptr) != 0);

    kiter_prev(iter, (void **) &item_ptr);
    TASSERT(item_ptr == item3);
    kiter_prev(iter, (void **) &item_ptr);
    TASSERT(item_ptr == item2);
    kiter_prev(iter, (void **) &item_ptr);
    TASSERT(item_ptr == item1);
}

void test_klist_iterator_remove() {
    char * item_ptr;
    struct klist_iter list_iter;
    kiter *iter = (kiter *)&list_iter;
    klist_iter_init(&list_iter, l1);

    kiter_next(iter, NULL); //go to first element.
    kiter_next(iter, NULL); //go to second.

    kiter_remove(iter, (void **) &item_ptr);
    TASSERT(item_ptr == item2);
    kiter_get(iter, (void **) &item_ptr);
    TASSERT(item_ptr == item3);
    klist_get(l1, 0, (void **) &item_ptr);
    TASSERT(item_ptr == item1);
    klist_get(l1, 1, (void **) &item_ptr);
    TASSERT(item_ptr == item3);
}

void test_klist_iterator() {
    int i=0;
    int d1 = 1;
    int d2 = 2;
    int *d;
    klist_reset(l1);
    klist_append(l1, &d1);
    klist_append(l1, &d2);

    struct klist_iter iter;
    klist_iter_init(&iter, l1);

    kiter_next(&iter, NULL);
    while (kiter_remove(&iter, &d) == 0) {
        i++;
        TASSERT(*d == i);
    }
    TASSERT(i == 2);
}


UNIT_TEST(klist) {
    l1 = klist_new();

    test_klist_append();
    test_klist_index();
    test_klist_iterator_iter();
    test_klist_iterator_remove();
    test_klist_iterator();

    klist_destroy(l1);
}
