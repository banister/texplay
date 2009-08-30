#include <ruby.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

#ifdef __APPLE__
# include <glut.h>
#else
# include <GL/glut.h>
#endif

#include "texplay.h"
#include "utils.h"
#include "bindings.h"
#include "actions.h"
#include "cache.h"
#include "compat.h"

/* associated with gen_eval */
#include "object2module.h"
#include "gen_eval.h"

/* syncing mode */
/* lazy_sync  = sync at end of paint block */
/* eager_sync = sync immediately (after action) */
/* no_sync    = do not sync at all */
sync sync_mode = eager_sync;

static void
process_x_y_pairs(VALUE image, int num_pairs, VALUE * argv, ...)
{
    va_list ap;
    int i;
    int draw_offset_x;
    int draw_offset_y;
    VALUE offset_val;

    offset_val = get_image_local(image, DRAW_OFFSET);

    draw_offset_x = NUM2INT(get_from_array(offset_val, 0));
    draw_offset_y = NUM2INT(get_from_array(offset_val, 1));

    va_start(ap, argv);
    if(is_a_point(argv[0])) {
        for(i = 0; i < num_pairs; i++) {
            int *x_ptr, *y_ptr;

            x_ptr = va_arg(ap, int*);
            y_ptr = va_arg(ap, int*);

            *x_ptr = NUM2INT(rb_funcall(argv[i], rb_intern("x"), 0)) + draw_offset_x;
            *y_ptr = NUM2INT(rb_funcall(argv[i], rb_intern("y"), 0)) + draw_offset_y;
        }
    }
    else {
        for(i = 0; i < (num_pairs * 2); i+=2) {
            int *x_ptr, *y_ptr;

            x_ptr = va_arg(ap, int*);
            y_ptr = va_arg(ap, int*);

            *x_ptr = NUM2INT(argv[i]) + draw_offset_x;
            *y_ptr = NUM2INT(argv[i + 1]) + draw_offset_y;
        }
    }
    va_end(ap);
}

        
/* singleton methods */

/* responsible for creating macros */
VALUE
M_create_macro(VALUE self, VALUE method_name) 
{
    VALUE proc;

    rb_need_block();

    /* convert the block to a proc */
    proc = rb_block_proc(); 

    /* define the method in the TexPlay class so that it is accessible to 'instances' */
    rb_funcall(self, rb_intern("define_method"), 2, method_name, proc);

    return Qnil;
}


/* responsible for removing macros */
VALUE
M_remove_macro(VALUE self, VALUE method_name) 
{

    /* remove the method in the TexPlay class  */
    rb_funcall(self, rb_intern("remove_method"), 1, method_name);

    return Qnil;
}

/* responsible for refreshing all entries in cache */
VALUE
M_refresh_cache_all(VALUE self) 
{
    cache_refresh_all();

    return Qnil;
}

/* creates a blank image */
VALUE
M_create_blank(VALUE self, VALUE window, VALUE width, VALUE height)
{
    VALUE fresh_image;

    fresh_image = create_image(window, NUM2INT(width), NUM2INT(height));
    
    return fresh_image;
}
/** end singleton methods **/

/* some helper methods */
static void
rb_lazy_bounds_to_image_bounds(VALUE image, image_bounds * bounds)
{
    VALUE lazy_bounds;
    
    lazy_bounds = get_image_local(image, LAZY_BOUNDS);
    
    Check_Type(lazy_bounds, T_ARRAY);
    
    bounds->xmin = FIX2INT(get_from_array(lazy_bounds, 0));
    bounds->ymin = FIX2INT(get_from_array(lazy_bounds, 1));
    bounds->xmax = FIX2INT(get_from_array(lazy_bounds, 2));
    bounds->ymax = FIX2INT(get_from_array(lazy_bounds, 3));
}

