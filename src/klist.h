/**
 * kmo/list.h
 * Copyright (C) 2005-2012 Opersys inc., All rights reserved.
 *
 * List utilities.
 *
 * @author François-Denis Gonthier
 */

#ifndef __K_LIST_H__
#define __K_LIST_H__

#include <kiter.h>

struct klist_node {
    struct klist_node *prev;
    struct klist_node *next;
    void * data;
};

typedef struct klist {
    struct klist_node *head;
    struct klist_node *tail;
    int length;
} klist;

klist *klist_new();

void klist_init(klist *self);
void klist_init_klist(klist *self, klist *init_list);
void klist_clean(klist *self);

void klist_destroy(klist *self);

void klist_reset(klist *self);

void klist_prepend(klist *self, void *data);
void klist_append(klist *self, void *data);

int klist_rm_head(klist *self, void **el);
int klist_rm_tail(klist *self, void **el);

int klist_head(klist *self, void **el);
int klist_tail(klist *self, void **el);
int klist_get(klist *self, int pos, void **el);

// ????????????????
//int list_remove_item_by_ptr(llist *, void *);

struct klist_iter {
    kiter iter;
    klist *list;
    struct klist_node *node;
};

void klist_iter_init(struct klist_iter *self, klist *list);

#endif // __K_LIST_H__
