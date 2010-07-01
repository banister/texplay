/* utils.c */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <ruby.h>
#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include "cache.h"
#include "texplay.h"
#include "utils.h"

#ifdef __APPLE__
#include <glut.h>
#else
#include <GL/glut.h>
#endif

/*
#define MULT_FLOAT4(X, Y) ({                            \
            asm volatile (                              \
                          "movups (%0), %%xmm0\n\t"     \
                          "mulps  (%1), %%xmm0\n\t"     \
                          "movups %%xmm0, (%1)"         \
                          :: "r" (X), "r" (Y)); })

#define COPY_FLOAT4(X, Y) ({                            \
            asm volatile (                              \
                          "movups (%0), %%xmm0\n\t"     \
                          "movups %%xmm0, (%1)"         \
                          :: "r" (X), "r" (Y)); })

*/
/* external linkage with static duration */
const rgba not_a_color_v = { -1.0, -1.0, -1.0, -1.0 };

/* utility functions */
char*
lowercase(char * string) 
{
    int  i = 0;

    while (string[i]) {
        string[i] = tolower(string[i]);
        i++;
    }
    
    return string;
}

char*
sym2string(VALUE sym) 
{
    return rb_id2name(SYM2ID(sym));
}

VALUE
string2sym(char * string) 
{
    return ID2SYM(rb_intern(string));
}

bool
is_a_hash(VALUE try_hash)
{
    return TYPE(try_hash) == T_HASH;
}

bool
is_an_array(VALUE try_array)
{
    return TYPE(try_array) == T_ARRAY;
}

bool is_a_num(VALUE try_num)
{
    return TYPE(try_num) == T_FIXNUM || TYPE(try_num) == T_FLOAT;
}

VALUE
get_from_hash(VALUE hash, char * sym) 
{

    if(TYPE(hash) != T_HASH) rb_raise(rb_eArgError, "hash argument expected");
    
    return rb_hash_aref(hash, string2sym(sym));
}

VALUE
set_hash_value(VALUE hash, char * sym, VALUE val)
{
    if(TYPE(hash) != T_HASH) rb_raise(rb_eArgError, "hash argument expected");
    
    rb_hash_aset(hash, string2sym(sym), val);

    return val;
}

VALUE
delete_from_hash(VALUE hash, char * sym)
{
    if(TYPE(hash) != T_HASH) rb_raise(rb_eArgError, "hash argument expected");
    
    return rb_hash_delete(hash, string2sym(sym));
}

/* returns true if 'hash' is a hash and the value mapped to key 'sym' is
   equal to 'val' */
bool
hash_value_is(VALUE hash, char * sym, VALUE val)
{
    if(TYPE(hash) != T_HASH) return false;

    if(get_from_hash(hash, sym) == val)
        return true;

    return false;
}
    
bool
has_optional_hash_arg(VALUE hash, char * sym) 
{
    if(TYPE(hash) != T_HASH) return false;

    if(NIL_P(get_from_hash(hash, sym)))
        return false;

    /* 'hash' is a hash and the sym exists */
    return true;
}

VALUE
set_array_value(VALUE array, int index, VALUE val)
{
    if(TYPE(array) != T_ARRAY) rb_raise(rb_eArgError, "array argument expected");

    rb_ary_store(array, index, val);

    return val;
}
    
VALUE
get_from_array(VALUE array, int index) 
{

    if(TYPE(array) != T_ARRAY) rb_raise(rb_eArgError, "array argument expected");
    
    return rb_ary_entry(array, index);
}


VALUE
init_image_local(VALUE image)
{
    VALUE image_local;
    
    if(!is_gosu_image(image))
        rb_raise(rb_eArgError, "not a valid image");
    
    /* initialize image_local hash if does not exist */
    if(!is_an_array(rb_iv_get(image, "__image_local__"))) {
        image_local = rb_ary_new();
        rb_iv_set(image, "__image_local__", image_local);
    }

    image_local = rb_iv_get(image, "__image_local__");

    return image_local;
}   
     