static VALUE
parse_sync_mode(VALUE user_sync_mode)
{
    sync mode;
    
    Check_Type(user_sync_mode, T_SYMBOL);
        
    if(user_sync_mode == string2sym("lazy_sync"))
        mode = lazy_sync;
    else if(user_sync_mode == string2sym("eager_sync"))
        mode = eager_sync;
    else if(user_sync_mode == string2sym("no_sync"))
        mode = no_sync;
    else
        rb_raise(rb_eArgError, "unrecognized sync mode: %s\n. Allowable modes are "
                 ":lazy_sync, :eager_sync, :no_sync.",
                 sym2string(user_sync_mode));

    return mode;
}
/* end helpers */

/* entry point for TexPlay paint actions */
VALUE
m_paint(int argc, VALUE * argv, VALUE self) 
{
    texture_info tex;
    VALUE options;
    image_bounds bounds;
    int arity;

    ADJUST_SELF(self);

    rb_scan_args(argc, argv, "01", &options);

    /* get texture info from image */
    get_texture_info(self, &tex);

    /* set default sync_mode to lazy */
    sync_mode = lazy_sync;

    /* parse sync_mode, overriding lazy sync mode? */
    if(has_optional_hash_arg(options, "sync_mode")) {
        VALUE user_sync_mode = get_from_hash(options, "sync_mode");
        sync_mode = parse_sync_mode(user_sync_mode);
    }

    /* if no block then just sync */
    if(!rb_block_given_p()) {

        rb_lazy_bounds_to_image_bounds(self, &bounds);

        create_subtexture_and_sync_to_gl(&bounds, &tex);
        
        /* reset the LAZY_BOUNDS now we've sync'd */
        set_image_local(self, LAZY_BOUNDS, Qnil);

        sync_mode = eager_sync;

        return self;
    }

    /* find arity of block */
    arity = FIX2INT(rb_funcall(rb_block_proc(), rb_intern("arity"), 0));

    /* yield self if the arity is 1, else gen_eval the block */
    switch(arity) {
    case -1:
    case 0:
        rb_gen_eval(0, 0, self);
        break;
    case 1:
        rb_yield(self);
        break;
    default:            
        rb_raise(rb_eArgError, "block arity must be either 1 or -1 or 0, received arity of: %d", arity);
    }

    /* if lazy sync is selected then sync now..as the paint block has finished executing the draw actions*/
    if(sync_mode == lazy_sync) {
        
        rb_lazy_bounds_to_image_bounds(self, &bounds);

        create_subtexture_and_sync_to_gl(&bounds, &tex);

        /* reset the LAZY_BOUNDS now we've sync'd */
        set_image_local(self, LAZY_BOUNDS, Qnil);

    }

    /* now we've finished the paint block we reset the default sync_mode back to eager */
    sync_mode = eager_sync;

    return self;
}

VALUE
m_force_sync(VALUE self, VALUE ary)
{
    image_bounds bounds;
    texture_info tex;
    
    ADJUST_SELF(self);

    Check_Type(ary, T_ARRAY);

    get_texture_info(self, &tex);

    bounds.xmin = NUM2INT(get_from_array(ary, 0));
    bounds.ymin = NUM2INT(get_from_array(ary, 1));
    bounds.xmax = NUM2INT(get_from_array(ary, 2));
    bounds.ymax = NUM2INT(get_from_array(ary, 3));

    create_subtexture_and_sync_to_gl(&bounds, &tex);

    return Qnil;
}

VALUE
m_dup_image(VALUE self)
{
    texture_info tex, dup_tex;
    VALUE dupped_image;
    VALUE window;

    ADJUST_SELF(self);

    get_texture_info(self, &tex);

    window = rb_funcall(self, rb_intern("__window__"), 0);

    /* create a new blank image with the height/width of the current image */
    dupped_image = create_image(window, tex.width, tex.height);

    /* get the new image's data */
    get_texture_info(dupped_image, &dup_tex);

    /* splice into the new image content from the current image, and sync it to gl */
    splice_do_action(0, 0, 0, 0, XMAX_OOB, YMAX_OOB, &tex, &dup_tex, Qnil, eager_sync, true, NULL);

    /* copy across the ivars too! */
    rb_copy_generic_ivar(dupped_image, self);

    /* we now have a full dup of the current image, return it */
    return dupped_image;
}

