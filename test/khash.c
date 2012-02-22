#include <khash.h>
#include <kstr.h>
#include <kutils.h>
#include "test.h"

#if 0
#ifndef MAX
# define MAX(a,b) (a)>(b)?(a):(b)
#endif

#ifndef MIN
# define MIN(a,b) (a)<(b)?(a):(b)
#endif

void get_perf_stat(khash *h, int *nb_string_ptr, int *longest_ptr)
{
	*nb_string_ptr = 0;
	*longest_ptr = 0;
	int current_len = 0;
        int i;
	
	for (i = 0; i < h->alloc_size; i++)
	{
		if (h->cell_array[i].key == NULL)
		{
			if (current_len != 0)
			{
				*nb_string_ptr += 1;
				*longest_ptr = MAX(*longest_ptr, current_len);
				current_len = 0;
			}
		}
		
		else
		{
			current_len++;
		}
	}
	
	if (current_len != 0)
	{
		*nb_string_ptr += 1;
		*longest_ptr = MAX(*longest_ptr, current_len);
		current_len = 0;
	}
}

static void stat_khash()
{
    khash h;
    int i, j;
    const int nb_elem = 10000;
    void *ptr[nb_elem];

    khash_init(&h);

    for (i = 0; i < 20; i++)
    {

        khash_clear(&h);

        util_generate_random((char *)ptr, nb_elem * sizeof(void *));

        for (j = 0; j < nb_elem; j++)
        {
            khash_add(&h, ptr[j], NULL);
        }

        int nb_string;
        int longest_string;
        get_perf_stat(&h, &nb_string, &longest_string);
        float avg_string_len = h.size / (float) nb_string;

        TASSERT(longest_string < 10);
        TASSERT(avg_string_len < 7.0);

        printf("    nb string %d, longest %d, average %f, collisions %f\n", nb_string, longest_string, avg_string_len, ((float)h.nb_collision)/h.size);
    }
}
#endif

static khash h;

void test_khash_iter() {
    struct khash_iter hash_iter;
    kiter *iter = (kiter *)&hash_iter;
    struct khash_cell *cell;
    int nb1 = 1;
    int nb2 = 2;
    int count = 0;

    khash_reset(&h);
    khash_set_func(&h, khash_int_key, khash_int_cmp);
    khash_add(&h, &nb1, &nb1);
    khash_add(&h, &nb2, &nb2);

    khash_iter_init(&hash_iter, &h);

    while(kiter_next(iter, (void *)&cell) == 0) {
        count++;
    }
    TASSERT(count == 2);
}

UNIT_TEST(khash) {
    kstr str;
    char *one = "one";
    int a = 1;
    int nb1 = 3;
    int nb2 = 4;
    int nb3 = 3;
    int *val;
    
    khash_init(&h);
    kstr_init(&str);
    
    khash_set_func(&h, khash_cstr_key, khash_cstr_cmp);
    khash_add(&h, one, &a);
    TASSERT(khash_get(&h, one, NULL, (void **)&val) == 0 && val == &a);
    kstr_sf(&str, "%c%c%c", 'o', 'n', 'e');
    TASSERT(khash_get(&h, str.data, NULL, (void **)&val) == 0 && val == &a);

    khash_reset(&h);
    khash_set_func(&h, khash_int_key, khash_int_cmp);
    khash_add(&h, &nb1, &nb1);
    khash_add(&h, &nb2, &nb2);
    TASSERT(khash_get(&h, &nb3, (void**)&val, NULL) == 0 && val == &nb1);

    test_khash_iter();
    
    khash_clean(&h);
    kstr_clean(&str);
}

