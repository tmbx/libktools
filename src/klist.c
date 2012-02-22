/**
 * src/klist.c
 * Copyright (C) 2005-2012 Opersys inc., All rights reserved.
 *
 * List utilities.
 */

#include <string.h>
#include "klist.h"
#include "kmem.h"
#include "kutils.h"
#include "kerror.h"

static struct klist_node *klist_node_new(void *data) {
    struct klist_node *self = kcalloc(sizeof(struct klist_node));
    self->data = data;
    return self;
}

static void klist_node_destroy(struct klist_node *self) {
    kfree(self);
}

static inline int klist_is_empty(klist *self) {
    return self->head->next == self->tail;
}

klist *klist_new() {
    klist *self = kmalloc(sizeof(klist));
    klist_init(self);
    return self;
}

void klist_init(klist *self) {
    self->head = klist_node_new(NULL);
    self->head->next = self->tail;
    self->head->prev = self->head;

    self->tail = klist_node_new(NULL);
    self->tail->prev = self->head;
    self->tail->next = self->tail;

    self->length = 0;
}

void klist_init_klist(klist *self, klist *init_list) {
    struct klist_iter list_iter;
    kiter *iter = (kiter *)&list_iter;
    void *data;

    klist_init(self);

    klist_iter_init(&list_iter, init_list);
    while (kiter_next(iter, &data))
        klist_append(self, data);
}

void klist_destroy(klist *self) {
    if (self) {
        klist_clean(self);
    }
    kfree(self);
}

void klist_clean(klist *self) {
    klist_reset(self);
    klist_node_destroy(self->head);
    klist_node_destroy(self->tail);
}

void klist_reset(klist *self) {
    while (self->length > 0)
        klist_rm_head(self, NULL);
}

void klist_prepend(klist *self, void *data) {
    struct klist_node *node = klist_node_new(data);

    node->next = self->head->next;
    node->prev = self->head;
    self->head->next->prev = node;
    self->head->next = node;
    self->length++;
}

void klist_append(klist *self, void *data) {
    struct klist_node *node = klist_node_new(data);
    node->prev = self->tail->prev;
    node->next = self->tail;
    self->tail->prev->next = node;
    self->tail->prev = node;
    self->length++;
}

int klist_rm_head(klist *self, void **el) {
    struct klist_node *node;
    if (klist_is_empty(self)) {
        KTOOLS_ERROR_SET("the list is empty");
        return -1;
    }

    node = self->head->next;
    self->head->next = node->next;
    self->head->next->prev = self->head;
    self->length --;

    if (el)
        *el = node->data;
    klist_node_destroy(node);
    return 0;
}

int klist_rm_tail(klist *self, void **el) {
    struct klist_node *node;
    if (klist_is_empty(self)) {
        KTOOLS_ERROR_SET("the list is empty");
        return -1;
    }

    node = self->tail->prev;
    self->tail->prev = node->prev;
    self->tail->prev->next = self->tail;
    self->length --;

    if (el)
        *el = node->data;
    klist_node_destroy(node);
    return 0;
}

int klist_head(klist *self, void **el) {
    if (klist_is_empty(self)) {
        KTOOLS_ERROR_SET("the list is empty");
        return -1;
    }

    *el = self->head->next->data;
    return 0;
}

int klist_tail(klist *self, void **el) {
    if (klist_is_empty(self)) {
        KTOOLS_ERROR_SET("the list is empty");
        return -1;
    }

    *el = self->tail->prev->data;
    return 0;
}

int klist_get(klist *self, int pos, void **el) {
    struct klist_iter list_iter;
    kiter *iter = (kiter *)&list_iter;

    klist_iter_init(&list_iter, self);
    for (; pos >= 0; pos --) {
        if (kiter_next(iter, el)) {
            KTOOLS_ERROR_SET("the list is too short");
            return -1;
        }
    }
    return 0;
}

/*****************
 * Klist iterator
 *****************/

static void klist_iter_begin(kiter *iter);
static int klist_iter_prev(kiter *iter);
static int klist_iter_next(kiter *iter);
static void klist_iter_end(kiter *iter);
static int klist_iter_get(kiter *iter, void **el);
static int klist_iter_remove(kiter *iter);
static int klist_iter_insert(kiter *iter, void *el);
static int klist_iter_insert_after(kiter *iter, void *el);
static int klist_iter_change(kiter *iter, void *el);
//static struct kiter *klist_iter_copy(kiter *iter);

