#include <stdio.h>
#include "test.h"
#include "krb_tree.h"
#include "khash.h"
#include "kstr.h"
#include "kutils.h"

static void check_rb_op(krb_tree *tree, int *nb, int exist_flag) {
    struct krb_node *node;
    krb_tree_check_consistency(tree);
    node = krb_tree_get_node(tree, nb);
    assert((exist_flag && node != NULL) || (! exist_flag && node == NULL));
    if (exist_flag) assert(node->key == nb);
}

UNIT_TEST(krb_tree) {
    krb_tree tree;
    khash nb_hash;
    int index;
    int nb_op = 5000;
    int max_nb_size = 1000; 
    
    krb_tree_init_func(&tree, krb_tree_int_cmp);
    khash_init_func(&nb_hash, khash_int_key, khash_int_cmp);

    for (index = 0; index < nb_op; index++) {
        
        /* Insert. */
        if (kutil_get_random_int(10) <= 5 && index < nb_op - max_nb_size - 1) {
            int *nb = (int *) kmalloc(sizeof(int));
            *nb = kutil_get_random_int(max_nb_size);

            if (khash_exist(&nb_hash, nb)) {
                kfree(nb);
                continue;
            }

            check_rb_op(&tree, nb, 0);
            khash_add(&nb_hash, nb, nb);
            krb_tree_add_fast(&tree, nb, nb);
            check_rb_op(&tree, nb, 1);
        }

        /* Delete. */
        else
        {
            int size = nb_hash.size;
            int pos, iter_index = -1, i, *nb;
            if (size == 0) continue;

            pos = kutil_get_random_int(size - 1);

            for (i = 0; i < pos; i++) khash_iter_next_value(&nb_hash, &iter_index);
            nb = (int *) khash_iter_next_value(&nb_hash, &iter_index);

            check_rb_op(&tree, nb, 1);
            khash_remove(&nb_hash, nb);
            krb_tree_remove(&tree, nb);
            check_rb_op(&tree, nb, 0);
            kfree(nb);
        }
    }

    assert(nb_hash.size == 0);
    
    krb_tree_clean(&tree);
    khash_clean(&nb_hash);
}
