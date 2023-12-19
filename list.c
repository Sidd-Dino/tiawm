// Circular Doubly Linked List

#include <stdlib.h>
#include "list.h"

void
free_ws(void *d) {
        ws *data = (ws *) d;
        free(data->wlist);
        free(data);
}

void
free_win(void *d) {
        win *data = (win *) d;
        data->ws = NULL;
        free(data);
}

void
mvnode(node **position, node *target) {
        if (NULL == position || NULL == target) return;

        if (*position == target) return;

        if (NULL != target->next) target->next->prev = target->prev;
        if (NULL != target->prev) target->prev->next = target->next;

        target->prev = target->next = target;

        if (NULL != *position) {
            target->next       = (*position);
            target->prev       = (*position)->prev;
            target->next->prev = target;
            target->prev->next = target;
        }

        *position = target;
}

node *
mknode(node **position, void *data) {
        node * new;
        if (!position) return NULL;

        new = (node *) malloc(sizeof(node));
        if (!new) return NULL;

        mvnode(position, new);
        new->data = data;

        return new;
}

void *
rmnode(node **list, node *del) {
        void * val;

        if (NULL == list || NULL == *list || NULL == del) return NULL;

        if (del->prev == del->next && del == del->next ) (*list) = NULL;
        else {
                del->next->prev = del->prev;
                del->prev->next = del->next;
        }

        (*list) = (*list == del) ? del->next : (*list);
        del->next = del->prev = NULL;

        val = del->data;

        free(del);

        return val;
}

void
rmlist(node **list, void (*rmdata)(void *)) {
        while(*list) {
                rmdata((*list)->data);
                rmnode(list, *list);
        }
}

// vim:cc=81 ts=8 sts=8 sw=8:
