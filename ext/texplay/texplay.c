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
#ifdef __APPLE__
# include <AvailabilityMacros.h>
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_9
# include <GLUT/glut.h>
#else
# include <GL/glut.h>
#endif
#else
# include <GL/glut.h>
#endif


/* setup ruby bindings */

/** constructor for TPPoint class **/
static VALUE m_init_TPPoint(int argc, VALUE * argv, VALUE self);
static void monkey_patch_gosu(void);
static VALUE gosu_window(void);

void
Init_texplay() {

  VALUE jm_Module = rb_define_module("TexPlay");
  VALUE TPPoint = rb_define_class_under(jm_Module, "TPPoint", rb_cObject);

  /** define basic point class TPPoint **/
  rb_attr(TPPoint, rb_intern("x"), 1, 1, Qtrue);
  rb_attr(TPPoint, rb_intern("y"), 1, 1, Qtrue);
  rb_define_method(TPPoint, "initialize", m_init_TPPoint, -1);
  /** end of TPPoint definition **/

  /* TexPlay methods */
  rb_define_method(jm_Module, "paint", m_paint, -1);
  rb_define_method(jm_Module, "get_pixel", m_getpixel, -1);
  rb_define_method(jm_Module, "circle", m_circle, -1);
  rb_define_method(jm_Module, "line", m_line, -1);
  rb_define_method(jm_Module, "rect", m_rect, -1);
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
  rb_define_method(jm_Module, "to_blob", m_to_blob, 0);
  rb_define_method(jm_Module, "force_sync", m_force_sync, 1);
  rb_define_method(jm_Module, "set_options", m_user_set_options, 1);
  rb_define_method(jm_Module, "get_options", m_get_options, 0);
  rb_define_method(jm_Module, "delete_options", m_user_delete_options, 0);

  rb_define_method(jm_Module, "refresh_cache", m_cache_refresh, 0);
    
  /* a constant containing the sidelength of largest allowable quad */
  rb_define_const(jm_Module, "TP_MAX_QUAD_SIZE", INT2FIX(max_quad_size() - 2));

  /* singleton method for creating & removing macros */
  rb_define_singleton_method(jm_Module, "create_macro", M_create_macro, 1);
  rb_define_singleton_method(jm_Module, "remove_macro", M_remove_macro, 1);
  rb_define_singleton_method(jm_Module, "refresh_cache_all", M_refresh_cache_all, 0);
  /* rb_define_singleton_method(jm_Module, "create_blank_image", M_create_blank, 3); */

  /** aliases; must be made on singleton class because we're using class methods **/
  rb_define_method(jm_Module, "box", m_rect, -1);
  rb_define_method(jm_Module, "colour", m_color, -1);
  rb_define_method(jm_Module, "composite", m_splice, -1);
  rb_define_method(jm_Module, "set_pixel", m_pixel, -1);
  rb_define_method(jm_Module, "[]", m_getpixel, -1);
  rb_define_method(jm_Module, "cache", m_cache_refresh, 0);
  /** end of aliases **/

  /** basic setup **/

  /* seed the random number generator */
  srand(time(NULL));

  monkey_patch_gosu();
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


static VALUE
gosu_window_to_blob(VALUE self, VALUE rb_x, VALUE rb_y, VALUE rb_width, VALUE rb_height)
{
  int x = FIX2INT(rb_x);
  int y = FIX2INT(rb_y);
  int width = FIX2INT(rb_width);
  int height = FIX2INT(rb_height);
  int window_height = FIX2INT(rb_funcall(self, rb_intern("height"), 0));

  VALUE blob = rb_str_new(NULL, 4 * width * height);
    
  rb_funcall(self, rb_intern("flush"), 0);
  glFinish();
    
  glReadPixels(x, y, width, window_height - y, GL_RGBA,
               GL_UNSIGNED_BYTE, RSTRING_PTR(blob));
    
  return blob;
}

static VALUE
gosu_window_to_texture(VALUE self, VALUE rb_tex_name, VALUE rb_xoffset, VALUE rb_yoffset,
                       VALUE rb_x, VALUE rb_y, VALUE rb_width, VALUE rb_height)
{

  int tex_name = FIX2INT(rb_tex_name);
  int xoffset = FIX2INT(rb_xoffset);
  int yoffset = FIX2INT(rb_yoffset);
  int x = FIX2INT(rb_x);
  int y = FIX2INT(rb_y);
  int width = FIX2INT(rb_width);
  int height = FIX2INT(rb_height);
    
  rb_funcall(self, rb_intern("flush"), 0);
  glFinish();

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, tex_name);
  
  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, x, y, width, height);
  glDisable(GL_TEXTURE_2D);  
  
  return Qnil;
}


static void
monkey_patch_gosu(void)
{
  rb_define_method(gosu_window(), "to_blob", gosu_window_to_blob, 4);
  rb_define_method(gosu_window(), "to_texture", gosu_window_to_texture, 7);
}    

static VALUE
gosu_window(void)
{
  static VALUE GosuWindow = 0;

  if (!GosuWindow) {
    VALUE Gosu = rb_const_get(rb_cObject, rb_intern("Gosu"));
    GosuWindow = rb_const_get(Gosu, rb_intern("Window"));
  }

  return GosuWindow;
}
    

  
                                  
                                  




