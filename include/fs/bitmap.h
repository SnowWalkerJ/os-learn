struct bitmap {
    int size;   // size in bytes
    char* map;
};

void empty_bitmap(struct bitmap*);
void set_one(struct bitmap*, unsigned int);
void set_zero(struct bitmap*, unsigned int);
unsigned int first_zero(struct bitmap*);
