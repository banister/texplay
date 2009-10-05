#ifndef GUARD_ACTIONS_H
#define GUARD_ACTIONS_H


/* lines */
void line_do_action(int, int, int, int, texture_info *, VALUE, sync, bool primary, action_struct * payload);

/* circles */
void circle_do_action(int, int, int, texture_info *, VALUE, sync, bool primary, action_struct * payload);

/* pixels */
void pixel_do_action(int, int, texture_info *, VALUE, sync, bool primary, action_struct * payload);

/* rectangles */
void rect_do_action(int x1, int y1, int x2, int y2, texture_info * tex, VALUE hash_arg,
                    sync sync_mode, bool primary, action_struct * payload);

/* flood fill */
void flood_fill_do_action(int x, int y, texture_info * tex, VALUE hash_arg, sync sync_mode, bool primary,
                          action_struct * payload);

/* glow fill */
void glow_fill_do_action(int x, int y, texture_info * tex, VALUE hash_arg, sync sync_mode, bool primary,
                         action_struct * payload);

/* scan fill */
void scan_fill_do_action(int x, int y, texture_info * tex, VALUE hash_arg,
                         sync sync_mode, bool primary, action_struct * payload);

/* polyline */
void polyline_do_action(VALUE points, texture_info * tex, VALUE hash_arg, sync sync_mode, bool primary,
                        action_struct * payload);

/* bezier */
void bezier_do_action(VALUE points, texture_info * tex, VALUE hash_arg, sync sync_mode, bool primary,
                      action_struct * payload);

/* ngon */
void ngon_do_action(int x, int y, int r, int num_sides, texture_info * tex, VALUE hash_arg,
                    sync sync_mode, bool primary, action_struct * payload);


/* splice */
void splice_do_action(int x0,int y0, int cx1, int cy1, int cx2, int cy2, texture_info * splice_tex,
                      texture_info * tex, VALUE hash_arg, sync sync_mode,
                      bool primary, action_struct * payload);

/* each iterator */
void each_pixel_do_action(int x1, int y1, int x2, int y2, VALUE proc, texture_info * tex, VALUE hash_arg,
                          sync sync_mode, bool primary, action_struct * payload);

#endif
