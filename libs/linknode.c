#include <libs/linknode.h>


void linknode_free(struct linknode* link) {
    if (link == NULL)
        return;
    linknode_free(link->next);
    free((void*)link->value);
    free(link);
}


struct linknode* linknode_tail(struct linknode* link) {
    if (link == NULL)
        return NULL;
    while (link->next) {
        link = link->next;
    }
    return link;
}


void linknode_append(struct linknode* link, void* val) {
    link = linknode_tail(link);
    struct linknode* new_link = (struct linknode*)malloc(sizeof(struct linknode));
    new_link->next = NULL;
    new_link->value = val;
    link->next = new_link;
}