void
set_image_local(VALUE image, int name, VALUE val)
{
    VALUE image_local;
    
    image_local = init_image_local(image);

    set_array_value(image_local, name, val);
}

VALUE
get_image_local(VALUE image, int name)
{
    VALUE image_local;
    VALUE val;

    init_image_local(image);

    /* this var holds all the image local variables in an array */
    image_local = rb_iv_get(image, "__image_local__");

    /* a particular image_local variable */
    val = get_from_array(image_local, name);
    
    /* if the variable exists then return it */
    if(!NIL_P(val))
        return val;

    /* otherwise initialize the variable and then return it */
    else {
        switch(name) {
            VALUE init_offset, init_bounds, init_color, init_defaults;
        case DRAW_OFFSET:
            init_offset = rb_ary_new2(2);
            set_array_value(init_offset, 0, INT2FIX(0));
            set_array_value(init_offset, 1, INT2FIX(0));

            set_array_value(image_local, DRAW_OFFSET, init_offset);

            return init_offset;
            break;
        case LAZY_BOUNDS:
            init_bounds = rb_ary_new2(4);
            set_array_value(init_bounds, 0, INT2FIX(XMAX_OOB));
            set_array_value(init_bounds, 1, INT2FIX(YMAX_OOB));
            set_array_value(init_bounds, 2, INT2FIX(XMIN_OOB));
            set_array_value(init_bounds, 3, INT2FIX(YMIN_OOB));

            set_array_value(image_local, LAZY_BOUNDS, init_bounds);

            return init_bounds;
            break;
        case IMAGE_COLOR:
            init_color = rb_ary_new2(4);
            set_array_value(init_color, 0, rb_float_new(1.0));
            set_array_value(init_color, 1, rb_float_new(1.0));
            set_array_value(init_color, 2, rb_float_new(1.0));
            set_array_value(init_color, 3, rb_float_new(1.0));

            set_array_value(image_local, IMAGE_COLOR, init_color);

            return init_color;
            break;
        case USER_DEFAULTS:
            init_defaults = rb_hash_new();

            set_array_value(image_local, USER_DEFAULTS, init_defaults);

            return init_defaults;
            break;
        default:
            rb_raise(rb_eArgError, "unrecognized image_local variable number. got %d", name);
        }
    }
    
    /* never reached */
    return Qnil;
}

rgba
convert_image_local_color_to_rgba(VALUE image)
{
    rgba color;
    VALUE image_local_color = get_image_local(image, IMAGE_COLOR);

    color.red = NUM2DBL(get_from_array(image_local_color, red)); 
    color.green = NUM2DBL(get_from_array(image_local_color, green)); 
    color.blue = NUM2DBL(get_from_array(image_local_color, blue)); 
    color.alpha = NUM2DBL(get_from_array(image_local_color, alpha)); 
    
    return color;
}

VALUE
save_rgba_to_image_local_color(VALUE image, rgba color)
{
    /* abbreviation for image_local_color */
    VALUE ilc = get_image_local(image, IMAGE_COLOR);

    set_array_value(ilc, 0, rb_float_new(color.red));
    set_array_value(ilc, 1, rb_float_new(color.green));
    set_array_value(ilc, 2, rb_float_new(color.blue));
    set_array_value(ilc, 3, rb_float_new(color.alpha));

    return ilc;
}

bool
not_a_color(rgba color1) 
{
    return color1.red < 0 || color1.green < 0 ||
      color1.blue < 0 || color1.alpha < 0;
}

bool
is_a_color(rgba color1)
{
    return color1.red >= 0 && color1.green >= 0 && color1.blue >= 0 && color1.alpha >= 0;
}

bool
is_rb_raw_color(VALUE cval)
{
  return TYPE(cval) == T_ARRAY &&
    is_a_num(get_from_array(cval, 0)) &&
    is_a_num(get_from_array(cval, 1)) &&
    is_a_num(get_from_array(cval, 2)) &&
    is_a_num(get_from_array(cval, 3));
}

bool
not_rb_raw_color(VALUE cval)
{
  return TYPE(cval) != T_ARRAY ||
    !is_a_num(get_from_array(cval, 0));
}

/** cmp_color related functions **/
static bool
is_transparent_color(rgba color1)
{
    return color1.red == -666 && color1.green == -666 && color1.blue == -666 &&
      color1.alpha == -666;
}

static bool
special_cmp_color(rgba color1, rgba color2)
{
    if (is_transparent_color(color1))
        return color2.alpha == 0;
    else if (is_transparent_color(color2))
        return color1.alpha == 0;
}

static bool
special_cmp_color_with_tolerance(rgba color1, rgba color2, float tolerance)
{
    if (is_transparent_color(color1))
      return (color1.alpha) <= tolerance;
    else if (is_transparent_color(color2))
        return color2.alpha <= tolerance;
}


static float
color_distance_squared(rgba c1, rgba c2)
{
  return (c1.red - c2.red) * (c1.red - c2.red) +
    (c1.green - c2.green) * (c1.green - c2.green) +
    (c1.blue - c2.blue) * (c1.blue - c2.blue) +
    (c1.alpha - c2.alpha) * (c1.alpha - c2.alpha);
}
      

bool
cmp_color_with_tolerance(rgba color1, rgba color2, float tolerance)
{
  if (color1.red < 0 || color2.red < 0)
    return special_cmp_color_with_tolerance(color1, color2, tolerance);
  
  return color_distance_squared(color1, color2) <= (tolerance * tolerance);
}

bool
cmp_color(rgba color1, rgba color2) 
{
    if (color1.red < 0 || color2.red < 0)
      return special_cmp_color(color1, color2);
    
    return (color1.red == color2.red) && (color1.green == color2.green) && (color1.blue == color2.blue)
        && (color1.alpha == color2.alpha);
}
        

/*** these functions are UNSAFE ***/
void
color_copy(float * source, float * dest) 
{
    //COPY_FLOAT4(source, dest);
    memcpy(dest, source, 4 * sizeof(float));
}

void
zero_color(float * tex) 
{
    memset(tex, 0, 4 * sizeof(float));
}
/***                           ***/

