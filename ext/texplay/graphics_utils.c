#include "texplay.h"
#include "utils.h"
#include "graphics_utils.h"
#include "cache.h"

#ifdef __APPLE__
#include <glut.h>
#else
#include <GL/glut.h>
#endif


/* small helper functions */
static void initialize_action_struct(action_struct * cur, VALUE hash_arg, sync sync_mode);
static void process_common_hash_args(action_struct * cur, VALUE * hash_arg, sync sync_mode, bool primary);
static void prepare_drawing_mode(action_struct * cur);
static void prepare_fill_texture(action_struct * cur);
static void prepare_color_control(action_struct * cur);
static rgba apply_lerp(action_struct * payload, texture_info * tex, int x, int y);
static rgba apply_alpha_blend(action_struct * payload, texture_info * tex, int x, int y, rgba blended_pixel);
static rgba apply_drawing_mode(action_struct * payload, texture_info * tex, int x, int y);

static rgba apply_color_control_transform(action_struct * payload, texture_info * tex, int x, int y);
static rgba exec_color_control_proc(action_struct * cur, texture_info * tex, int x, int y, rgba blended_pixel);
/* end helpers */


void
update_lazy_bounds(action_struct * cur, texture_info * tex)
{
    
    /* only update global bounds if we're doing a lazy_sync */
    if(cur->sync_mode == lazy_sync) {
        int xmin, ymin, xmax, ymax;
        VALUE lazy_bounds;

        lazy_bounds = get_image_local(tex->image, LAZY_BOUNDS);

        xmin = INT2FIX(MIN(cur->xmin, FIX2INT(get_from_array(lazy_bounds, 0))));
        ymin = INT2FIX(MIN(cur->ymin, FIX2INT(get_from_array(lazy_bounds, 1))));
        xmax = INT2FIX(MAX(cur->xmax, FIX2INT(get_from_array(lazy_bounds, 2))));
        ymax = INT2FIX(MAX(cur->ymax, FIX2INT(get_from_array(lazy_bounds, 3))));

        set_array_value(lazy_bounds, 0, xmin);
        set_array_value(lazy_bounds, 1, ymin);
        set_array_value(lazy_bounds, 2, xmax);
        set_array_value(lazy_bounds, 3, ymax);
    }
}

void
update_bounds(action_struct * cur, int xmin, int ymin, int xmax, int ymax)
{
    if(xmin > xmax) SWAP(xmin, xmax);
    if(ymin > ymax) SWAP(ymin, ymax);
    
    cur->xmin = MIN(cur->xmin, xmin);
    cur->ymin = MIN(cur->ymin, ymin);
    cur->xmax = MAX(cur->xmax, xmax);
    cur->ymax = MAX(cur->ymax, ymax);
}

void
set_local_bounds(action_struct * cur, int xmin, int ymin, int xmax, int ymax, texture_info * tex)
{
    if(cur->sync_mode == no_sync)
        return;

    /* local bounds used by both eager_sync and lazy_sync: */
    
    /* eager sync: to demarcate precise area to sync to opengl */
    /* lazy sync: to update global bounds */
    cur->xmin = xmin;
    cur->ymin = ymin;
    cur->xmax = xmax;
    cur->ymax = ymax;
}

void
draw_prologue(action_struct * cur, texture_info * tex, int xmin, int ymin, int xmax, int ymax,
              VALUE * hash_arg, sync sync_mode, bool primary, action_struct ** payload_ptr)
{
    if(!primary) return;

    /* set the payload pointer */
    *payload_ptr = cur;
    
    /* not too happy about having this here, look at texplay.h for why */
    cur->tex = tex;
    
    process_common_hash_args(cur, hash_arg, sync_mode, primary);
    
    set_local_bounds(cur, xmin, ymin, xmax, ymax, tex);
}

void
draw_epilogue(action_struct * cur, texture_info * tex, bool primary)
{
    /* only primary actions get sync'd */
    if(!primary) return;
    
    switch(cur->sync_mode) {

        /* do not sync */
    case no_sync:
        return;
        break;

        /* sync immediately */
    case eager_sync:
        create_subtexture_and_sync_to_gl(IMAGE_BOUNDS(cur), tex);
        break;

        /* sync later (at end of paint block?) */
    case lazy_sync:
        update_lazy_bounds(cur, tex);
        break;
        
    default:
        rb_raise(rb_eRuntimeError,
                 "sync_mode may only be: lazy_sync, eager_sync, no_sync. got %d\n", cur->sync_mode);
    }
}

/* set the pixel color at (x, y) */
void
set_pixel_color(rgba * pixel_color, texture_info * tex, int x, int y) 
{
    float * tex_data; 

    /* should pixel be drawn ? */
    if (x > (tex->width - 1) || x < 0 || y > (tex->height - 1) || y < 0)
        return;

    /* if not a color then do not draw */
    if(not_a_color(*pixel_color))
        return;
    
    tex_data = get_pixel_data(tex, x, y);

    /* set the pixel color */
    tex_data[red] = pixel_color->red;
    tex_data[green] = pixel_color->green;
    tex_data[blue] = pixel_color->blue;
    tex_data[alpha] = pixel_color->alpha;    
}

void
set_pixel_color_with_style(action_struct * payload, texture_info * tex, int x, int y)
{

    rgba blended_pixel;

    blended_pixel = payload->color;

    /* for linear interpolation */
    if(payload->pen.has_lerp) {
      blended_pixel = apply_lerp(payload, tex, x, y);
    }

    /* for color_control transform */
    if(payload->pen.has_color_control_transform)
        blended_pixel = apply_color_control_transform(payload, tex, x, y);
    
    /* for texture fill  */
    if(payload->pen.has_source_texture)
        blended_pixel = get_pixel_color(&payload->pen.source_tex,
                                         x % payload->pen.source_tex.width,
                                         y % payload->pen.source_tex.height);

    /* for color_control block */
    if(payload->pen.has_color_control_proc)
        blended_pixel = exec_color_control_proc(payload, tex, x,  y, blended_pixel);
    

    /*  TO DO: do bitwise pixel combinations here */
    if(payload->pen.has_drawing_mode) {
      blended_pixel = apply_drawing_mode(payload, tex, x, y);
    }

    /*  TO DO: refactor into its own helper function
        & rewrite using sse2 */
    if(payload->pen.alpha_blend)
        blended_pixel = apply_alpha_blend(payload, tex, x,  y, blended_pixel);
    

    set_pixel_color(&blended_pixel, tex, x, y);
}


rgba
get_pixel_color_from_chunk(float * chunk, int width, int height, int x, int y)
{
    rgba my_color;

    int offset;

    if (x > (width - 1) || x < 0 || y > (height - 1) || y < 0)
        return not_a_color_v;

    offset = 4 * (x + y * width);

    my_color.red = chunk[offset + red];
    my_color.green = chunk[offset + green];
    my_color.blue = chunk[offset + blue];
    my_color.alpha = chunk[offset + alpha];

    return my_color;
}


rgba
get_pixel_color(texture_info * tex, int x, int y) 
{
    rgba my_color;
    
    int offset;

    /* if pixel location is out of range return not_a_color_v */
    if (x > (tex->width - 1) || x < 0 || y > (tex->height - 1) || y < 0)
        return not_a_color_v;

    offset = calc_pixel_offset(tex, x, y);
    
    my_color.red = tex->td_array[offset + red];
    my_color.green = tex->td_array[offset + green];
    my_color.blue = tex->td_array[offset + blue];
    my_color.alpha = tex->td_array[offset + alpha];

    return my_color;
}

/* return the array where pixel data is stored */
float*
get_pixel_data(texture_info * tex, int x, int y) 
{
    int offset = calc_pixel_offset(tex, x, y); 

    return &tex->td_array[offset];
}

/* if 2nd param is Qnil, then create a blank image with 'width' and 'height
   otherwise, try to use the 2nd param (dup) to create a duplicate image 
 */
VALUE
create_image(VALUE window, int width, int height)
{
    VALUE gosu = rb_const_get(rb_cObject, rb_intern("Gosu"));
    VALUE image = rb_const_get(gosu, rb_intern("Image"));

    VALUE tp = rb_const_get(rb_cObject, rb_intern("TexPlay"));
    VALUE empty_image_stub = rb_const_get(tp, rb_intern("EmptyImageStub"));
    
    VALUE rmagick_img;
    VALUE new_image;

    rmagick_img = rb_funcall(empty_image_stub, rb_intern("new"), 2, INT2FIX(width), INT2FIX(height));

    new_image = rb_funcall(image, rb_intern("new"), 2, window, rmagick_img);

    return new_image;
}
    


/* static functions */
static void
initialize_action_struct(action_struct * cur, VALUE hash_arg, sync sync_mode)
{
    /* initialize action-struct to default values */
    cur->sync_mode = sync_mode;
    cur->hash_arg = hash_arg;

    cur->color = convert_image_local_color_to_rgba(cur->tex->image);
    cur->pen.has_color_control_proc = false;
    cur->pen.has_color_control_transform = false;
    cur->pen.has_source_texture = false;
    cur->pen.alpha_blend = false;

    /* set static color control transformations to defaults */
    cur->pen.color_mult.red = 1.0;
    cur->pen.color_mult.green = 1.0;
    cur->pen.color_mult.blue = 1.0;
    cur->pen.color_mult.alpha = 1.0;

    cur->pen.color_add.red = 0.0;
    cur->pen.color_add.green = 0.0;
    cur->pen.color_add.blue = 0.0;
    cur->pen.color_add.alpha = 0.0;

    /* lerp is off by default */
    cur->pen.has_lerp = false;

    /* drawing mode is off by deafult */
    cur->pen.has_drawing_mode = false;
}
    
/* TODO: fix this function below, it's too ugly and bulky and weird **/
static void
process_common_hash_args(action_struct * cur, VALUE * hash_arg, sync sync_mode, bool primary)
{
    
    VALUE user_defaults;  
    VALUE hash_blend;


    /* if a hash doesn't exist then create one */
    if(!is_a_hash(*hash_arg)) 
        *hash_arg = rb_hash_new();

    /* init the action to default values */
    initialize_action_struct(cur, *hash_arg, sync_mode);

    /* get the user default options & merge with given options */
    user_defaults = get_image_local(cur->tex->image, USER_DEFAULTS);
    hash_blend = rb_funcall(user_defaults, rb_intern("merge"), 1, *hash_arg);
    rb_funcall(*hash_arg, rb_intern("merge!"), 1, hash_blend);

    if(has_optional_hash_arg(*hash_arg, "color")) {
        VALUE c = get_from_hash(*hash_arg, "color");
        cur->color = convert_rb_color_to_rgba(c);
        if(c == string2sym("random")) {
            set_hash_value(*hash_arg, "color", convert_rgba_to_rb_color(&cur->color));
        }
    }

    /* shadows */
    if(RTEST(get_from_hash(*hash_arg, "shadow"))) {
        cur->pen.color_mult.red = 0.66;
        cur->pen.color_mult.green = 0.66;
        cur->pen.color_mult.blue = 0.66;
        cur->pen.color_mult.alpha = 1;

        cur->pen.has_color_control_transform = true;
    }

    /* lerp */
    if(RTEST(get_from_hash(*hash_arg, "lerp"))) {
      cur->pen.lerp = NUM2DBL(get_from_hash(*hash_arg, "lerp"));

      /* bounds */
      if(cur->pen.lerp > 1.0) cur->pen.lerp = 1.0;
      if(cur->pen.lerp < 0.0) cur->pen.lerp = 0.0;
      cur->pen.has_lerp = true;
    }
    
    /* sync mode */
    if(has_optional_hash_arg(*hash_arg, "sync_mode")) {
        VALUE user_sync_mode = get_from_hash(*hash_arg, "sync_mode");

        Check_Type(user_sync_mode, T_SYMBOL);
        
        if(user_sync_mode == string2sym("lazy_sync"))
            cur->sync_mode = lazy_sync;
        else if(user_sync_mode == string2sym("eager_sync"))
            cur->sync_mode = eager_sync;
        else if(user_sync_mode == string2sym("no_sync"))
            cur->sync_mode = no_sync;
        else
            rb_raise(rb_eArgError, "unrecognized sync mode: %s\n. Allowable modes are "
                     ":lazy_sync, :eager_sync, :no_sync.",
                     sym2string(user_sync_mode));

        delete_from_hash(*hash_arg, "sync_mode");
        
    }

    /* process drawing mode */
    prepare_drawing_mode(cur);

    /* process the color_control block or transform (if there is one) */
    prepare_color_control(cur);

    /* process the filling texture (if there is one) */
    prepare_fill_texture(cur);

    /* does the user want to blend alpha values ? */
    if(get_from_hash(*hash_arg, "alpha_blend") == Qtrue)
        cur->pen.alpha_blend = true;

}

static void
prepare_drawing_mode(action_struct * cur)
{
  if(is_a_hash(cur->hash_arg)) {
    /* drawing mode */
    if(has_optional_hash_arg(cur->hash_arg, "mode")) {
      cur->pen.has_drawing_mode = true;

      VALUE draw_mode = get_from_hash(cur->hash_arg, "mode");


      Check_Type(draw_mode, T_SYMBOL);
        
      if(draw_mode == string2sym("clear"))
        cur->pen.drawing_mode = clear;

      else if(draw_mode == string2sym("copy"))
        cur->pen.drawing_mode = copy;

      else if(draw_mode == string2sym("noop"))
        cur->pen.drawing_mode = noop;

      else if(draw_mode == string2sym("set"))
        cur->pen.drawing_mode = set;

      else if(draw_mode == string2sym("copy_inverted"))
        cur->pen.drawing_mode = copy_inverted;

      else if(draw_mode == string2sym("invert"))
        cur->pen.drawing_mode = invert;

      else if(draw_mode == string2sym("and_reverse"))
        cur->pen.drawing_mode = and_reverse;

      else if(draw_mode == string2sym("and"))
        cur->pen.drawing_mode = and;

      else if(draw_mode == string2sym("or"))
        cur->pen.drawing_mode = or;

      else if(draw_mode == string2sym("nand"))
        cur->pen.drawing_mode = nand;

      else if(draw_mode == string2sym("nor"))
        cur->pen.drawing_mode = nor;

      else if(draw_mode == string2sym("xor"))
        cur->pen.drawing_mode = xor;

      else if(draw_mode == string2sym("equiv"))
        cur->pen.drawing_mode = equiv;

      else if(draw_mode == string2sym("and_inverted"))
        cur->pen.drawing_mode = and_inverted;

      else if(draw_mode == string2sym("or_inverted"))
        cur->pen.drawing_mode = or_inverted;

      else if(draw_mode == string2sym("additive"))
        cur->pen.drawing_mode = additive;
      else if(draw_mode == string2sym("multiply"))
        cur->pen.drawing_mode = multiply;
      else if(draw_mode == string2sym("screen"))
        cur->pen.drawing_mode = screen;
      else if(draw_mode == string2sym("overlay"))
        cur->pen.drawing_mode = overlay;
      else if(draw_mode == string2sym("darken"))
        cur->pen.drawing_mode = darken;
      else if(draw_mode == string2sym("lighten"))
        cur->pen.drawing_mode = lighten;
      else if(draw_mode == string2sym("colordodge"))
        cur->pen.drawing_mode = colordodge;
      else if(draw_mode == string2sym("colorburn"))
        cur->pen.drawing_mode = colorburn;
      else if(draw_mode == string2sym("hardlight"))
        cur->pen.drawing_mode = hardlight;
      else if(draw_mode == string2sym("softlight"))
        cur->pen.drawing_mode = softlight;
      else if(draw_mode == string2sym("difference"))
        cur->pen.drawing_mode = difference;
      else if(draw_mode == string2sym("exclusion"))
        cur->pen.drawing_mode = exclusion;      
      else
        rb_raise(rb_eArgError, "unrecognized drawing mode: %s\n.",
                 sym2string(draw_mode));
    }
  }
}
  

/* set action color to return value of color_control proc */
static void
prepare_color_control(action_struct * cur)
{

    if(is_a_hash(cur->hash_arg)) {
        VALUE try_val = get_from_hash(cur->hash_arg, "color_control");
        
        if(rb_respond_to(try_val, rb_intern("call"))) {
            cur->pen.color_control_proc = try_val;
            cur->pen.color_control_arity = FIX2INT(rb_funcall(try_val, rb_intern("arity"), 0));
            cur->pen.has_color_control_proc = true;
        }
        else if(is_a_hash(try_val)) {
            VALUE try_add = get_from_hash(try_val, "add");
            VALUE try_mult = get_from_hash(try_val, "mult");

            if(is_an_array(try_add)) {
                if(RARRAY_LEN(try_add) < 4)
                    rb_raise(rb_eArgError, ":color_control transform :add needs 4 parameters");
                
                cur->pen.color_add.red = NUM2DBL(get_from_array(try_add, 0));
                cur->pen.color_add.green = NUM2DBL(get_from_array(try_add, 1));
                cur->pen.color_add.blue = NUM2DBL(get_from_array(try_add, 2));
                cur->pen.color_add.alpha = NUM2DBL(get_from_array(try_add, 3));

                cur->pen.has_color_control_transform = true;
            }
            if(is_an_array(try_mult)) {
                if(RARRAY_LEN(try_mult) < 4)
                    rb_raise(rb_eArgError, ":color_control transform :mult needs 4 parameters");

                cur->pen.color_mult.red = NUM2DBL(get_from_array(try_mult, 0));
                cur->pen.color_mult.green = NUM2DBL(get_from_array(try_mult, 1));
                cur->pen.color_mult.blue = NUM2DBL(get_from_array(try_mult, 2));
                cur->pen.color_mult.alpha = NUM2DBL(get_from_array(try_mult, 3));

                cur->pen.has_color_control_transform = true;
            }
                
        }
    }
}

static rgba 
exec_color_control_proc(action_struct * cur, texture_info * tex, int x, int y, rgba blended_pixel)
{
    int arity = cur->pen.color_control_arity;
    VALUE proc = cur->pen.color_control_proc;
    rgba old_color = get_pixel_color(tex, x, y);
    rgba current_color = blended_pixel;
    rgba new_color;

    if(!cur->pen.has_color_control_proc)
        rb_raise(rb_eRuntimeError, "needs a proc");

    switch(arity) {
    case -1:
    case 0:
        new_color = convert_rb_color_to_rgba(rb_funcall(proc, rb_intern("call"), 0));
        break;
        
    case 1:
        new_color = convert_rb_color_to_rgba(rb_funcall(proc, rb_intern("call"), arity,
                                                        convert_rgba_to_rb_color(&old_color)));
        break;
            
    case 2:
        new_color = convert_rb_color_to_rgba(rb_funcall(proc, rb_intern("call"), arity,
                                                        convert_rgba_to_rb_color(&old_color),
                                                        convert_rgba_to_rb_color(&current_color)));
        break;
            
    case 3:
        new_color = convert_rb_color_to_rgba(rb_funcall(proc, rb_intern("call"), arity,
                                                        convert_rgba_to_rb_color(&old_color),
                                                        INT2FIX(x), INT2FIX(y)));
        break;
    case 4:
        new_color = convert_rb_color_to_rgba(rb_funcall(proc, rb_intern("call"), arity,
                                                        convert_rgba_to_rb_color(&old_color),
                                                        convert_rgba_to_rb_color(&current_color),
                                                        INT2FIX(x), INT2FIX(y)));
        break;
    default:
        rb_raise(rb_eArgError, "permissible arities for color_control proc are 1, 2, 3  and 4. Got %d\n",
                 arity);
    }
        
    /* update the action color */
    return new_color;
}

static void
prepare_fill_texture(action_struct * payload)
{
    if(is_a_hash(payload->hash_arg)) {
        VALUE try_image = get_from_hash(payload->hash_arg, "texture");
        if(is_gosu_image(try_image)) {

            get_texture_info(try_image, &payload->pen.source_tex);
            payload->pen.has_source_texture = true;
        }
    }
}

/***********************************/
/**** drawing mode related code ****/
/***********************************/

typedef struct {
    unsigned char red, green, blue, alpha;
} rgba_char;

static inline rgba_char 
color_float_to_int_format(rgba c)
{
  return (rgba_char) { c.red * 255,
                       c.green * 255,
                       c.blue * 255,
                       c.alpha * 255
                      };
}

static inline rgba 
color_int_vals_to_float_format(unsigned char r, unsigned char g, unsigned char b,
                   unsigned char a)
{
  return (rgba) { r / 255.0,
                  g / 255.0,
                  b / 255.0,
                  a / 255.0
                };
}

/* using terminology from photoshop PDF. b=background, s=source */
static inline rgba
mode_multiply(rgba b, rgba s)
{
  return (rgba) { b.red * s.red,
                  b.green * s.green,
                  b.blue * s.blue,
                  b.alpha * s.alpha };
}

static inline rgba
mode_screen(rgba b, rgba s)
{
  return (rgba) { b.red + s.red - (b.red * s.red),
                  b.green + s.green - (b.green * s.green),
                  b.blue + s.blue - (b.blue * s.blue),
                  b.alpha + s.alpha - (b.alpha * s.alpha) };
}

static inline float
mode_hardlight_channel(float b, float s)
{
  if (s <= 0.5)
    return 2 * b *  s; 
          
  else
    return b + s - (b * s); 
}

static inline rgba
mode_hardlight(rgba b, rgba s)
{
  return (rgba) { mode_hardlight_channel(b.red, s.red),
                  mode_hardlight_channel(b.green, s.green),
                  mode_hardlight_channel(b.blue, s.blue),
                  mode_hardlight_channel(b.alpha, s.alpha) };
}

/* function from the photoshop PDF to implement soft lighting */
static inline float
D(float x)
{
  if (x <= 0.25)
    return ((16 * x - 12) * x + 4) * x;
  else
    return sqrt(x);
}

static inline float
mode_softlight_channel(float b, float s)
{
  if (s <= 0.5)
    return b - (1 - 2 * s) * b * (1 - b);
  else
    return b + (2 * s - 1) * (D(b) - b);
}

static inline rgba
mode_softlight(rgba b, rgba s)
{
  return (rgba) { mode_softlight_channel(b.red, s.red),
                  mode_softlight_channel(b.green, s.green),
                  mode_softlight_channel(b.blue, s.blue),
                  mode_softlight_channel(b.alpha, s.alpha) };
}

static inline float
mode_colordodge_channel(float b, float s)
{
  if (s < 1)
    return MIN(1, b / (1 - s));
  else
    return 1;
}

static inline float
mode_colorburn_channel(float b, float s)
{
  if (s > 0)
    return 1 - MIN(1, (1 - b) / s);
  else
    return 0;
}
  
