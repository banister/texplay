#ifndef GUARD_GRAPHICS_UTILS_H
#define GUARD_GRAPHICS_UTILS_H

void update_lazy_bounds(action_struct * cur, texture_info * tex);
void update_bounds(action_struct * cur, int xmin, int ymin, int xmax, int ymax);
void set_local_bounds(action_struct * cur, int xmin, int ymin, int xmax, int ymax, texture_info * tex);

void draw_prologue(action_struct * cur, texture_info * tex, int xmin, int ymin, int xmax, int ymax, VALUE * hash_arg,   
                                                                      sync sync_mode, bool primary, action_struct ** payload_ptr);
void draw_epilogue(action_struct * cur, texture_info * tex, bool primary);

void set_pixel_color_with_style(action_struct * payload, texture_info * tex, int x, int y);
void set_pixel_color(rgba * pixel_color, texture_info * tex, int x, int y);

rgba get_pixel_color_from_chunk(float * chunk, int width, int height, int x, int y);
rgba get_pixel_color(texture_info * tex, int x, int y);
float* get_pixel_data(texture_info * tex, int x, int y);

/* create a blank gosu image of width and height */
VALUE create_image(VALUE window, int width, int height);

typedef struct {
    float * data;
} RawImageData;

static inline VALUE
wrap_raw_image(float * image_data) {
    RawImageData * c_data;
    VALUE ret = Data_Make_Struct(cImageData, RawImageData, 0, NULL, c_data);
    c_data->data = image_data;
    return ret;    
}


#endif
