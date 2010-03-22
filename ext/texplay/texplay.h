/* texplay.h (C) John Mair 2008 
 * This program is distributed under the terms of the MIT License
 *   
 *            
 *                                                   */

#ifndef GUARD_TEXPLAY_H
#define GUARD_TEXPLAY_H

#include <ruby.h>

/* #defines */
#define OOB_VAL 9999
#define XMAX_OOB OOB_VAL
#define YMAX_OOB OOB_VAL
#define XMIN_OOB -OOB_VAL
#define YMIN_OOB -OOB_VAL

#define PI 3.14159265358979

/* macros */
#define SWAP(X, Y)  {(X) ^= (Y); (Y) ^= (X); (X) ^= (Y);} 
#define ROUND(X) (int)((X) + 0.5)
#define ARY_SIZE(X) sizeof(X) / sizeof(*X)
#define SGN(X) ((X) >= 0 ? 1 : -1)
#define MAX(X, Y) ((X) > (Y)) ? (X) :(Y)
#define MIN(X, Y) ((X) < (Y)) ? (X) : (Y)
#define ABS(X) ((X) >= 0 ? (X) : -(X))

/* enums */
typedef enum e_bool {
    false, true
} bool;

typedef enum e_color {
    red, green, blue, alpha
} color_t;

typedef enum e_sync_mode {
    lazy_sync, eager_sync, no_sync
} sync;

typedef enum {
  clear, copy, noop,
  set, copy_inverted,
  invert, and_reverse, and,
  or, nand, nor, xor,
  equiv, and_inverted,
  or_inverted, additive,
  multiply, screen, overlay,
  darken, lighten, colordodge,
  colorburn, hardlight, softlight,
  difference, exclusion
} draw_mode;

/* structs */
typedef struct s_rgba {
    float red, green, blue, alpha;
} rgba;

/* stores image data */
typedef struct {
    int width, height;    
    float top, left;    
    int tname;
    float * td_array;
    int yincr, firstpixel;
    int x_offset, y_offset;
    VALUE image;
} texture_info;


/* convenience macro */
#define IMAGE_BOUNDS(X) ((image_bounds *) (X))
typedef struct {
    int xmin;
    int ymin;
    int xmax;
    int ymax;
} image_bounds;


typedef struct action_struct {
    int xmin, ymin, xmax, ymax;
    sync sync_mode;

    /* pointer to associated texture */
    /* a bit of a kludge having this here
       since it's only being used by convert_image_local_color_to_rgba */
    texture_info * tex;
    
    VALUE hash_arg;

    /* action color */
    rgba color;

    /* pen data */
    struct {

        /* color control, dynamic */
        bool has_color_control_proc;
        VALUE color_control_proc;
        int color_control_arity;

        /* color control, static */
        bool has_color_control_transform;
        rgba color_mult;
        rgba color_add;

        /* texture fill */
        bool has_source_texture;
        texture_info source_tex;

        /* lerp */
        bool has_lerp;
        float lerp;

        /* alpha blend */
        bool alpha_blend;

        /* drawing mode */
        bool has_drawing_mode;
        draw_mode drawing_mode;
      
    } pen;

} action_struct;


#endif
