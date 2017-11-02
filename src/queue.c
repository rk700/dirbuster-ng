#include "queue.h"

struct queue *queue_add(struct queue *s, char* str) {
    if (NULL == s) {
        printf("Queue not initialized\n");
        return s;
    }

    struct elt *p = malloc(1 * sizeof(struct elt));

    if (NULL == p) {
        fprintf(stderr, "IN %s, %s: malloc() failed\n", __FILE__, "list_add");
        return s;
    }

    // p->entry  = (char*) calloc (0,(strlen(str) +1) * sizeof(char));
    p->entry = strdup(str);
    if (p->entry == NULL) {
        fprintf(stderr, "strdup failed when adding entry %s\n", str);
        free(p);
        return s;
    }
    p->next = NULL;

    if (NULL == s->head && NULL == s->tail) {
       s->head = s->tail = p;
       return s;
    } 
    else {
       s->tail->next = p;
       s->tail = p;
    }
    
    return s;
}

struct queue *queue_rem(struct queue *s) {
    struct elt *h = NULL;
    struct elt *p = NULL;

    h = s->head;
    p = h->next;
    free(h->entry);
    free(h);
    s->head = p;
    if (NULL == s->head) {
        s->tail = NULL;
    }
    return s;
}

struct queue *queue_free(struct queue *s) {
    while (s->head) {
        queue_rem(s);
    }
    return s;
}

struct queue *queue_new(void) {
    struct queue *p = malloc(1 * sizeof(*p));

    if (NULL == p) {
        fprintf(stderr, "LINE: %d, malloc() failed\n", __LINE__);
    }
    p->head = p->tail = NULL;
    return p;
}
