#include "kutils.h"
#define __BUILDING_KITER
#include "kiter.h"
#include"kerror.h"

/* Internal get */
static int kiter_fetch(struct kiter *self, void **el) {
    int ret = self->ops->get(self, &self->current);
    if (ret == 0)
        self->status = KITER_CUR;
    else if (ret < 0)
        self->status = KITER_BEG;
    else
        self->status = KITER_END;

    if (el)
        *el = self->current;

    return ret;
}

/* Called when by the implementer to initialize the iterator (super class). */
void kiter_init(struct kiter *self, struct kiter_ops *ops) {
    self->ops = ops;
    self->status = KITER_BEG;
    self->current = NULL;
}

/* Sets the current element as beiing the begining, a call to next will place
 * the iterator on the first element if the structure is not empty. */
void kiter_begin(struct kiter *self) {
    self->ops->begin(self);
    self->status = KITER_BEG;
    self->current = NULL;
}

/* Goto the previous element then get it. It returns -1 if it sets the current
 * to the begining. */
int kiter_prev(struct kiter *self, void **el) {
    if (self->status == KITER_BEG || self->ops->prev(self)) {
        self->status = KITER_BEG;
        self->current = NULL;
        return -1;
    }

    self->status = KITER_CUR;

    return kiter_fetch(self, el);
}

/* Goto the next element then get it. It returns 1 if it sets the current to
 * the end. */
int kiter_next(struct kiter *self, void **el) {
    if (self->status == KITER_END || self->ops->next(self)) {
        self->status = KITER_END;
        self->current = NULL;
        return 1;
    }

    self->status = KITER_CUR;

    return kiter_fetch(self, el);
}

/* Set the current element as beiing after the last. A call to prev will set
 * the current element as beiing the last if the structure is not empty. */
void kiter_end(struct kiter *self) {
    self->ops->end(self);
    self->status = KITER_END;
    self->current = NULL;
}

/* Returns the value of the current node. Returns -1 if we are at start and 1
 * if we are at end. */
int kiter_get(struct kiter *self, void **el) {
    if (el)
        *el = self->current;

    switch (self->status) {
        case KITER_BEG:
            return -1;
        case KITER_CUR:
            return 0;
        case KITER_END:
            return 1;
    }
    return -1;
}

/* Remove the current node, returns -1 if we are at start and 1 if we are at
 * end. */
int kiter_remove(struct kiter *self, void **el) {
    int ret;
    enum kiter_status status = self->status;

    if (el)
        *el = self->current;

    ret = self->ops->remove(self);

    if (ret) {
    	/* FD reports a bug in klist_iter_remove, I don't know what it is and I
	 * don't have time to investigate. The bug messes out the error context,
	 * so I'm commenting this out for the time being. FIXME.
	 */
        /*if (ret < 0) {
            KTOOLS_ERROR_SET("cannot remove the item at beginning");
        } else if (ret > 0) {
            KTOOLS_ERROR_SET("cannot remove the item at end");
        }*/
        return ret;
    }

    

    kiter_fetch(self, NULL);

    switch (status) {
        case KITER_BEG:
            return -1;
        case KITER_CUR:
            return 0;
        case KITER_END:
            return 1;
    }
    return -1;
}

/* Insert a node before the current one. It returns -1 if we are at the start.
 */
int kiter_insert(struct kiter *self, void *el) {
    if (self->status == KITER_BEG) {
        KTOOLS_ERROR_SET("cannot insert an item before beginning");
        return -1;
    }

    return self->ops->insert(self, el);
}

/* Insert a node after the current one. It returns 1 if we are at the end. */
int kiter_insert_after(struct kiter *self, void *el) {
    if (self->status == KITER_END) {
        KTOOLS_ERROR_SET("cannot insert an item after end");
        return -1;
    }

    return self->ops->insert_after(self, el);
}

/* Replace the current node element by a new one. It returns -1 when on start
 * and 1 when on end. */
int kiter_replace(struct kiter *self, void **orig_el, void *new_el) {
    int ret;
    if (orig_el)
        *orig_el = self->current;
    ret = self->ops->change(self, new_el);

    if (ret < 0) {
        KTOOLS_ERROR_SET("cannot remove the item at beginning");
    } else if (ret > 0) {
        KTOOLS_ERROR_SET("cannot remove the item at end");
    }

    return ret;
}

#if 0 /* Add if needed */
struct kiter *kiter_copy(kiter *self) {
    return self->ops->copy(self);
}

void kiter_destroy(kiter *self) {
    self->ops->destroy(self);
}
#endif