static struct kiter_ops klist_iter_ops = {
    klist_iter_begin,
    klist_iter_prev,
    klist_iter_next,
    klist_iter_end,
    klist_iter_get,
    klist_iter_remove,
    klist_iter_insert,
    klist_iter_insert_after,
    klist_iter_change,
};

/*static inline int klist_iter_is_root(kiter *iter) {
    struct klist_iter *self = (struct klist_iter *)iter;
    if (self->node == self->list->root) {
        return 1;
    }
    return 0;
}*/

static void klist_iter_begin(kiter *iter) {
    struct klist_iter *self = (struct klist_iter *)iter;
    self->node = self->list->head;
}

static int klist_iter_prev(kiter *iter) {
    struct klist_iter *self = (struct klist_iter *)iter;
    self->node = self->node->prev;
    if (self->node == self->list->head)
        return -1;
    return 0;
}

static int klist_iter_next(kiter *iter) {
    struct klist_iter *self = (struct klist_iter *)iter;
    self->node = self->node->next;
    if (self->node == self->list->tail)
        return 1;
    return 0;
}

static void klist_iter_end(kiter *iter) {
    struct klist_iter *self = (struct klist_iter *)iter;
    self->node = self->list->tail;
}

static int klist_iter_get(kiter *iter, void **el) {
    struct klist_iter *self = (struct klist_iter *)iter;
    int ret;
    if (self->node == self->list->head)
            ret = -1;
    else if (self->node == self->list->tail)
            ret = 1;
    else
            ret = 0;

    *el = self->node->data;
    return ret;
}

static int klist_iter_remove(kiter *iter) {
    struct klist_iter *self = (struct klist_iter *)iter;
    struct klist_node *prev;
    struct klist_node *next;
    if (self->node == self->list->head)
        return -1;
    if (self->node == self->list->tail)
        return 1;

    prev = self->node->prev;
    next = self->node->next;
    prev->next = next;
    next->prev = prev;

    klist_node_destroy(self->node);
    self->list->length--;

    self->node = next;

    return 0;
}

static int klist_iter_insert(kiter *iter, void *el) {
    struct klist_iter *self = (struct klist_iter *)iter;
    struct klist_node *node;

    if (self->node == self->list->head) {
        return -1;
    }

    node = klist_node_new(el);
    node->prev = self->node->prev;
    node->next = self->node;
    node->prev->next = node;
    node->next->prev = node;

    return 0;
}

static int klist_iter_insert_after(kiter *iter, void *el) {
    struct klist_iter *self = (struct klist_iter *)iter;
    struct klist_node *node;

    if (self->node == self->list->tail) {
        return -1;
    }

    node = klist_node_new(el);
    node->prev = self->node;
    node->next = self->node->next;
    node->prev->next = node;
    node->next->prev = node;

    return 0;
}

static int klist_iter_change(kiter *iter, void *el) {
    struct klist_iter *self = (struct klist_iter *)iter;
    int ret;
    if (self->node == self->list->head)
        ret = -1;
    else if (self->node == self->list->tail)
        ret = 1;
    else {
        self->node->data = el;
        ret = 0;
    }
    return ret;
}

#if 0
/* TODO: insert the iterator in a iterator hash table in the list. */
static struct kiter *klist_iter_copy(kiter *iter) {
    struct kiter *copy = kmalloc(sizeof(struct klist_iter));
    memcpy(copy, iter, sizeof(struct klist_iter));
    return copy;
}

/* TODO: remove the iterator from the iterator hash table in the array. Do not use for now. */
void klist_iter_destroy(struct kiter *self) {
    kfree(self);
}

#endif

/* TODO: insert the iterator in an iterator hash table in the list. This would be useful if we remove the item this iterator is pointing to from the list interface of another kiter on the same list. This way we could advance de iterator before removing it. */
void klist_iter_init(struct klist_iter *self, klist *list) {
    self->list = list;
    self->node = self->list->head;
    kiter_init((kiter *)self, &klist_iter_ops);
}