VALUE
m_clone_image(VALUE self)
{
    VALUE cloned_image;

    ADJUST_SELF(self);

    cloned_image = m_dup_image(self);

    /* the main diff b/w clone and dup is that clone also dups the singleton */
    KLASS_OF(cloned_image) = rb_singleton_class_clone(self);

    return cloned_image;
}
    
VALUE
m_user_set_options(VALUE self, VALUE options)
{
    ADJUST_SELF(self);
    
    if(!is_a_hash(options))
        rb_raise(rb_eArgError, "only a single hash argument is accepted");

    set_image_local(self, USER_DEFAULTS, options);

    return Qnil;
}

VALUE
m_user_delete_options(VALUE self)
{

    ADJUST_SELF(self);

    set_image_local(self, USER_DEFAULTS, Qnil);

    return Qnil;
}

VALUE
m_get_options(VALUE self)
{
    ADJUST_SELF(self);

    return get_image_local(self, USER_DEFAULTS);
}

/* return the pixel colour for the given x, y */
VALUE
m_getpixel(int argc, VALUE * argv, VALUE self) 
{
    int x1, y1;
    texture_info tex;
    rgba pix;

    /* change self to hidden self if using gen_eval */
    ADJUST_SELF(self);

    process_x_y_pairs(self, 1, argv, &x1, &y1);

    /* get texture info */
    get_texture_info(self, &tex);

    /* locate the desired pixel; */
    pix = get_pixel_color(&tex, x1, y1);

    if(not_a_color(pix))
        return Qnil;
    else
        return convert_rgba_to_rb_color(&pix);
}

/* circle action */
VALUE
m_circle(int argc, VALUE * argv, VALUE self) 
{
    int x1, y1, r;
    int last = argc - 1;
    VALUE options;
    texture_info tex;    
    
    ADJUST_SELF(self);
    
    if(argc < 1) rb_raise(rb_eArgError, "circle action needs at least 1 parameter");

    process_x_y_pairs(self, 1, argv, &x1, &y1);

    if(is_a_point(argv[0]))
        r = NUM2INT(argv[1]);
    else
        r = NUM2INT(argv[2]);

    options = argv[last];

    get_texture_info(self, &tex);

    circle_do_action(x1, y1, r, &tex, options, sync_mode, true, NULL);

    return self;
}

/* ngon */
VALUE
m_ngon(int argc, VALUE * argv, VALUE self)
{
    int x1, y1, r, n;
    int last = argc - 1;
    VALUE options;
    texture_info tex;

    ADJUST_SELF(self);
    
    if(argc < 4) rb_raise(rb_eArgError, "ngon requires at least 4 parameters (x, y, radius, num_sides)");

    process_x_y_pairs(self, 1, argv, &x1, &y1);

    options = argv[last];

    get_texture_info(self, &tex);

    r = NUM2INT(argv[2]);
    n = NUM2INT(argv[3]);

    ngon_do_action(x1, y1, r, n, &tex, options, sync_mode, true, NULL);

    return self;
}
    

/* flood fill action */
VALUE
m_flood_fill(int argc, VALUE * argv, VALUE self)
{
    int x1, y1;
    int last = argc - 1;
    VALUE options;
    texture_info tex;
    bool iter = false, glow = false;
    
    ADJUST_SELF(self);

    if (argc < 1) rb_raise(rb_eArgError, "flood fill action needs at least 1 parameter");

    process_x_y_pairs(self, 1, argv, &x1, &y1);

    options = argv[last];

    get_texture_info(self, &tex);

    if(is_a_hash(options)) {
        if(RTEST(get_from_hash(options, "iter"))) 
            iter = true;
        if(RTEST(get_from_hash(options, "glow"))) 
            glow = true;
    }

    if(iter) {
        flood_fill_do_action(x1, y1, &tex, options, sync_mode, true, NULL);
    }
    else if(glow) {
        glow_fill_do_action(x1, y1, &tex, options, sync_mode, true, NULL);

    }
    /* this is the default fill */
    else {
        scan_fill_do_action(x1, y1, &tex, options, sync_mode, true, NULL);
    }
    
    return self;
}
    

