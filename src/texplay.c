/* texplay.c, light-weight alternative to rmagick for ruby */
/* (C) John Mair 2009
 * This program is distributed under the terms of the MIT License
 *                                                                */

#include <ruby.h>
#include <stdio.h>
#include <time.h>
#include "texplay.h"
#include "actions.h"
#include "utils.h"
#include "bindings.h"
#include "object2module.h"
#include "gen_eval.h"

/* setup ruby bindings */

/** constructor for TPPoint class **/
static VALUE m_init_TPPoint(int argc, VALUE * argv, VALUE self);

/** methods for EmptyImageStub **/
static VALUE m_init_EmptyImageStub(int argc, VALUE * argv, VALUE self);
static VALUE m_EmptyImageStub_columns(VALUE self);
static VALUE m_EmptyImageStub_rows(VALUE self);
static VALUE m_EmptyImageStub_to_blob(VALUE self);
/** end of EmptyImageStub prototypes **/

void
Init_ctexplay() {

    VALUE jm_Module = rb_define_module("TexPlay");
    VALUE TPPoint = rb_define_class_under(jm_Module, "TPPoint", rb_cObject);
    VALUE EmptyImageStub = rb_define_class_under(jm_Module, "EmptyImageStub", rb_cObject);

    /** define basic point class TPPoint **/
    rb_attr(TPPoint, rb_intern("x"), 1, 1, Qtrue);
    rb_attr(TPPoint, rb_intern("y"), 1, 1, Qtrue);
    rb_define_method(TPPoint, "initialize", m_init_TPPoint, -1);
    /** end of TPPoint definition **/

    /** define EmptyImageStub class for creating blank images **/
    rb_define_method(EmptyImageStub, "rows", m_EmptyImageStub_rows, 0);
    rb_define_method(EmptyImageStub, "columns", m_EmptyImageStub_columns, 0);
    rb_define_method(EmptyImageStub, "to_blob", m_EmptyImageStub_to_blob, 0);
    rb_define_method(EmptyImageStub, "initialize", m_init_EmptyImageStub, -1);
    /** end EmptyImageStub definition **/

    /* TexPlay methods */
    rb_define_method(jm_Module, "paint", m_paint, -1);
    rb_define_method(jm_Module, "get_pixel", m_getpixel, -1);
    rb_define_method(jm_Module, "circle", m_circle, -1);
    rb_define_method(jm_Module, "line", m_line, -1);
    rb_define_method(jm_Module, "rect", m_rect, -1);
    rb_define_method(jm_Module, "clear", m_clear, -1);
    rb_define_method(jm_Module, "pixel", m_pixel, -1);
    rb_define_method(jm_Module, "fill", m_flood_fill, -1);
    rb_define_method(jm_Module, "bezier", m_bezier, -1);
    rb_define_method(jm_Module, "polyline", m_polyline, -1);
    rb_define_method(jm_Module, "ngon", m_ngon, -1);
    
    rb_define_method(jm_Module, "splice", m_splice, -1);
    
    rb_define_method(jm_Module, "color", m_color, -1);
    rb_define_method(jm_Module, "offset", m_offset, -1);
    rb_define_method(jm_Module, "method_missing", m_missing, -1);
    rb_define_method(jm_Module, "quad_cached?", m_quad_cached, 0);

    rb_define_method(jm_Module, "each", m_each, -1);
    
    /* needs to be updated, not yet done **/
    /* rb_define_method(jm_Module, "bitmask", m_bitmask, -1); */
    /* rb_define_method(jm_Module, "leftshift", m_lshift, -1); */
    /* rb_define_method(jm_Module, "rightshift", m_rshift, -1); */
    /* rb_define_method(jm_Module, "[]=", m_special_pixel, -1); */

    rb_define_method(jm_Module, "dup", m_dup_image, 0);
    rb_define_method(jm_Module, "clone", m_clone_image, 0);
    rb_define_method(jm_Module, "force_sync", m_force_sync, 1);
    rb_define_method(jm_Module, "set_options", m_user_set_options, 1);
    rb_define_method(jm_Module, "get_options", m_get_options, 0);
    rb_define_method(jm_Module, "delete_options", m_user_delete_options, 0);

    rb_define_method(jm_Module, "refresh_cache", m_cache_refresh, 0);
    
    /* a constant containing the sidelength of largest allowable quad */
    rb_define_const(jm_Module, "TP_MAX_QUAD_SIZE", INT2FIX(max_quad_size()));

    /* singleton method for creating & removing macros */
    rb_define_singleton_method(jm_Module, "create_macro", M_create_macro, 1);
    rb_define_singleton_method(jm_Module, "remove_macro", M_remove_macro, 1);
    rb_define_singleton_method(jm_Module, "refresh_cache_all", M_refresh_cache_all, 0);
    rb_define_singleton_method(jm_Module, "create_blank_image", M_create_blank, 3);

    /** aliases; must be made on singleton class because we're using class methods **/
    rb_define_method(jm_Module, "box", m_rect, -1);
    rb_define_method(jm_Module, "colour", m_color, -1);
    rb_define_method(jm_Module, "composite", m_splice, -1);
    rb_define_method(jm_Module, "set_pixel", m_pixel, -1);
    rb_define_method(jm_Module, "[]", m_getpixel, 2);
    /** end of aliases **/

    /** associated with gen_eval **/
    rb_define_method(rb_cObject, "gen_eval", rb_gen_eval, -1);
    rb_define_method(rb_cObject, "capture", rb_capture, 0);
    
    rb_define_method(rb_cObject, "to_module", rb_to_module , 0);
    rb_define_method(rb_cObject, "reset_tbls", rb_reset_tbls , 0);
    rb_define_method(rb_cObject, "gen_extend", rb_gen_extend, -1);
    rb_define_method(rb_cModule, "gen_include", rb_gen_include, -1);

    /* below is much too hard to achieve in pure C */
    rb_eval_string("class Proc;"
                   "    def __context__;"
                   "        eval('self', self.binding);"
                   "    end;"
                   "end;"
                   );
    
    rb_define_alias(rb_cObject, "gen_eval_with", "gen_eval");
    /** end of gen_eval defs **/


    /** basic setup **/

    /* seed the random number generator */
    srand(time(NULL));

    /** end basic setup **/
}