static rgba
apply_drawing_mode(action_struct * payload, texture_info * tex, int x, int y)
{
  rgba finished_pixel;
  
  rgba source_pixel = payload->color;  
  rgba dest_pixel = get_pixel_color(tex, x, y);

  rgba_char dest_pixel_char = color_float_to_int_format(dest_pixel);
  rgba_char source_pixel_char = color_float_to_int_format(source_pixel);
  
  switch(payload->pen.drawing_mode)
    {
      
    /* bitwise blending functions */
    case clear:
      finished_pixel = (rgba) { 0, 0, 0, 0 };
      break;
    case copy:
      finished_pixel = source_pixel;
      break;
    case noop:
      finished_pixel = dest_pixel;
      break;
    case set:
      finished_pixel = (rgba) { 1, 1, 1, 1 };
      break;
    case copy_inverted:
      finished_pixel = color_int_vals_to_float_format(~source_pixel_char.red,
                                               ~source_pixel_char.green,
                                               ~source_pixel_char.blue,
                                               ~source_pixel_char.alpha);
      break;
    case invert:
      finished_pixel = color_int_vals_to_float_format(~dest_pixel_char.red,
                                               ~dest_pixel_char.green,
                                               ~dest_pixel_char.blue,
                                               ~dest_pixel_char.alpha);
                                               
      break;
    case and_reverse:
      finished_pixel = color_int_vals_to_float_format(source_pixel_char.red | ~dest_pixel_char.red,
                                               source_pixel_char.green | ~dest_pixel_char.green,
                                               source_pixel_char.blue | ~dest_pixel_char.blue,
                                               source_pixel_char.alpha | ~dest_pixel_char.alpha);
      break;
    case and:
      finished_pixel = color_int_vals_to_float_format(source_pixel_char.red & dest_pixel_char.red,
                                               source_pixel_char.green & dest_pixel_char.green,
                                               source_pixel_char.blue & dest_pixel_char.blue,
                                               source_pixel_char.alpha & dest_pixel_char.alpha);
      break;
    case or:
      finished_pixel = color_int_vals_to_float_format(source_pixel_char.red | dest_pixel_char.red,
                                               source_pixel_char.green | dest_pixel_char.green,
                                               source_pixel_char.blue | dest_pixel_char.blue,
                                               source_pixel_char.alpha | dest_pixel_char.alpha);
      
      break;
    case nand:
      finished_pixel = color_int_vals_to_float_format(~(source_pixel_char.red & dest_pixel_char.red),
                                               ~(source_pixel_char.green & dest_pixel_char.green),
                                               ~(source_pixel_char.blue & dest_pixel_char.blue),
                                               ~(source_pixel_char.alpha & dest_pixel_char.alpha));
      
      break;
    case nor:
      finished_pixel = color_int_vals_to_float_format(~(source_pixel_char.red | dest_pixel_char.red),
                                               ~(source_pixel_char.green | dest_pixel_char.green),
                                               ~(source_pixel_char.blue | dest_pixel_char.blue),
                                               ~(source_pixel_char.alpha | dest_pixel_char.alpha));
      
      break;
    case xor:
      finished_pixel = color_int_vals_to_float_format(source_pixel_char.red ^ dest_pixel_char.red,
                                               source_pixel_char.green ^ dest_pixel_char.green,
                                               source_pixel_char.blue ^ dest_pixel_char.blue,
                                               source_pixel_char.alpha ^ dest_pixel_char.alpha);
      
      break;
    case equiv:
      finished_pixel = color_int_vals_to_float_format(~(source_pixel_char.red ^ dest_pixel_char.red),
                                               ~(source_pixel_char.green ^ dest_pixel_char.green),
                                               ~(source_pixel_char.blue ^ dest_pixel_char.blue),
                                               ~(source_pixel_char.alpha ^ dest_pixel_char.alpha));
      
      break;
    case and_inverted:
      finished_pixel = color_int_vals_to_float_format(~source_pixel_char.red & dest_pixel_char.red,
                                               ~source_pixel_char.green & dest_pixel_char.green,
                                               ~source_pixel_char.blue & dest_pixel_char.blue,
                                               ~source_pixel_char.alpha & dest_pixel_char.alpha);      
      break;
    case or_inverted:
      finished_pixel = color_int_vals_to_float_format(~source_pixel_char.red | dest_pixel_char.red,
                                               ~source_pixel_char.green | dest_pixel_char.green,
                                               ~source_pixel_char.blue | dest_pixel_char.blue,
                                               ~source_pixel_char.alpha | dest_pixel_char.alpha);
      
      break;

    /* photoshop style blending functions */
    case additive:
      finished_pixel = (rgba) { MIN(source_pixel.red + dest_pixel.red, 1),
                                MIN(source_pixel.green + dest_pixel.green, 1),
                                MIN(source_pixel.blue + dest_pixel.blue, 1),
                                MIN(source_pixel.alpha + dest_pixel.alpha, 1) };
      break;
    case multiply:
      finished_pixel = mode_multiply(dest_pixel, source_pixel);
                       
      break;
    case screen:
      finished_pixel = mode_screen(dest_pixel, source_pixel);
      
      break;
    case overlay:
      finished_pixel = mode_hardlight(source_pixel, dest_pixel);

      break;

    case darken:
      finished_pixel = (rgba) { MIN(source_pixel.red, dest_pixel.red),
                                MIN(source_pixel.green, dest_pixel.green),
                                MIN(source_pixel.blue, dest_pixel.blue),
                                MIN(source_pixel.alpha, dest_pixel.alpha) };
      break;
    case lighten:
      finished_pixel = (rgba) { MAX(source_pixel.red, dest_pixel.red),
                                MAX(source_pixel.green, dest_pixel.green),
                                MAX(source_pixel.blue, dest_pixel.blue),
                                MAX(source_pixel.alpha, dest_pixel.alpha) };
      break;
    case colordodge:
      finished_pixel = (rgba) { mode_colordodge_channel(dest_pixel.red, source_pixel.red),
                                mode_colordodge_channel(dest_pixel.green, source_pixel.green),
                                mode_colordodge_channel(dest_pixel.blue, source_pixel.blue),
                                mode_colordodge_channel(dest_pixel.alpha, source_pixel.alpha) };
                                
      break;
    case colorburn:
      finished_pixel = (rgba) { mode_colorburn_channel(dest_pixel.red, source_pixel.red),
                                mode_colorburn_channel(dest_pixel.green, source_pixel.green),
                                mode_colorburn_channel(dest_pixel.blue, source_pixel.blue),
                                mode_colorburn_channel(dest_pixel.alpha, source_pixel.alpha) };
      break;
    case hardlight:
      finished_pixel = mode_hardlight(dest_pixel, source_pixel);

      break;
    case softlight:
      finished_pixel = mode_softlight(dest_pixel, source_pixel);

      break;
    case difference:
      finished_pixel = (rgba) { ABS(dest_pixel.red - source_pixel.red),
                                ABS(dest_pixel.green - source_pixel.green),
                                ABS(dest_pixel.blue - source_pixel.blue),
                                ABS(dest_pixel.alpha - source_pixel.alpha) };
      break;
    case exclusion:
      finished_pixel = (rgba) { dest_pixel.red + source_pixel.red - (2 * dest_pixel.red * source_pixel.red),
                                dest_pixel.green + source_pixel.green - (2 * dest_pixel.green * source_pixel.green),
                                dest_pixel.blue + source_pixel.blue - (2 * dest_pixel.blue * source_pixel.blue),
                                dest_pixel.alpha + source_pixel.alpha - (2 * dest_pixel.alpha * source_pixel.alpha) };
      break;
    }

  return finished_pixel;
}
/***************************************/
/**** end drawing mode related code ****/
/***************************************/


