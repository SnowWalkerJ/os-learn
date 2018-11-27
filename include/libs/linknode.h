#ifndef LINKNODE_H
#define LINKNODE_H
#include <kernel/memory.h>
#include <stddef.h>
#include <stdint.h>

struct linknode {
    void *value;
    struct linknode *next;
};

void linknode_free(struct linknode *link);

struct linknode *linknode_tail(struct linknode *link);

void linknode_append(struct linknode *link, void *val);

#endif /* LINK_NODE_H */