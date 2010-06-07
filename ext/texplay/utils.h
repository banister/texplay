#ifndef GUARD_UTILS_H
#define GUARD_UTILS_H


/* for use with get_image_local, and set_image_local */
#define DRAW_OFFSET 0
#define LAZY_BOUNDS 1
#define IMAGE_COLOR 2
#define USER_DEFAULTS 3 

/* shared global */
extern const rgba not_a_color_v;

/* color and pixel related functions */
rgba find_color_from_string(char * try_color);
bool cmp_color(rgba color1, rgba color2);
void color_copy(float * source, float * dest);
rgba get_pixel_color(texture_info * tex, int x, int y) ;
float* get_pixel_data(texture_info * tex, int x, int y);
void set_pixel_color(rgba * pixel_color, texture_info * tex, int x, int y);
void zero_color(float * tex);
bool not_a_color(rgba color1);
bool is_a_color(rgba color1);
VALUE convert_rgba_to_rb_color(rgba * pix);
rgba convert_rb_color_to_rgba(VALUE cval);
bool is_gosu_color(VALUE try_color);
bool is_rb_raw_color(VALUE cval);
bool not_rb_raw_color(VALUE cval);
rgba convert_gosu_to_rgba_color(VALUE gcolor);

VALUE save_rgba_to_image_local_color(VALUE image, rgba color);
rgba convert_image_local_color_to_rgba(VALUE image);


/* string/symbol related functions */
char* lowercase(char*);
char* sym2string(VALUE);
VALUE string2sym(char*);

/* is it a hash? */
bool is_a_hash(VALUE try_hash);

/* is an array? */
bool is_an_array(VALUE try_array);

/* is a num? */
bool is_a_num(VALUE try_num);

/* helpful C wrapper for rb_hash_aref */
VALUE get_from_hash(VALUE hash, char * sym);

/* a wrapper for rb_hash_aset */
VALUE set_hash_value(VALUE hash, char * sym, VALUE val);

/* a wrapper for rb_hash_delete */
VALUE delete_from_hash(VALUE hash, char * sym);

/* does the hash key 'sym' map to value 'val'? */
bool hash_value_is(VALUE hash, char * sym, VALUE val);

/* determine whether (1) hash is a T_HASH (2) the sym exists in the hash */
bool has_optional_hash_arg(VALUE hash, char * sym);


/* helpful C wrapper for rb_array_store */
VALUE set_array_value(VALUE array, int index, VALUE val);

/* helpful C wrapper for rb_array_entry */
VALUE get_from_array(VALUE array, int index);

/* set the lazy_bounds ivar for an image */
VALUE set_ivar_array(VALUE obj, char * ivar, int argc, ...);

/* initialize lazy_bounds */
void init_lazy_bounds(VALUE image);

/* initialize the image_local var array */
VALUE init_image_local(VALUE image);

/* return an image local variable */
VALUE get_image_local(VALUE image, int name);

/* set an image local variable */
void set_image_local(VALUE image, int name, VALUE val);

/* error checking functions */
void check_mask(VALUE mask);

void check_image(VALUE image);

/* is try_image a Gosu::Image ? */
bool is_gosu_image(VALUE try_image);


/* make boundaries sane */
void constrain_boundaries(int * x0, int * y0, int * x1, int * y1, int width, int height);

/* line clipping */
void cohen_sutherland_clip (int * x0, int * y0,int * x1, int * y1, int xmin, int ymin,
                            int xmax, int ymax);

/* check if point is bound by rect and inner thickness */
bool bound_by_rect_and_inner(int x, int y, int x0, int y0, int x1, int y1, int inner);

/* check if point is bound by rect */
bool bound_by_rect(int x, int y, int x0, int y0, int x1, int y1);

/* calculate the array offset for a given pixel */
int calc_pixel_offset(texture_info * tex, int x, int y);

/* calculate the array offset for a given pixel in an action context */
int calc_pixel_offset_for_action(action_struct * cur, texture_info * tex, int x, int y);

/* return a pointer to a new texture */
float* allocate_texture(int width, int height);

/* other function prototypes, get info for a texture */
void get_texture_info(VALUE image, texture_info * tex);

/* ensure gl_tex_info returns non nil */
VALUE check_for_texture_info(VALUE image);

/* responsible for syncing a subimage to gl */
void sync_to_gl(int tex_name, int x_offset, int y_offset, int width, int height, void * sub);

/* create a subtexture and sync to gl */
void create_subtexture_and_sync_to_gl(image_bounds * img_bounds, texture_info * tex);

float * get_image_chunk(texture_info * tex, int xmin, int ymin, int xmax, int ymax);

rgba get_pixel_color_from_chunk(float * chunk, int width, int height, int x, int y);

/* from Texture.cpp in Gosu Graphics folder */
unsigned max_quad_size(void);

/* point utils */
bool is_a_point(VALUE try_point);
VALUE point_x(VALUE point);
VALUE point_y(VALUE point);

/* mathematical utils, used by bezier curves */
double power(float base, int exp);
unsigned fact(int n);
unsigned comb(int n, int k);
unsigned perm(int n, int r);

double bernstein(int n, int k, float u);

#endif