static rgba
apply_lerp(action_struct * payload, texture_info * tex, int x, int y)
{
  rgba finished_pixel;
  rgba dest_pixel = get_pixel_color(tex, x, y);

  finished_pixel.red = payload->pen.lerp * payload->color.red +
    (1 - payload->pen.lerp) * dest_pixel.red;

  finished_pixel.green = payload->pen.lerp * payload->color.green +
    (1 - payload->pen.lerp) * dest_pixel.green;

  finished_pixel.blue = payload->pen.lerp * payload->color.blue +
    (1 - payload->pen.lerp) * dest_pixel.blue;

  finished_pixel.alpha = payload->pen.lerp * payload->color.alpha +
    (1 - payload->pen.lerp) * dest_pixel.alpha;

  return finished_pixel;
}

  
/* TODO: reimplement using SSE2 */
static rgba
apply_color_control_transform(action_struct * payload, texture_info * tex, int x, int y)
                              
{
    rgba transformed_color;
    
    transformed_color = get_pixel_color(tex, x, y);
        
    transformed_color.red += payload->pen.color_add.red; 
    transformed_color.green += payload->pen.color_add.green; 
    transformed_color.blue += payload->pen.color_add.blue; 
    transformed_color.alpha += payload->pen.color_add.alpha;

    transformed_color.red *= payload->pen.color_mult.red; 
    transformed_color.green *= payload->pen.color_mult.green; 
    transformed_color.blue *= payload->pen.color_mult.blue; 
    transformed_color.alpha *= payload->pen.color_mult.alpha;

    return transformed_color;
}

static rgba
apply_alpha_blend(action_struct * payload, texture_info * tex, int x, int y, rgba blended_pixel)
{
    rgba dest_pixel = get_pixel_color(tex, x, y);
    rgba finished_pixel;


    if(not_a_color(blended_pixel))
        return blended_pixel;
    
    /* alpha blending is nothing more than a weighted average of src and dest pixels
       based on source alpha value */
    /* NB: destination alpha value is ignored */

    /** TO DO: rewrite this using sse2 instructions **/
    finished_pixel.red = blended_pixel.alpha * blended_pixel.red + (1 - blended_pixel.alpha)
        * dest_pixel.red;

    finished_pixel.green = blended_pixel.alpha * blended_pixel.green + (1 - blended_pixel.alpha)
        * dest_pixel.green;

    finished_pixel.blue = blended_pixel.alpha * blended_pixel.blue + (1 - blended_pixel.alpha)
        * dest_pixel.blue;


    finished_pixel.alpha = blended_pixel.alpha * blended_pixel.alpha + (1 - blended_pixel.alpha)
      * dest_pixel.alpha;


    return finished_pixel;
}

