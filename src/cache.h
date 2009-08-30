/* cache.h */
/* see cache.c for annotations */

#ifndef GUARD_CACHE_H
#define GUARD_CACHE_H

/* defines */
#define CACHE_SIZE 10000

/* data types */
typedef struct {
    int tname;
    int sidelength;
    float * tdata;
} cache_entry;

/* functions */
void cache_refresh_all(void);
void cache_refresh_entry(int tname);
cache_entry * cache_create_entry(int texture_name);
cache_entry * find_in_cache(int texture_name);
cache_entry * find_or_create_cache_entry(int texture_name);

#endif
