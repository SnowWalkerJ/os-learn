#include <stddef.h>
#include <kernel/kmemory.h>
#include <kernel/console.h>


// TODO: page mapping


struct bucket_desc {
    void* page;
    void* free_ptr;
    unsigned short size;
    unsigned short refcnt;
    struct bucket_desc* next;
};


struct bucket_dir {
    int size;
    struct bucket_desc* head;
};


struct bucket_dir bucket_directory[9];

struct bucket_desc* free_buckets = NULL;


void init_malloc() {
    bucket_directory[0].size = 32;
    bucket_directory[0].head = (struct bucket_desc*)NULL;
    bucket_directory[1].size = 64;
    bucket_directory[1].head = (struct bucket_desc*)NULL;
    bucket_directory[2].size = 128;
    bucket_directory[2].head = (struct bucket_desc*)NULL;
    bucket_directory[3].size = 256;
    bucket_directory[3].head = (struct bucket_desc*)NULL;
    bucket_directory[4].size = 512;
    bucket_directory[4].head = (struct bucket_desc*)NULL;
    bucket_directory[5].size = 1024;
    bucket_directory[5].head = (struct bucket_desc*)NULL;
    bucket_directory[6].size = 2048;
    bucket_directory[6].head = (struct bucket_desc*)NULL;
    bucket_directory[7].size = 4096;
    bucket_directory[7].head = (struct bucket_desc*)NULL;
    bucket_directory[8].size = 0;
    bucket_directory[8].head = (struct bucket_desc*)NULL;
}


#define DIR_NR(size) (size) <= 32 ? 0 : \
                        (size) <= 64 ? 1 : \
                            (size) <= 128 ? 2: \
                                (size) <= 256 ? 3: \
                                    (size) <= 512 ? 4 : \
                                        (size) <= 1024 ? 5 : \
                                            (size) <= 2048 ? 6 : 7


struct bucket_desc* find_bucket(int size) {
    if (size > 4096) return NULL;
    struct bucket_desc* bucket;
    int dir_id = DIR_NR(size);
    for(; bucket_directory[dir_id].size; dir_id++) {
        bucket = bucket_directory[dir_id].head;
        if (bucket) {
            break;
        }
    }
    return bucket;
}


void expand_free_buckets() {
    if (free_buckets) return;
    void* page = kalloc_page();
    for (int i = 0; i < (int)(PAGE_SIZE / sizeof(struct bucket_desc)); i++) {
        struct bucket_desc* bucket = (struct bucket_desc*)page + i;
        bucket->free_ptr = NULL;
        bucket->size = 0;
        bucket->page = NULL;
        bucket->refcnt = 0;
        bucket->next = bucket+1;
    }
    ((struct bucket_desc*)page + PAGE_SIZE / sizeof(struct bucket_desc))->next = free_buckets;
    free_buckets = (struct bucket_desc*)page;
}


void remove_from_directory(struct bucket_desc* bucket) {
    if (!bucket) return;
    int dir_id = DIR_NR(bucket->size);
    struct bucket_desc* head = bucket_directory[dir_id].head;
    if (head == bucket) {
        bucket_directory[dir_id].head = bucket->next;
    } else {
        for (; head->next; head = head->next) {
            if (head->next == bucket)
                head->next = bucket->next;
        }
    }
}


void insert_into_directory(struct bucket_desc* bucket) {
    int dir_id = DIR_NR(bucket->size) - 1;
    if (dir_id < 0) return;
    bucket->next = bucket_directory[dir_id].head;
    bucket_directory[dir_id].head = bucket;
}


struct bucket_desc* create_bucket() {
    if (!free_buckets)
        expand_free_buckets();
    struct bucket_desc* bucket = free_buckets;
    free_buckets = free_buckets->next;
    bucket->page = bucket->free_ptr = kalloc_page();
    bucket->size = PAGE_SIZE;
    bucket->next = NULL;
    bucket->refcnt = 0;
    return bucket;
}


void release_bucket(struct bucket_desc* bucket) {
    kfree_page(bucket->page);
    bucket->page = bucket->free_ptr = NULL;
    bucket->refcnt = 0;
    bucket->size = 0;
    bucket->next = free_buckets;
    free_buckets = bucket;
}


void* malloc(size_t size) {
    struct bucket_desc* bucket;
    bucket = find_bucket(size);
    if (bucket) {
        remove_from_directory(bucket);
    } else {
        bucket = create_bucket();
    }
    bucket->size -= size;
    void* ptr = bucket->free_ptr;
    bucket->free_ptr += size;
    bucket->refcnt++;
    insert_into_directory(bucket);
    return ptr;
}


void free(void* ptr) {
    int success = 0;
    for (struct bucket_dir* dir = bucket_directory; dir->size; dir++) {
        for (struct bucket_desc* desc = dir->head; desc; desc = desc->next) {
            if (desc->page <= ptr && desc->free_ptr >= ptr) {
                desc->refcnt--;
                if (!desc->refcnt) {
                    release_bucket(desc);
                }
                success = 1;
                break;
            }
        }
    }
    if (!success) panic("Unable to free a pointer");
}