/* line action */
VALUE
m_line(int argc, VALUE * argv, VALUE self) 
{
    int x1, y1, x2, y2;
    int last = argc - 1;
    VALUE options;
    texture_info tex;

    ADJUST_SELF(self);

    if(argc < 2) rb_raise(rb_eArgError, "line action needs at least 2 parameters");

    process_x_y_pairs(self, 2, argv, &x1, &y1, &x2, &y2);

    options = argv[last];

    get_texture_info(self, &tex);

    line_do_action(x1, y1, x2, y2, &tex, options, sync_mode, true, NULL);

    return self;
}

/* box action */
VALUE
m_rect(int argc, VALUE * argv, VALUE self) 
{

    int x1, y1, x2, y2;
    int last = argc - 1;
    VALUE options;
    texture_info tex;

    ADJUST_SELF(self);

    if(argc < 2) rb_raise(rb_eArgError, "rect action needs at least 2 parameters");

    process_x_y_pairs(self, 2, argv, &x1, &y1, &x2, &y2);

    options = argv[last];

    get_texture_info(self, &tex);
 
    rect_do_action(x1, y1, x2, y2, &tex, options, sync_mode, true, NULL);

    return self;
}


/* pixel action */
VALUE
m_pixel(int argc, VALUE * argv, VALUE self) 
{
    int x1, y1;
    int last = argc - 1;
    VALUE options;
    texture_info tex;
    
    ADJUST_SELF(self);

    if(argc < 1) rb_raise(rb_eArgError, "pixel action needs 1 parameter");

    process_x_y_pairs(self, 1, argv, &x1, &y1);

    options = argv[last];

    get_texture_info(self, &tex);
    
    pixel_do_action(x1, y1, &tex, options, sync_mode, true, NULL);

    return self;
}

/* bezier curve */
VALUE
m_bezier(int argc, VALUE * argv, VALUE self)
{
    VALUE points = Qnil;
    VALUE options = Qnil;
    int last = argc - 1;
    texture_info tex;

    ADJUST_SELF(self);

    if(argc < 1) rb_raise(rb_eArgError, "bezier action needs at least 1 parameter");
    
    /* get array of points */
    points = argv[0];
    Check_Type(points, T_ARRAY);
    
    options = argv[last];
    
    get_texture_info(self, &tex);

    bezier_do_action(points, &tex, options, sync_mode, true, NULL);

    return self;
}

/* bezier curve */
VALUE
m_polyline(int argc, VALUE * argv, VALUE self)
{
    VALUE points = Qnil;
    VALUE options = Qnil;
    int last = argc - 1;
    texture_info tex;

    ADJUST_SELF(self);

    if(argc < 1) rb_raise(rb_eArgError, "polyline action needs at least 1 parameter");

    /* get array of points */
    points = argv[0];
    Check_Type(points, T_ARRAY);

    options = argv[last];

    get_texture_info(self, &tex);

    polyline_do_action(points, &tex, options, sync_mode, true, NULL);

    return self;
}



/* splice action */
VALUE
m_splice(int argc, VALUE * argv, VALUE self) 
{
    int x0, y0;
    int cx1 = 0, cy1 = 0, cx2 = XMAX_OOB, cy2 = YMAX_OOB;
    texture_info splice_tex;
    int last = argc - 1;
    texture_info tex;
    VALUE options;

    ADJUST_SELF(self);

    if(argc < 3) rb_raise(rb_eArgError, "splice action needs at least 3 parameters");

    if(!is_gosu_image(argv[0]))
        rb_raise(rb_eArgError, "first parameter must be a valid Gosu::Image");

    /* get the splice image */
    get_texture_info(argv[0], &splice_tex);

    /* add 1 to argv to skip the Image parameter */
    process_x_y_pairs(self, 1, argv + 1, &x0, &y0);

    /* get the hash args */
    options = argv[last];

    get_texture_info(self, &tex);
    
    /* get the crop boundaries */
    if(is_a_hash(options)) 
        if(RTEST(get_from_hash(options, "crop"))) {
            VALUE c = get_from_hash(options, "crop");
            Check_Type(c, T_ARRAY);
            cx1 = NUM2INT(get_from_array(c, 0));
            cy1 = NUM2INT(get_from_array(c, 1));
            cx2 = NUM2INT(get_from_array(c, 2));
            cy2 = NUM2INT(get_from_array(c, 3));
        }

    splice_do_action(x0, y0, cx1, cy1, cx2, cy2, &splice_tex,
                      &tex, options, sync_mode, true, NULL);
    
    return self;
}