rgba
find_color_from_string(char * try_color) 
{
    rgba cur_color;

    if(!strcmp("red", try_color)) {
        cur_color.red = 1.0; cur_color.green = 0.0; cur_color.blue = 0.0; cur_color.alpha = 1.0;
    }
    else if(!strcmp("green", try_color)) {
        cur_color.red = 0.0; cur_color.green = 1.0; cur_color.blue = 0.0; cur_color.alpha = 1.0;
    }
    else if(!strcmp("blue", try_color)) {
        cur_color.red = 0.0; cur_color.green = 0.0; cur_color.blue = 1.0; cur_color.alpha = 1.0;
    }
    else if(!strcmp("black", try_color)) {
        cur_color.red = 0.0; cur_color.green = 0.0; cur_color.blue = 0.0; cur_color.alpha = 1.0;
    }
    else if(!strcmp("white", try_color)) {
        cur_color.red = 1.0; cur_color.green = 1.0; cur_color.blue = 1.0; cur_color.alpha = 1.0;
    }
    else if(!strcmp("purple", try_color)) {
        cur_color.red = 1.0; cur_color.green = 0.0; cur_color.blue = 1.0; cur_color.alpha = 1.0;
    }
    else if(!strcmp("yellow", try_color)) {
        cur_color.red = 1.0; cur_color.green = 1.0; cur_color.blue = 0.0; cur_color.alpha = 1.0;
    }
    else if(!strcmp("cyan", try_color)) {
        cur_color.red = 0.0; cur_color.green = 1.0; cur_color.blue = 1.0; cur_color.alpha = 1.0;
    }
    else if(!strcmp("orange", try_color)) {
        cur_color.red = 1.0; cur_color.green = 0.5; cur_color.blue = 0.0; cur_color.alpha = 1.0;
    }
    else if(!strcmp("brown", try_color)) {
        cur_color.red = 0.39; cur_color.green = 0.26; cur_color.blue = 0.13; cur_color.alpha = 1.0;
    }
    else if(!strcmp("turquoise", try_color)) {
        cur_color.red = 0.1; cur_color.green = 0.6; cur_color.blue = 0.8; cur_color.alpha = 1.0;
    }
    else if(!strcmp("tyrian", try_color))  {
        cur_color.red = 0.4; cur_color.green = 0.007; cur_color.blue = 0.235; cur_color.alpha = 1.0;
    }
    else if(!strcmp("alpha", try_color)) {
        cur_color.red = 0.0; cur_color.green = 0.0; cur_color.blue = 0.0; cur_color.alpha = 0.0;
    }
    else if(!strcmp("transparent", try_color)) {
        cur_color.red = -666; cur_color.green = -666; cur_color.blue = -666; cur_color.alpha = -666;
    }
    else if(!strcmp("none", try_color)) {
        cur_color = not_a_color_v;
    }
    else if(!strcmp("random", try_color) || !strcmp("rand", try_color)) {
        cur_color.red = rand() / (float)RAND_MAX;
        cur_color.green = rand() / (float)RAND_MAX;
        cur_color.blue = rand() / (float)RAND_MAX;
        cur_color.alpha = 1.0;
    }

    else
        rb_raise(rb_eArgError, "invalid colour specified (no color matches the symbol: %s)\n", try_color);

    return cur_color;
}

rgba
convert_gosu_to_rgba_color(VALUE gcolor)
{

  return (rgba) { 
          FIX2INT(rb_funcall(gcolor, rb_intern("red"), 0)) / 255.0,
          FIX2INT(rb_funcall(gcolor, rb_intern("green"), 0)) / 255.0,
          FIX2INT(rb_funcall(gcolor, rb_intern("blue"), 0)) / 255.0,
          FIX2INT(rb_funcall(gcolor, rb_intern("alpha"), 0)) / 255.0
            };
}


/* convert C color to Ruby color */
VALUE
convert_rgba_to_rb_color(rgba * pix)
{
    if (not_a_color(*pix)) return Qnil;
  
    /* create a new ruby array to store the pixel data */
    VALUE pix_array = rb_ary_new2(4);

    /* store the pixel data */
    rb_ary_store(pix_array, red, rb_float_new(pix->red));
    rb_ary_store(pix_array, green, rb_float_new(pix->green));
    rb_ary_store(pix_array, blue, rb_float_new(pix->blue));
    rb_ary_store(pix_array, alpha, rb_float_new(pix->alpha));

    return pix_array;
}

/* convert C color to gosu color */
VALUE
convert_rgba_to_gosu_color(rgba * pix)
{
    if (not_a_color(*pix)) return Qnil;

    VALUE gosu_color = rb_funcall(gosu_color_class(), rb_intern("new"), 0);

    rb_funcall(gosu_color, rb_intern("red="), 1, INT2FIX(pix->red * 255));
    rb_funcall(gosu_color, rb_intern("green="), 1, INT2FIX(pix->green * 255));
    rb_funcall(gosu_color, rb_intern("blue="), 1, INT2FIX(pix->blue * 255));
    rb_funcall(gosu_color, rb_intern("alpha="), 1, INT2FIX(pix->alpha * 255));

    return gosu_color;
}

VALUE
gosu_color_class()
{
   static VALUE gcolor_class = 0;

   if (gcolor_class == 0) {
       VALUE gosu_class = rb_const_get(rb_cObject, rb_intern("Gosu"));
        gcolor_class = rb_const_get(gosu_class, rb_intern("Color"));
    }

   return gcolor_class;
}

