#include <kernel/console.h>
#include <kernel/kmemory.h>
#include <kernel/page.h>
#include <stddef.h>

// TODO: page mapping

struct bucket_desc {
    void *page;
    void *free_ptr;
    unsigned short size;
    unsigned short refcnt;
    struct bucket_desc *next;
};

#define DIRS 8
struct bucket_desc *bucket_directory[DIRS] = {NULL};

struct bucket_desc *free_buckets = NULL;

void init_malloc() {
    bucket_directory[0] = (struct bucket_desc *)NULL;
    bucket_directory[1] = (struct bucket_desc *)NULL;
    bucket_directory[2] = (struct bucket_desc *)NULL;
    bucket_directory[3] = (struct bucket_desc *)NULL;
    bucket_directory[4] = (struct bucket_desc *)NULL;
    bucket_directory[5] = (struct bucket_desc *)NULL;
    bucket_directory[6] = (struct bucket_desc *)NULL;
    bucket_directory[7] = (struct bucket_desc *)NULL;
}

#define R_DIR_NR(size)                                                         \
    ((size) <= 32                                                              \
         ? 1                                                                   \
         : (size) <= 64                                                        \
               ? 2                                                             \
               : (size) <= 128                                                 \
                     ? 3                                                       \
                     : (size) <= 256                                           \
                           ? 4                                                 \
                           : (size) <= 512                                     \
                                 ? 5                                           \
                                 : (size) <= 1024 ? 6                          \
                                                  : (size) <= 2048 ? 7 : 8)

#define W_DIR_NR(size)                                                         \
    ((size) < 32                                                               \
         ? 0                                                                   \
         : (size) < 64                                                         \
               ? 1                                                             \
               : (size) < 128                                                  \
                     ? 2                                                       \
                     : (size) < 256                                            \
                           ? 3                                                 \
                           : (size) < 512                                      \
                                 ? 4                                           \
                                 : (size) < 1024 ? 5 : (size) < 2048 ? 6 : 7)

static struct bucket_desc *find_bucket(int size) {
    if (size > 4096)
        return NULL;
    struct bucket_desc *bucket;
    int dir_id = R_DIR_NR(size);
    if (dir_id >= DIRS)
        return NULL;
    for (; dir_id < DIRS; dir_id++) {
        bucket = bucket_directory[dir_id];
        if (bucket) {
            break;
        }
    }
    return bucket;
}

static void expand_free_buckets() {
    if (free_buckets)
        return;
    struct bucket_desc *page = (struct bucket_desc *)kalloc_page();
    bind_page(page,
              page); // TODO: when multiprocess happens, this should change
    for (int i = 0; i < (int)(PAGE_SIZE / sizeof(struct bucket_desc)); i++) {
        struct bucket_desc *bucket = page + i;
        bucket->free_ptr           = NULL;
        bucket->size               = 0;
        bucket->page               = NULL;
        bucket->refcnt             = 0;
        bucket->next               = bucket + 1;
    }
    (page + PAGE_SIZE / sizeof(struct bucket_desc) - 1)->next = free_buckets;
    free_buckets                                              = page;
}

static void remove_from_directory(struct bucket_desc *bucket) {
    if (!bucket)
        return;
    int dir_id               = W_DIR_NR(bucket->size);
    struct bucket_desc *head = bucket_directory[dir_id];
    if (head == bucket) {
        bucket_directory[dir_id] = bucket->next;
    } else {
        for (; head->next; head = head->next) {
            if (head->next == bucket)
                head->next = bucket->next;
        }
    }
}

static void insert_into_directory(struct bucket_desc *bucket) {
    int dir_id               = W_DIR_NR(bucket->size);
    bucket->next             = bucket_directory[dir_id];
    bucket_directory[dir_id] = bucket;
}

static struct bucket_desc *create_bucket() {
    if (!free_buckets)
        expand_free_buckets();
    struct bucket_desc *bucket = free_buckets;
    free_buckets               = free_buckets->next;
    bucket->page = bucket->free_ptr = kalloc_page();
    bind_page(bucket->page, bucket->page);
    bucket->size   = PAGE_SIZE;
    bucket->next   = NULL;
    bucket->refcnt = 0;
    return bucket;
}

static void release_bucket(struct bucket_desc *bucket) {
    unbind_page(bucket->page);
    kfree_page(bucket->page);
    bucket->page = bucket->free_ptr = NULL;
    bucket->refcnt                  = 0;
    bucket->size                    = 0;
    bucket->next                    = free_buckets;
    free_buckets                    = bucket;
}

void *malloc(size_t size) {
    struct bucket_desc *bucket;
    bucket = find_bucket(size);
    if (bucket) {
        remove_from_directory(bucket);
    } else {
        bucket = create_bucket();
    }
    bucket->size -= size;
    void *ptr = bucket->free_ptr;
    bucket->free_ptr += size;
    bucket->refcnt++;
    insert_into_directory(bucket);
    return ptr;
}

void free(void *ptr) {
    int success = 0;
    for (int i = 0; i < DIRS; i++) {
        for (struct bucket_desc *desc = bucket_directory[i]; desc;
             desc                     = desc->next) {
            if (desc->page <= ptr && desc->free_ptr >= ptr) {
                desc->refcnt--;
                if (!desc->refcnt) {
                    remove_from_directory(desc);
                    release_bucket(desc);
                }
                success = 1;
                break;
            }
        }
    }
    if (!success)
        panic("Unable to free a pointer");
}