/* NEW from utils.c */
float *
allocate_texture(int width, int height)
{
    float * new_texture;
    int mval;
    
    mval = 4 * width * height * sizeof(float);
    //    assert(mval > 0); 
        
    new_texture = malloc(mval);
    
    return new_texture;
}

/* get information from texture */
void
get_texture_info(VALUE image, texture_info * tex) 
{
    VALUE info, gc_state_off;
    int toppos, leftpos;
    float top, left;    
    cache_entry * entry;

    /* hack to prevent segfault */
    gc_state_off = rb_gc_disable();

    tex->width = FIX2INT(rb_funcall(image, rb_intern("width"), 0));
    tex->height = FIX2INT(rb_funcall(image, rb_intern("height"), 0));

    /* ensure gl_tex_info returns non nil */
    info = check_for_texture_info(image);

    top = NUM2DBL(rb_funcall(info, rb_intern("top"), 0));
    left = NUM2DBL(rb_funcall(info, rb_intern("left"), 0));
    tex->tname = FIX2INT(rb_funcall(info ,rb_intern("tex_name"),0));

    /* search for texture in cache (& create if not extant) */
    entry = find_or_create_cache_entry(tex->tname);

    tex->td_array = entry->tdata;
    tex->yincr = entry->sidelength;

    /* scratch variables */
    toppos = ROUND(top * entry->sidelength * entry->sidelength);
    leftpos = ROUND(left * entry->sidelength);

    /* find the first pixel for the image */
    tex->firstpixel = (int)(toppos + leftpos);

    tex->x_offset = ROUND(left * tex->yincr);
    tex->y_offset = ROUND(top * tex->yincr);

    /* save the associated Gosu::Image */
    tex->image = image;
    
    /* only enable gc if was enabled on function entry */
    if(!gc_state_off) rb_gc_enable();
}

/* ensure gl_tex_info returns non nil */
VALUE
check_for_texture_info(VALUE image) 
{
    VALUE info;

    info = rb_funcall(image, rb_intern("gl_tex_info"), 0);
    
    if(NIL_P(info)) {
        VALUE image_name = rb_inspect(image);
        int width = FIX2INT(rb_funcall(image, rb_intern("width"), 0));
        int height = FIX2INT(rb_funcall(image, rb_intern("height"), 0));
        
        rb_raise(rb_eException, "Error: gl_tex_info returns nil for %s (%i x %i). Could be caused by "
                 "very large image or Gosu bug. Try updating Gosu or use a smaller image. Note: TexPlay should"
                 " be able to work with %i x %i images.",
                 StringValuePtr(image_name), width, height, max_quad_size() - 2, max_quad_size() - 2);
    }

    return info;
}

/* responsible for syncing a subimage to gl */
void
sync_to_gl(int tex_name, int x_offset, int y_offset, int width, int height, void * sub)
{
    /* set the opengl texture context */
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex_name);

    /* sync subtexture */
    glTexSubImage2D(GL_TEXTURE_2D, 0, x_offset, y_offset, width, height,
                    GL_RGBA, GL_FLOAT, sub);

    glDisable(GL_TEXTURE_2D);
}

void
create_subtexture_and_sync_to_gl(image_bounds * img_bounds, texture_info * tex)
{
    /* image vars */
    int xbound, ybound;
    float * sub = NULL;

    /* make the current action's boundaries sane; left until this point because we only
       know height/width here */
    constrain_boundaries(&img_bounds->xmin, &img_bounds->ymin, &img_bounds->xmax, &img_bounds->ymax,
                         tex->width, tex->height);
    
    /* helper variables */
    ybound = img_bounds->ymax - img_bounds->ymin + 1;
    xbound = img_bounds->xmax - img_bounds->xmin + 1;
        
    sub = get_image_chunk(tex, img_bounds->xmin, img_bounds->ymin, img_bounds->xmax, img_bounds->ymax);

    sync_to_gl(tex->tname, tex->x_offset + img_bounds->xmin, tex->y_offset + img_bounds->ymin, xbound,
               ybound, (void*)sub);

    free(sub);
}

float *
get_image_chunk(texture_info * tex, int xmin, int ymin, int xmax, int ymax)
{
    int xbound;
    int ybound;
    float * image_buf = NULL;

    constrain_boundaries(&xmin, &ymin, &xmax, &ymax, tex->width, tex->height);

    xbound = xmax - xmin + 1;
    ybound = ymax - ymin + 1;

    image_buf = allocate_texture(xbound, ybound);

    for(int y = 0; y < ybound; y++)
        for(int x = 0; x < xbound; x++) {
            int buf_index = 4 * (x + y * xbound);
                
            int offset = calc_pixel_offset(tex, x + xmin, y + ymin);

            color_copy(tex->td_array + offset, image_buf + buf_index);
        }

    return image_buf;
}