/* convert Ruby color to C color */
rgba
convert_rb_color_to_rgba(VALUE cval)
{
    rgba my_color;

    if (is_gosu_color(cval)) return convert_gosu_to_rgba_color(cval);
    
    /* current color for actions */
    switch(TYPE(cval)) {
        char * try_color;
    case T_SYMBOL:
        try_color = lowercase(sym2string(cval));

        my_color = find_color_from_string(try_color);

        break;
    case T_ARRAY:
        my_color.red = NUM2DBL(rb_ary_entry(cval, red));
        my_color.green = NUM2DBL(rb_ary_entry(cval, green));
        my_color.blue = NUM2DBL(rb_ary_entry(cval, blue));

        if(NUM2INT(rb_funcall(cval, rb_intern("length"), 0)) > 3) 
            my_color.alpha = NUM2DBL(rb_ary_entry(cval, alpha));
        else
            my_color.alpha = 1;

        break;

    /* hex literals */
    case T_FIXNUM:
    case T_BIGNUM:
      return convert_gosu_to_rgba_color(rb_funcall(gosu_color_class(),
                                                   rb_intern("new"), 1, cval));
    break;
        
    default:
        rb_raise(rb_eArgError, "unsupported argument type for color. Got type 0x%x\n", TYPE(cval) );
    }

    /* a valid color */
    if(is_a_color(my_color))
        return my_color;

    /* special condition for when color is taken from outside range of bitmap. Color is just ignored */
    else if(not_a_color(my_color))
        return not_a_color_v;

    /* anything else should fail */
    else
        rb_raise(rb_eArgError, "invalid colour specified (negative value given)\n");
}

/* error checking functions */
void
check_mask(VALUE mask) 
{
    char * try_mask;

    if(TYPE(mask) != T_ARRAY && TYPE(mask) != T_SYMBOL)
        rb_raise(rb_eArgError, "array or symbol parameter required");

    /* is it a valid mask symbol? */
    if(TYPE(mask) == T_SYMBOL) {
        try_mask = lowercase(sym2string(mask));
        if(*try_mask == '_') try_mask++;
        if(not_a_color(find_color_from_string(try_mask))) {
            rb_raise(rb_eArgError, "unrecognized mask symbol: %s\n", sym2string(mask));
        }
    }
}

void
check_image(VALUE image) 
{
    if(!rb_respond_to(image, rb_intern("gl_tex_info")))
        rb_raise(rb_eRuntimeError,"must specify a valid source image");
}

bool
is_gosu_image(VALUE try_image)
{
    if(rb_respond_to(try_image, rb_intern("gl_tex_info")))
        return true;

    return false;
}

bool
is_gosu_color(VALUE try_color)
{
    if(rb_respond_to(try_color, rb_intern("red")))
        return true;

    return false;
}



/** cohen-sutherland line clipper **/
#define outcode int
const int RIGHT = 8;  //1000
const int TOP = 4;    //0100
const int LEFT = 2;   //0010
const int BOTTOM = 1; //0001
 
//Compute the bit code for a point (x, y) using the clip rectangle
//bounded diagonally by (xmin, ymin), and (xmax, ymax)
static outcode
ComputeOutCode (int x, int y, int xmin, int ymin, int xmax, int ymax)
{
	outcode code = 0;
	if (y > ymax)              //above the clip window
		code |= TOP;
	else if (y < ymin)         //below the clip window
		code |= BOTTOM;
	if (x > xmax)              //to the right of clip window
		code |= RIGHT;
	else if (x < xmin)         //to the left of clip window
		code |= LEFT;
	return code;
}
 
