/* cache.c */

#include <ruby.h>

#ifdef __APPLE__
#include <glut.h>
#else
#include <GL/glut.h>
#endif

#include "cache.h"
#include "texplay.h"

/* var has internal linkage, static duration */
/* contains cache data */
static cache_t cache = {0}; 

/* create a new cache entry */
cache_entry*
cache_create_entry(int tname) {
    float * new_array;
    int sidelength, new_element = cache.len;
    GLint saved_tname;

    if(cache.len >= CACHE_SIZE) {  rb_raise(rb_eRuntimeError, "cache is full! increase CACHE_SIZE");  }

    /* save current texture binding */
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &saved_tname);

    /* opengl initialization code */
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tname);

    /* get length of a side, since square texture */
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &sidelength);

    /* initialize texture data array, mult. by 4 because {rgba} */
    new_array = malloc(sidelength * sidelength * 4 * sizeof(float));

    /* get texture data from video memory */
    glGetTexImage(GL_TEXTURE_2D,  0, GL_RGBA, GL_FLOAT,(void*)(new_array));

    /* save texture data in the cache */
    cache.entry[new_element].tname = tname;
    cache.entry[new_element].sidelength = sidelength;
    cache.entry[new_element].tdata = new_array;

    /* update size of cache */
    cache.len++;

    /* restore saved texture binding */
    glBindTexture(GL_TEXTURE_2D, saved_tname);

    return &cache.entry[new_element];
}

/* return the entry if it exists, otherwise return NULL */
cache_entry*
find_in_cache(int tname) {
    /* check if entry exists in cache */
    int index;
    for(index = 0; index < cache.len; index++) 
        if(cache.entry[index].tname == tname) 
            return &cache.entry[index];
    
    
    return NULL;
}

/* if the entry doesn't exist then create it and return it.
   otherwise just return it */
cache_entry*
find_or_create_cache_entry(int tname) {
    cache_entry * entry;

    if((entry=find_in_cache(tname)))
        return entry;
    else
        return cache_create_entry(tname);
}

/* refresh the cache for all quads */
void
cache_refresh_all(void) {
    float * tdata;
    int tname, index;
    GLint saved_tname;

    /* save current texture binding */
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &saved_tname);

    for(index = 0; index < cache.len; index++) {
        tdata = cache.entry[index].tdata;
        tname = cache.entry[index].tname;

        /* opengl initialization code */
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tname);

        glGetTexImage(GL_TEXTURE_2D,  0, GL_RGBA, GL_FLOAT, (void*)(tdata));
    }

    /* restore saved texture binding */
    glBindTexture(GL_TEXTURE_2D, saved_tname);
}

/* refresh the cache for a specific quad */
void
cache_refresh_entry(int tname) {
    GLint saved_tname;
    cache_entry * entry;

    /* save current texture binding */
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &saved_tname);

    entry = find_in_cache(tname);
    
    /* opengl initialization code */
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, entry->tname);

    glGetTexImage(GL_TEXTURE_2D,  0, GL_RGBA, GL_FLOAT, (void*)(entry->tdata));

    /* restore saved texture binding */
    glBindTexture(GL_TEXTURE_2D, saved_tname);
}