/* clear action - really just an alias for box */
VALUE
m_clear(int argc, VALUE * argv, VALUE self) 
{
    VALUE parms[4];

    parms[0] = INT2NUM(0);
    parms[1] = INT2NUM(0);
    parms[2] = INT2NUM(XMAX_OOB);
    parms[3] = INT2NUM(YMAX_OOB);

    //  m_box(ARY_SIZE(parms), parms, self);

    return self;
}

/* offset function */
VALUE
m_offset(int argc, VALUE * argv, VALUE self) 
{    
    char * try_offset;

    ADJUST_SELF(self);

    if(argc == 0)
        return get_image_local(self, DRAW_OFFSET);
    
    switch(TYPE(argv[0])) {

    case T_ARRAY:

        set_image_local(self, DRAW_OFFSET, argv[0]);
        break;
    case T_SYMBOL:
        try_offset = sym2string(argv[0]);
        
        if(!strcmp("default", try_offset)) {
            set_image_local(self, DRAW_OFFSET, Qnil);
        }
        else {            
            rb_raise(rb_eArgError, "no such offset defined: %s\n", try_offset);
        }

        break;
    default:
        rb_raise(rb_eArgError, "invalid offset. please use an array or :default.");
    }
    return Qnil;
}

/* color change */
VALUE
m_color(int argc, VALUE * argv, VALUE self) 
{
    VALUE first = argv[0];
    rgba new_color;

    ADJUST_SELF(self);

    /* if no params then return action current color */
    if(argc == 0) 
        return get_image_local(self, IMAGE_COLOR);
    
    /* otherwise set the action color */
    /* NB: we cannot just set image_local_color to 'first' because first may not be an array,
       it could also be a symbol */
    
    new_color = convert_rb_color_to_rgba(first);

    /* im quite sure i DO want to set the color even if it is not_a_color.
       why ? consistency only
       (NB: not_a_color_v is skipped by the set_pixel_color routine */
    
    /* if(is_a_color(new_color)) */
    
    save_rgba_to_image_local_color(self, new_color);
    
    return Qnil;
}

/* this function manages all other method calls */
VALUE
m_missing(int argc, VALUE * argv, VALUE self) 
{
    char * action_name = lowercase(sym2string(argv[0]));

    /* try case insensitive version of action name */
    if(rb_respond_to(self, rb_intern(action_name))) {
        rb_funcall2(self, rb_intern(action_name), --argc, ++argv);
    }
    /* still no match? then method does not exist */
    else {
        rb_raise (rb_eRuntimeError, "unrecognized action: %s\n", action_name);
    }

    return self;
}

/* refreshes the cache */
VALUE
m_cache_refresh(VALUE self) 
{
    texture_info tex;

    ADJUST_SELF(self);

    get_texture_info(self, &tex);

    cache_refresh_entry(tex.tname);

    return self;
}

/* check whether img quad is already cached */
VALUE
m_quad_cached(VALUE self) 
{
    VALUE info, gc_state_off;
    int tex_name;
    cache_entry * entry;

    ADJUST_SELF(self);

    /* prevent weird segfault bug */
    gc_state_off = rb_gc_disable();

    /* ensure gl_tex_info returns non nil */
    info = check_for_texture_info(self);

    tex_name = FIX2INT(rb_funcall(info, rb_intern("tex_name"), 0));

    entry = find_in_cache(tex_name);
    
    /* only enable gc if was enabled on function entry */
    if(!gc_state_off) rb_gc_enable();

    return entry ? Qtrue : Qfalse;
}