/** Cohen-Sutherland clipping algorithm clips a line from
P0 = (x0, y0) to P1 = (x1, y1) against a rectangle with 
diagonal from (xmin, ymin) to (xmax, ymax). **/
void
cohen_sutherland_clip (int * x0, int * y0,int * x1, int * y1, int xmin, int ymin,
                                int xmax, int ymax)
{
    //Outcodes for P0, P1, and whatever point lies outside the clip rectangle
    outcode outcode0, outcode1, outcodeOut;
    bool accept = false, done = false;
    int tx0 = *x0, ty0 = *y0, tx1 = *x1, ty1 = *y1;
 
    //compute outcodes
    outcode0 = ComputeOutCode (tx0, ty0, xmin, ymin, xmax, ymax);
    outcode1 = ComputeOutCode (tx1, ty1, xmin, ymin, xmax, ymax);
 
    do{
        if (!(outcode0 | outcode1))      //logical or is 0. Trivially accept and get out of loop
            {
                accept = true;
                done = true;
            }
        else if (outcode0 & outcode1)   //logical and is not 0. Trivially reject and get out of loop
            done = true;
        else
            {
                //failed both tests, so calculate the line segment to clip
                //from an outside point to an intersection with clip edge
                double x, y;
                //At least one endpoint is outside the clip rectangle; pick it.
                outcodeOut = outcode0? outcode0: outcode1;
                //Now find the intersection point;
                //use formulas y = y0 + slope * (x - x0), x = x0 + (1/slope)* (y - y0)
                if (outcodeOut & TOP)          //point is above the clip rectangle
                    {
                        x = tx0 + (tx1 - tx0) * (ymax - ty0)/(ty1 - ty0);
                        y = ymax;
                    }
                else if (outcodeOut & BOTTOM)  //point is below the clip rectangle
                    {
                        x = tx0 + (tx1 - tx0) * (ymin - ty0)/(ty1 - ty0);
                        y = ymin;
                    }
                else if (outcodeOut & RIGHT)   //point is to the right of clip rectangle
                    {
                        y = ty0 + (ty1 - ty0) * (xmax - tx0)/(tx1 - tx0);
                        x = xmax;
                    }
                else                           //point is to the left of clip rectangle
                    {
                        y = ty0 + (ty1 - ty0) * (xmin - tx0)/(tx1 - tx0);
                        x = xmin;
                    }
                //Now we move outside point to intersection point to clip
                //and get ready for next pass.
                if (outcodeOut == outcode0)
                    {
                        tx0 = x;
                        ty0 = y;
                        outcode0 = ComputeOutCode (tx0, ty0, xmin, ymin, xmax, ymax);
                    }
                else 
                    {
                        tx1 = x;
                        ty1 = y;
                        outcode1 = ComputeOutCode (tx1, ty1, xmin, ymin, xmax, ymax);
                    }
            }
    }while (!done);
 
    if (accept)
	{
            *x0 = tx0; *x1 = tx1;
            *y0 = ty0; *y1 = ty1;
	}
 
}
/** end of cohen-sutherland line clipper **/


void
constrain_boundaries(int * x0, int * y0, int * x1, int * y1, int width, int height) 
{
    if(*y0 < 0) *y0 = 0;
    if(*y1 < 0) *y1 = 0;
    
    if(*x0 < 0) *x0 = 0;
    if(*x1 < 0) *x1 = 0;
    
    if(*x0 > (width - 1)) *x0 = width - 1;
    if(*x1 > (width - 1)) *x1 = width - 1;
    
    if(*y0 > (height - 1)) *y0 = height - 1;
    if(*y1 > (height - 1)) *y1 = height - 1;

    if(*y0 > *y1) { SWAP(*y0, *y1); }
    if(*x0 > *x1) { SWAP(*x0, *x1); }
}


/* returns true if point (x, y) is within bounds designated by rect (x0, y0)-(x1, y1)
   and inner thickness: 'inner'
*/
bool
bound_by_rect_and_inner(int x, int y, int x0, int y0, int x1, int y1, int inner) 
{
                          
    return ((x >= x0) && (x <= x1) && (y >= y0) && (y <= y1)) && 
        !((x >= x0 + inner) && (x <= x1 - inner) && (y >= y0 + inner) && (y <= y1 - inner));
}

