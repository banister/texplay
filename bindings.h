#ifndef GUARD_BINDINGS_H
#define GUARD_BINDINGS_H

/* class methods */
VALUE M_create_macro(VALUE self , VALUE method_name);
VALUE M_remove_macro(VALUE self, VALUE method_name);
VALUE M_refresh_cache_all(VALUE self);
VALUE M_create_blank(VALUE self, VALUE window, VALUE width, VALUE height);

/* instance methods */
VALUE m_paint(int argc, VALUE * argv, VALUE self);
VALUE m_getpixel(int argc, VALUE * argv, VALUE self);
VALUE m_circle(int argc, VALUE * argv, VALUE self);
VALUE m_line(int argc, VALUE * argv, VALUE self);
VALUE m_rect(int argc, VALUE * argv, VALUE self);
VALUE m_pixel(int argc, VALUE * argv, VALUE self);
VALUE m_flood_fill(int argc, VALUE * argv, VALUE self);
VALUE m_bezier(int argc, VALUE * argv, VALUE self);
VALUE m_polyline(int argc, VALUE * argv, VALUE self);
VALUE m_ngon(int argc, VALUE * argv, VALUE self);
VALUE m_special_pixel(int argc, VALUE * argv, VALUE self);
VALUE m_splice(int argc, VALUE * argv, VALUE self);
VALUE m_clear(int argc, VALUE * argv, VALUE self);
VALUE m_offset(int argc, VALUE * argv, VALUE self);
VALUE m_color(int argc, VALUE * argv, VALUE self);
VALUE m_missing(int argc, VALUE * argv, VALUE self);
VALUE m_bitmask(int argc, VALUE * argv, VALUE self);
VALUE m_lshift(int argc, VALUE * argv, VALUE self);
VALUE m_rshift(int argc, VALUE * argv, VALUE self);

VALUE m_each(int argc, VALUE * argv, VALUE self);


VALUE m_quad_cached(VALUE self);
VALUE m_cache_refresh(VALUE self);

VALUE m_user_set_options(VALUE self, VALUE options);
VALUE m_user_delete_options(VALUE self);
VALUE m_get_options(VALUE self);
VALUE m_force_sync(VALUE self, VALUE ary);

VALUE m_dup_image(VALUE self);



#endif