/** m_each **/
VALUE
m_each(int argc, VALUE * argv, VALUE self)
{
    int x1 = 0, y1 = 0, x2 = XMAX_OOB, y2 = YMAX_OOB;
    texture_info tex;
    VALUE proc;
    VALUE options = argv[0];

    rb_need_block();

    ADJUST_SELF(self);

    get_texture_info(self, &tex);

    if(argc == 1) {
        Check_Type(options, T_HASH);
        if(RTEST(get_from_hash(options, "region"))) {
            VALUE region = get_from_hash(options, "region");
            Check_Type(region, T_ARRAY);

            if(RARRAY_LEN(region) < 4)
                rb_raise(rb_eArgError, "region requires 4 elements");
            
            x1 = NUM2INT(get_from_array(region, 0));
            y1 = NUM2INT(get_from_array(region, 1));
            x2 = NUM2INT(get_from_array(region, 2));
            y2 = NUM2INT(get_from_array(region, 3));

        }

    }

    constrain_boundaries(&x1, &y1,
                         &x2, &y2, tex.width, tex.height);

    proc = rb_block_proc();
    
    each_pixel_do_action(x1, y1, x2, y2, proc, &tex, options, sync_mode, true, NULL);

    return self;
}
/** end of each **/

/** turtle drawing functions **/
/* static VALUE */
/* m_turtle_move_to */

/* VALUE */
/* m_bezier(int argc, VALUE * argv, VALUE self) */
/* { */
/*     VALUE points = Qnil; */
/*     VALUE options = Qnil; */
/*     int last = argc - 1; */
/*     texture_info tex; */

/*     ADJUST_SELF(self); */

/*     if(argc < 1) rb_raise(rb_eArgError, "bezier action needs at least 1 parameter"); */
    
/*     /\* get array of points *\/ */
/*     points = argv[0]; */
/*     Check_Type(points, T_ARRAY); */
    
/*     options = argv[last]; */
    
/*     get_texture_info(self, &tex); */

/*     bezier_do_action(points, &tex, options, sync_mode, true, NULL); */

/*     return self; */
/* } */

/** end turtle drawing **/


/* below is yucky old code that needs to be updated */
/* each_pixel iterator */
            
            
/* VALUE */
/* m_each(int argc, VALUE * argv, VALUE self) */
/* { */
/*     int x0, y0, x1, y1, xbound, ybound, arity; */
/*     VALUE options, region, pixel_data[2], yield_vals; */
/*     register int x, y; */
/*     texture_info tex; */
/*     image_bounds bounds; */

/*     rb_need_block(); */

/*     arity = FIX2INT(rb_funcall(rb_block_proc(), rb_intern("arity"), 0)); */
/*     if(arity != 1 && arity != 3) */
/*         rb_raise(rb_eRuntimeError, "block arity must be either 1 or 3"); */

/* /\*     ADJUST_SELF(self); *\/ */

/* /\*     rb_scan_args(argc, argv, "01", &options); *\/ */
    
/* /\*     /\\* get texture info *\\/ *\/ */
/* /\*     get_texture_info(self, &tex); *\/ */

/* /\*     /\\* default values for region *\\/ *\/ */
/* /\*     x0 = 0; y0 = 0; x1 = tex.width; y1 = tex.height; *\/ */

/* /\*     if(has_optional_hash_arg(options, "region")) { *\/ */
/* /\*         region = get_from_hash(options, "region"); *\/ */

/* /\*         x0 = NUM2INT(get_from_array(region, 0)); *\/ */
/* /\*         y0 = NUM2INT(get_from_array(region, 1)); *\/ */
/* /\*         x1 = NUM2INT(get_from_array(region, 2)); *\/ */
/* /\*         y1 = NUM2INT(get_from_array(region, 3)); *\/ */
        
/* /\*         constrain_boundaries(&x0, &y0, &x1, &y1, tex.width, tex.height); *\/ */
/* /\*     } *\/ */

/* /\*     /\\* width and height of action *\\/ *\/ */
/* /\*     xbound = x1 - x0; *\/ */
/* /\*     ybound = y1 - y0; *\/ */
    
/* /\*     yield_vals = rb_ary_new(); *\/ */