/* same as above but excluding inner rectangle */
bool
bound_by_rect(int x, int y, int x0, int y0, int x1, int y1) 
{
    return bound_by_rect_and_inner(x, y, x0, y0, x1, y1, OOB_VAL);
}

/* calculate the array offset for a given pixel in action context */
int
calc_pixel_offset_for_action(action_struct * cur, texture_info * tex, int x, int y) 
{
    int offset = calc_pixel_offset(tex, x + cur->xmin, y + cur->ymin);

    return offset;
}

/* calculate the array offset for a given pixel */
int
calc_pixel_offset(texture_info * tex, int x, int y) 
{
    int offset = 4 * (tex->firstpixel + x + tex->yincr * y);    

    return offset;
}

/* NEWEST version that solves segfault on linux, back
   in  action due to availability of MAX_TEXTURE_SIZE constant
   in ruby 1.8 */
unsigned
max_quad_size(void)
{

    static unsigned size = 0;
    
    if (size == 0) {
      VALUE gosu = rb_const_get(rb_cObject, rb_intern("Gosu"));
      VALUE rb_size = rb_const_get(gosu, rb_intern("MAX_TEXTURE_SIZE"));

      size = FIX2INT(rb_size);
    }
    
    return size;
}

 /* old version for quick update, OUT OF ACTIONN */
/* unsigned */
/* max_quad_size(void) */
/* { */
/* #ifdef __APPLE__ */
/*   return 1024; */
/* #else */
/*   static unsigned MIN_SIZE = 256, MAX_SIZE = 1024; */
 
/*   static unsigned size = 0; */
/*   if (size == 0) */
/*     { */
/*       GLint width = 1; */
/*       size = MIN_SIZE / 2; */
/*       do { */
/*         size *= 2; */
/*         glTexImage2D(GL_PROXY_TEXTURE_2D, 0, 4, size * 2, size * 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0); */
/*         glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);  */
/*       } while (width != 0 && size < MAX_SIZE); */
/*     } */
    
/*   return size; */
/* #endif */
/* } */

/* point format utilities */
bool
is_a_point(VALUE try_point)
{
    /* if it responds to 'x' it's near enough (technically must respond to x AND y) */
    /* added the is_a_num() check due to WEIRD bug where FIXNUMS were responding to the 'x' method (wtf?) but returning nil when invoked */
    if(rb_respond_to(try_point, rb_intern("x")) && !is_a_num(try_point))
        return true;

    return false;
}

VALUE
point_x(VALUE point)
{
    return rb_funcall(point, rb_intern("x"), 0);
}

VALUE
point_y(VALUE point)
{
    return rb_funcall(point, rb_intern("y"), 0);
}

/* mathematical utils, used mainly by bezier curves */
double
power(float base, int exp)
{
    float ans = 1.0;
    if(base == 0.0) {
        if(exp == 0)
            return 1;
        else
            return 0;
    }
    else if(exp == 0) return 1;
    else {
        for(int k = exp; k >= 1; k--) {
            ans = ans * base;
        }
        return ans;
    }
}

unsigned
fact(int n)
{
    if (n == 0 || n == 1) return 1;
    else
        return (n * fact(n - 1));
}

unsigned
comb(int n, int r)
{
    /* nCr is symmetrical about n / 2 */
    if(r > (n / 2))
        r = n - r;
    
    return perm(n, r) / fact(r);
}

unsigned
perm(int n, int r)
{
    int val = 1;
    for(int i = n; i > (n - r); i--)
        val *= i;

    return val;
}

double
bernstein(int n, int k, float u)
{
    double temp = comb(n, k) * pow(u, k) * pow(1 - u, n - k);
    return temp;
}