/** constructor for TPPoint class **/
static VALUE
m_init_TPPoint(int argc, VALUE * argv, VALUE self)
{
    if(argc == 0) {
        rb_iv_set(self, "@x", INT2FIX(0));
        rb_iv_set(self, "@y", INT2FIX(0));
    }
    else if(argc == 2){
        if(is_a_num(argv[0]) && is_a_num(argv[1])) {
            rb_iv_set(self, "@x", argv[0]);
            rb_iv_set(self, "@y", argv[1]);
        }
        else
            rb_raise(rb_eArgError, "must provide two numbers");
    }
    else
        rb_raise(rb_eArgError, "please provide x and y args only");

    return Qnil;
    
}
/** end constructor for TPPoint **/

/** methods for EmptyImageStub **/
static VALUE
m_init_EmptyImageStub(int argc, VALUE * argv, VALUE self)
{
    VALUE width = argv[0];
    VALUE height = argv[1];
    
    if(argc == 2) {
        if(!is_a_num(argv[0]) || !is_a_num(argv[1]))
            rb_raise(rb_eArgError, "must provide two numbers (width and height)");
        
        rb_iv_set(self, "@w", width);
        rb_iv_set(self, "@h", height);
    }
    else
        rb_raise(rb_eArgError, "invalid arguments, provide either an (x, y) or an image to dup");
        
    return Qnil;
}

static VALUE
m_EmptyImageStub_columns(VALUE self)
{
    return rb_iv_get(self, "@w");
}

static VALUE
m_EmptyImageStub_rows(VALUE self)
{
    return rb_iv_get(self, "@h");
}

static VALUE
m_EmptyImageStub_to_blob(VALUE self)
{

    int width = NUM2INT(rb_iv_get(self, "@w"));
    int height = NUM2INT(rb_iv_get(self, "@h"));
    int size = width * height * 4;
    char buf[size];

    memset(buf, 0, size);
        
    return rb_str_new(buf, size);
}
/** end methods for EmptyImageStub **/
    
            
    