/* /\*     for(y = 0; y < ybound; y++) *\/ */
/* /\*         for(x = 0; x < xbound; x++) { *\/ */
/* /\*             VALUE pixel_color; *\/ */
/* /\*             rgba old_color; *\/ */

/* /\*             /\\* adjusted x and y *\\/ *\/ */
/* /\*             register int ax = x + x0, ay = y + y0; *\/ */
            
/* /\*             pixel_data[0] = INT2FIX(ax); *\/ */
/* /\*             pixel_data[1] = INT2FIX(ay); *\/ */
            
/* /\*             pixel_color = m_getpixel(self, INT2FIX(ax), INT2FIX(ay)); *\/ */

/* /\*             if(arity == 1) { *\/ */
/* /\*                 rb_yield(pixel_color); *\/ */
/* /\*             } *\/ */
/* /\*             else if(arity == 3) { *\/ */
/* /\*                 rb_ary_store(yield_vals, 0, pixel_color); *\/ */
/* /\*                 rb_ary_store(yield_vals, 1, INT2FIX(x)); *\/ */
/* /\*                 rb_ary_store(yield_vals, 2, INT2FIX(y)); *\/ */
                
/* /\*                 rb_yield(yield_vals); *\/ */
/* /\*             } *\/ */

/* /\*             m_color(1, &pixel_color, self); *\/ */
/* /\*             //            process_action(pixel, self, 2, pixel_data, false); *\/ */
/* /\*             //            color_struct = old_color; *\/ */
/* /\*         } *\/ */

/* /\*     bounds.xmin = x0; *\/ */
/* /\*     bounds.ymin = y0; *\/ */
/* /\*     bounds.xmax = x1; *\/ */
/* /\*     bounds.ymax = y1; *\/ */

/* /\*     create_subtexture_and_sync_to_gl(&bounds, &tex); *\/ */

/* /\*     return self; *\/ */
/* /\* } *\/ */

/* /\** end each_pixel algorithm **\/} */
/* /\** end each_pixel algorithm **\/ */


/* /\* VALUE *\/ */
/* /\* m_lshift(int argc, VALUE * argv, VALUE self)  *\/ */
/* /\* { *\/ */
/* /\*     int y,x, step, yoffset; *\/ */
/* /\*     VALUE options, loop; *\/ */
/* /\*     register int offset; *\/ */
/* /\*     texture_info tex; *\/ */
/* /\*     image_bounds bounds; *\/ */

/* /\*     ADJUST_SELF(self); *\/ */

/* /\*     rb_scan_args(argc, argv, "01", &options); *\/ */

/* /\*     /\\* default values for other params *\\/ *\/ */
/* /\*     step = 1; loop = Qfalse; *\/ */

/* /\*     if(TYPE(options) == T_HASH) { *\/ */
/* /\*         step = NUM2INT(get_from_hash(options, "step")); *\/ */
/* /\*         loop = get_from_hash(options, "loop"); *\/ */
/* /\*     } *\/ */
/* /\*     else if(options != Qnil) { *\/ */
/* /\*         rb_raise(rb_eArgError, "argument must be a hash"); *\/ */
/* /\*     } *\/ */
    
/* /\*     /\\* get texture info *\\/ *\/ */
/* /\*     get_texture_info(self, &tex); *\/ */
    
/* /\*     for(y = 0; y < tex.height; y++) { *\/ */
/* /\*         for(x = 0; x < tex.width; x++) { *\/ */
/* /\*             offset = calc_pixel_offset(&tex, x, y); *\/ */

/* /\*             if((x + step) < tex.width)  { *\/ */
/* /\*                 color_copy(tex.td_array + offset + step * 4, tex.td_array + offset); *\/ */
/* /\*             } *\/ */
/* /\*             else { *\/ */
/* /\*                 if(loop == Qtrue) { *\/ */
/* /\*                     yoffset = calc_pixel_offset(&tex, x + step - tex.width, y); *\/ */

/* /\*                     color_copy(tex.td_array + yoffset, tex.td_array + offset); *\/ */
/* /\*                 } *\/ */
/* /\*                 else { *\/ */

/* /\*                     zero_color(tex.td_array + offset); *\/ */
/* /\*                 } *\/ */
/* /\*             } *\/ */

/* /\*         } *\/ */
/* /\*     } *\/ */

/* /\*     bounds.xmin = 0; *\/ */
/* /\*     bounds.xmax = tex.width; *\/ */
/* /\*     bounds.ymin = 0; *\/ */
/* /\*     bounds.ymax = tex.height; *\/ */

/* /\*     create_subtexture_and_sync_to_gl(&bounds, &tex); *\/ */

/* /\*     return Qnil; *\/ */
/* /\* } *\/ */

/* /\* VALUE *\/ */
/* /\* m_rshift(int argc, VALUE * argv, VALUE self)  *\/ */
/* /\* { *\/ */
/* /\*     int y,x, step, yoffset; *\/ */
/* /\*     VALUE options, loop; *\/ */
/* /\*     register int offset; *\/ */
/* /\*     texture_info tex; *\/ */
/* /\*     image_bounds bounds; *\/ */

/* /\*     ADJUST_SELF(self); *\/ */

/* /\*     rb_scan_args(argc, argv, "01", &options); *\/ */
    
/* /\*     /\\* default values for other params *\\/ *\/ */
/* /\*     step = 1; loop = Qfalse; *\/ */

/* /\*     if(TYPE(options) == T_HASH) { *\/ */
/* /\*         step = NUM2INT(get_from_hash(options, "step")); *\/ */
/* /\*         loop = get_from_hash(options, "loop"); *\/ */
/* /\*     } *\/ */
/* /\*     else if(options != Qnil) { *\/ */
/* /\*         rb_raise(rb_eArgError, "argument must be a hash"); *\/ */
/* /\*     } *\/ */
    
/* /\*     /\\* get texture info *\\/ *\/ */
/* /\*     get_texture_info(self, &tex); *\/ */

/* /\*     for(y = 0; y < tex.height; y++) { *\/ */
/* /\*         for(x = tex.width - 1; x > -1; x--) { *\/ */
/* /\*             offset = calc_pixel_offset(&tex, x, y); *\/ */

/* /\*             if((x - step) > -1) { *\/ */
/* /\*                 color_copy(tex.td_array + offset - step * 4, tex.td_array + offset); *\/ */
/* /\*             } *\/ */
/* /\*             else { *\/ */
/* /\*                 if(loop == Qtrue) { *\/ */
/* /\*                     yoffset = calc_pixel_offset(&tex, x + tex.width - step, y); *\/ */
/* /\*                     color_copy(tex.td_array + yoffset, tex.td_array + offset); *\/ */
/* /\*                 } *\/ */
/* /\*                 else { *\/ */
/* /\*                     zero_color(tex.td_array + offset); *\/ */
/* /\*                 } *\/ */
/* /\*             } *\/ */

/* /\*         } *\/ */
/* /\*     } *\/ */

/* /\*     bounds.xmin = 0; *\/ */
/* /\*     bounds.xmax = tex.width; *\/ */
/* /\*     bounds.ymin = 0; *\/ */
/* /\*     bounds.ymax = tex.height; *\/ */

/* /\*     create_subtexture_and_sync_to_gl(&bounds, &tex); *\/ */

/* /\*     return Qnil; *\/ */
/* /\* } *\/ */

/* /\* /\\* special pixel action for image[]= *\\/ *\/ */
/* /\* VALUE *\/ */
/* /\* m_special_pixel(int argc, VALUE * argv, VALUE self) *\/ */
/* /\* { *\/ */

/* /\*     rgba old_color; *\/ */

/* /\*     ADJUST_SELF(self); *\/ */

/* /\*     if(argc < 3) rb_raise(rb_eArgError, "[]= action needs 3 parameters"); *\/ */

/* /\*     m_color(1, &argv[2], self); *\/ */

/* /\*     m_pixel(2, argv, self); *\/ */

/* /\*     //    color_struct = old_color; *\/ */


/* /\*     return Qnil; *\/ */
/* /\* } *\/ */

/* /\* /\\* instance methods *\\/ *\/ */

