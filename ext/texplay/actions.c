#include "texplay.h"
#include "utils.h"
#include "graphics_utils.h"
#include "actions.h"
#include <assert.h>
#include <math.h>
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


/** line_do_action, bresenham's algorithm **/
typedef enum { no_mode, until_color, while_color} trace_mode_type;

/* utility func to manage both kinds of color comparions */
static bool
cmp_color_with_or_without_tolerance(rgba c1, rgba c2, action_struct * payload)
{
  return payload->pen.has_tolerance ? cmp_color_with_tolerance(c1, c2, payload->pen.tolerance) : cmp_color(c1, c2);
}

static inline bool
is_trace_match(action_struct * cur, rgba c, rgba trace_color, trace_mode_type trace_mode)
{
  if(trace_mode == while_color) {
      if(!cmp_color_with_or_without_tolerance(c, trace_color, cur))
        return true;
  }
  else if(trace_mode == until_color) {
      if(cmp_color_with_or_without_tolerance(c, trace_color, cur))
        return true;
  }

  return false;
}

trace_match
line_do_action(int x1, int y1, int x2, int y2, texture_info * tex, VALUE hash_arg,
               sync sync_mode, bool primary, action_struct * payload)
{
    int x, y, W, H, F;
    int xinc, yinc;
    action_struct cur;
    int thickness = 1;

    bool has_trace = false;
    rgba trace_color;
    trace_mode_type trace_mode = no_mode;
    
    if(has_optional_hash_arg(hash_arg, "thickness")) 
        thickness = NUM2INT(get_from_hash(hash_arg, "thickness"));
    
    if(has_optional_hash_arg(hash_arg, "trace") && primary) {
      VALUE trace_hash = get_from_hash(hash_arg, "trace");

      /* we're tracing (not drawing) */
      has_trace = true;

      /* since we're not drawing, no need to sync */
      sync_mode = no_sync;

      if(has_optional_hash_arg(trace_hash, "until_color")) {
        VALUE c = get_from_hash(trace_hash, "until_color");
        trace_color = convert_rb_color_to_rgba(c);
        trace_mode = until_color;
      }
      else if(has_optional_hash_arg(trace_hash, "while_color")) {
        VALUE c = get_from_hash(trace_hash, "while_color");
        trace_color = convert_rb_color_to_rgba(c);
        trace_mode = while_color;
      }
    }

    draw_prologue(&cur, tex, x1, y1,
                  x2, y2, &hash_arg, sync_mode, primary, &payload);
    

    /* clip the line */
    cohen_sutherland_clip(&x1, &y1, &x2, &y2, 0, 0, tex->width - 1, tex->height - 1);
    
    W = ABS(x2 - x1);
    H = ABS(y2 - y1);
    	
    if(x1 < x2)
        xinc = 1;
    else
        xinc = -1;
		
    if(y1 < y2)
        yinc = 1;
    else
        yinc = -1;
		
    x = x1;
    y = y1;
	
    if(W >= H) {
        F = 2 * H - W;
        while(x != x2) {
           if(thickness <= 1) {
             if(!has_trace)
                set_pixel_color_with_style(payload, tex, x, y);
             else {
               rgba c = get_pixel_color(tex, x, y);

               if (is_trace_match(payload, c, trace_color, trace_mode)) 
                 return (trace_match) { x, y, c };
             }
            }
            else {
                set_hash_value(hash_arg, "fill", Qtrue);
                circle_do_action(x, y, thickness / 2, tex, hash_arg, no_sync, false, payload);
            }
           
            if(F < 0)
                F += 2 * H;
            else {
                F += 2 * (H - W);
                y += yinc;
            }
            x += xinc;
        }
    }
    else {
        F = 2 * W - H;
        while(y != y2 ) {

            if(thickness <= 1) {
             if(!has_trace)
                set_pixel_color_with_style(payload, tex, x, y);
             else {
               rgba c = get_pixel_color(tex, x, y);

               if (is_trace_match(payload, c, trace_color, trace_mode)) 
                 return (trace_match) { x, y, c };
             }                
            }
            else {
                set_hash_value(hash_arg, "fill", Qtrue);
                circle_do_action(x, y, thickness / 2, tex, hash_arg, no_sync, false, payload);
            }
            
            if(F < 0)
                F += 2 * W;
            else {
                F += 2 * (W - H);
                x += xinc;
            }
            y += yinc;
        }
    }

    if(thickness <= 1) {
        if(!has_trace)
          set_pixel_color_with_style(payload, tex, x2, y2);
        else {
          rgba c = get_pixel_color(tex, x, y);
          
          if (is_trace_match(payload, c, trace_color, trace_mode)) 
            return (trace_match) { x, y, c };
        }        
    }
    else {
        set_hash_value(hash_arg, "fill", Qtrue);
        circle_do_action(x2, y2, thickness / 2, tex, hash_arg, no_sync, false, payload);
                
    }
    draw_epilogue(&cur, tex, primary);

    return (trace_match) { .x = -9999, .y = -9999, .color = not_a_color_v };
}
/** end line **/

/** polyline algorithm **/

/* used by both polyline and bezier */
#define SIMPLE_FORMAT 0
#define POINT_FORMAT 1

/* calculate a single point */
static void
polyline_point(VALUE points, int k, int * x, int * y, int format, int draw_offset_x,
               int draw_offset_y)
{
    int xy_index;
    
    switch(format) {
    case POINT_FORMAT:
        *x = NUM2INT(point_x(get_from_array(points, k))) + draw_offset_x;
        *y = NUM2INT(point_y(get_from_array(points, k))) + draw_offset_y;
        break;
        
    case SIMPLE_FORMAT:
        xy_index = k * 2;
        *x = NUM2INT(get_from_array(points, xy_index)) + draw_offset_x;
        *y = NUM2INT(get_from_array(points, xy_index + 1)) + draw_offset_y;

        break;
    default:
        rb_raise(rb_eArgError, "pixel format must be either POINT_FORMAT or SIMPLE_FORMAT");
    }
}
        
void
polyline_do_action(VALUE points, texture_info * tex, VALUE hash_arg,
                   sync sync_mode, bool primary, action_struct * payload)
{

    int x1, y1, x2, y2;
    int format;
    int num_point_pairs;
    int k;
    int draw_offset_y, draw_offset_x;
    action_struct cur;
    VALUE offset_val;
    bool closed = false;
    
    draw_prologue(&cur, tex, XMAX_OOB, YMAX_OOB, XMIN_OOB, YMIN_OOB, &hash_arg, sync_mode, primary, &payload);

    /* calculate offset */
    offset_val = get_image_local(tex->image, DRAW_OFFSET);

    draw_offset_x = NUM2INT(get_from_array(offset_val, 0));
    draw_offset_y = NUM2INT(get_from_array(offset_val, 1));

    /* if the polyline is 'closed' make the last point the first */
    if(is_a_hash(hash_arg))
        if(RTEST(get_from_hash(hash_arg, "closed")) || RTEST(get_from_hash(hash_arg, "close"))) {
            
            /* so that our additional point is not persistent */
            points = rb_obj_dup(points);
            closed = true;
        }
    /* determine format of points */
    if(is_a_point(get_from_array(points, 0))) {
        format = POINT_FORMAT;

        /* if the polyline is closed to form a polygon then make the last point and first point identical */
        if(closed)
            rb_ary_push(points, get_from_array(points, 0));

        num_point_pairs = RARRAY_LEN(points);
    }
    else {
        format = SIMPLE_FORMAT;

        /* ensure there is an 'x' for every 'y'  */
        if(RARRAY_LEN(points) % 2)
            rb_raise(rb_eArgError, "polyline needs an even number of points. got %d\n",
                     (int)RARRAY_LEN(points));

        if(closed) {
            rb_ary_push(points, get_from_array(points, 0));
            rb_ary_push(points, get_from_array(points, 1));
        }

        num_point_pairs = RARRAY_LEN(points) / 2;
    }

    /* calculate first point */
    polyline_point(points, 0, &x1, &y1, format, draw_offset_x, draw_offset_y);
    
    /* calc the points and draw the polyline */
    for(k = 1; k < num_point_pairs; k++) {

        polyline_point(points, k, &x2, &y2, format, draw_offset_x, draw_offset_y);
    
        line_do_action(x1, y1, x2, y2, tex, hash_arg, no_sync, false, payload);

        /* update drawing rectangle */
        update_bounds(payload, x1, y1, x2, y2);
        
        x1 = x2; y1 = y2;
    }
    
    draw_epilogue(&cur, tex, primary);
}
/** end polyline **/

/* regular polygon algorithm */
void
ngon_do_action(int x, int y, int r, int num_sides, texture_info * tex, VALUE hash_arg,
               sync sync_mode, bool primary, action_struct * payload)
{
    action_struct cur;
    int x1, y1, x2, y2, x0, y0;
    int thickness;
    float angle = 0;

    draw_prologue(&cur, tex, x - r, y - r,
                  x + r, y + r, &hash_arg, sync_mode, primary, &payload);


    if(is_a_hash(hash_arg)) {
        if(RTEST(get_from_hash(hash_arg, "thickness"))) {
            thickness = NUM2INT(get_from_hash(hash_arg, "thickness"));

            /* TO DO: find a better way of doing this */
            cur.xmin = x - r - thickness / 2;
            cur.ymin = y - r - thickness / 2;
            cur.xmax = x + r + thickness / 2;
            cur.ymax = y + r + thickness / 2;
        }

        if(RTEST(get_from_hash(hash_arg, "start_angle"))) {
            angle = NUM2INT(get_from_hash(hash_arg, "start_angle")) / 360.0 * 2 * PI;
        }
    }
    
    /* calculate first point */
    x0 = x1 = x + r * cos(angle);
    y0 = y1 = y + r * sin(angle);

    for(int n = 0; n < num_sides; n++) {
        x2 = x + r * cos((2 * PI / num_sides) * n + angle);
        y2 = y + r * sin((2 * PI / num_sides) * n + angle);

        line_do_action(x1, y1, x2, y2, tex, hash_arg, no_sync, false, payload);

        x1 = x2; y1 = y2;
    }

    line_do_action(x0, y0, x1, y1, tex, hash_arg, no_sync, false, payload);

    draw_epilogue(&cur, tex, primary);
}
/** end of ngon */    

/** rectangle algorithm **/
void
rect_do_action(int x1, int y1, int x2, int y2, texture_info * tex, VALUE hash_arg,
               sync sync_mode, bool primary, action_struct * payload)
{
    action_struct cur;
    bool fill = false;
    int thickness = 1;

    draw_prologue(&cur, tex, x1, y1,
                  x2, y2, &hash_arg, sync_mode, primary, &payload);

    
    if(is_a_hash(hash_arg)) {

        /* make our private copy of the hash so we can mess with it */
        hash_arg = rb_obj_dup(hash_arg);

        if(RTEST(get_from_hash(hash_arg, "fill")) || RTEST(get_from_hash(hash_arg, "filled"))) {
            fill = true;

            /* since we're filling the rect, line thickness is irrelevant */
            delete_from_hash(hash_arg, "thickness");
        }
        else if(RTEST(get_from_hash(hash_arg, "thickness"))) {
            thickness = NUM2INT(get_from_hash(hash_arg, "thickness"));
            /* TO DO: find a better way of doing this */

            if(thickness > 1) {
                cur.xmin = x1 - thickness / 2;
                cur.ymin = y1 - thickness / 2;
                cur.xmax = x2 + thickness / 2 + 1;
                cur.ymax = y2 + thickness / 2 + 1;
            }
        }
    }
    if(!fill) {
        line_do_action(x1, y1, x2, y1, tex, hash_arg, no_sync, false, payload);
        line_do_action(x1, y1, x1, y2, tex, hash_arg, no_sync, false, payload);
        line_do_action(x1, y2, x2, y2, tex, hash_arg, no_sync, false, payload);
        line_do_action(x2, y1, x2, y2, tex, hash_arg, no_sync, false, payload);
    }
    else {
        if(y1 > y2) SWAP(y1, y2);

        for(int y = y1; y <= y2; y++)
            line_do_action(x1, y, x2, y, tex, hash_arg, no_sync, false, payload);
    }

    draw_epilogue(&cur, tex, primary);
}


/** midpoint circle algorithm **/
void
circle_do_action(int x1, int y1, int r, texture_info * tex, VALUE hash_arg,
                 sync sync_mode, bool primary, action_struct * payload)
{

    int x, y;
    float p;
    action_struct cur;
    bool fill = false;

    draw_prologue(&cur, tex, x1 - r, y1 - r, x1 + r, y1 + r, &hash_arg,
                  sync_mode, primary, &payload);


    if(is_a_hash(hash_arg)) {

        /* make our private copy of the hash so we can mess with it */
        hash_arg = rb_obj_dup(hash_arg);

        if(RTEST(get_from_hash(hash_arg, "fill")) || RTEST(get_from_hash(hash_arg, "filled"))) {
            fill = true;

            /* to prevent infinite recursion set line thickness to 1 :D
               NB: a filled circle uses lines and a thick line uses filled circles :D */
            delete_from_hash(hash_arg, "thickness");
        }
    }
    
    x = 0 ; y = r;
    p = 5 / 4 - r;
    if(!fill) {
        while (x <= y) {
            set_pixel_color_with_style(payload, tex, x1 + x, y1 + y);
            set_pixel_color_with_style(payload, tex, x1 + x, y1 - y);
            set_pixel_color_with_style(payload, tex, x1 - x, y1 + y);
            set_pixel_color_with_style(payload, tex, x1 - x, y1 - y);
            set_pixel_color_with_style(payload, tex, x1 + y, y1 + x);
            set_pixel_color_with_style(payload, tex, x1 + y, y1 - x);
            set_pixel_color_with_style(payload, tex, x1 - y, y1 + x);
            set_pixel_color_with_style(payload, tex, x1 - y, y1 - x);

            if (p < 0) {
                p += 2 * x + 3;
            }
            else {
                y--;
                p += 2 * (x - y) + 5;
            }
            x++;
        }
    }
    else {
        while (x <= y) {
            line_do_action(x1 - x, y1 + y, x1 + x, y1 + y, tex, hash_arg, no_sync, false, payload);
            line_do_action(x1 - x, y1 - y, x1 + x, y1 - y, tex, hash_arg, no_sync, false, payload);
            line_do_action(x1 - y, y1 + x, x1 + y, y1 + x, tex, hash_arg, no_sync, false, payload);
            line_do_action(x1 - y, y1 - x, x1 + y, y1 - x, tex, hash_arg, no_sync, false, payload);

            if (p < 0) {
                p += 2 * x + 3;
            }
            else {
                y--;
                p += 2 * (x - y) + 5;
            }
            x++;
        }
    }

    draw_epilogue(&cur, tex, primary);
}
/** end circle **/

/** set pixel  algorithm **/
void
pixel_do_action(int x1, int y1, texture_info * tex, VALUE hash_arg,
                sync sync_mode, bool primary, action_struct * payload)
{
    action_struct cur;

    draw_prologue(&cur, tex, x1, y1, x1, y1, &hash_arg, sync_mode, primary, &payload);

    set_pixel_color_with_style(payload, tex, x1, y1);

    draw_epilogue(&cur, tex, primary);
}
/** end set pixel **/

/*** non recursive FLOOD FILL, inspired by John R. Shaw ***/
typedef struct { int x1, x2, y, dy; } LINESEGMENT;

#define MAXDEPTH 10000

#define PUSH(XL, XR, Y, DY)                                             \
    if( sp < stack+MAXDEPTH && Y+(DY) >= nMinX && Y+(DY) <= nMaxY )     \
        { sp->x1 = XL; sp->x2 = XR; sp->y = Y; sp->dy = DY; ++sp; }

#define POP(XL, XR, Y, DY)                                              \
    { --sp; XL = sp->x1; XR = sp->x2; Y = sp->y+(DY = sp->dy); }

void
flood_fill_do_action(int x, int y, texture_info * tex, VALUE hash_arg,
                     sync sync_mode, bool primary, action_struct * payload)
{
    int left, x1, x2, dy;
    rgba old_color;
    LINESEGMENT stack[MAXDEPTH], *sp = stack;

    action_struct cur;

    int nMinX, nMinY, nMaxX, nMaxY;
    
    /* NOTE: 1024 is just a place-holder to indicate maximum possible width/height.
       Values will be constrained to realistic dimensions by constrain_boundaries() function */
    draw_prologue(&cur, tex, 0, 0, 1024, 1024, &hash_arg, sync_mode, primary, &payload);

    /* fill hates alpha_blend so let's turn it off */
    payload->pen.alpha_blend = false;

    nMinX = cur.xmin; nMinY = cur.ymin;
    nMaxX = cur.xmax; nMaxY = cur.ymax;

    old_color = get_pixel_color(tex, x, y);
    if( cmp_color(old_color, cur.color) )
        return;

    if( x < nMinX || x > nMaxX || y < nMinX || y > nMaxY )
        return;

    PUSH(x, x, y, 1);        /* needed in some cases */
    PUSH(x, x, y + 1, -1);    /* seed segment (popped 1st) */

    while( sp > stack ) {
        POP(x1, x2, y, dy);

        for( x = x1; x >= nMinX && cmp_color(get_pixel_color(tex, x, y), old_color); --x ) {
            set_pixel_color_with_style(payload, tex, x, y);
        }

        if( x >= x1 )
            goto SKIP;

        left = x + 1;
        if( left < x1 )
            PUSH(y, left, x1 - 1, -dy);    /* leak on left? */

        x = x1 + 1;

        do {
            for( ; x <= nMaxX && cmp_color(get_pixel_color(tex, x, y), old_color); ++x ){
                set_pixel_color_with_style(payload, tex, x, y);
            }

            PUSH(left, x - 1, y, dy);

            if( x > x2 + 1 )
                PUSH(x2 + 1, x - 1, y, -dy);    /* leak on right? */

        SKIP:        for( ++x; x <= x2 && !cmp_color(get_pixel_color(tex, x, y), old_color); ++x ) {;}

            left = x;
        } while( x <= x2 );
    }

    draw_epilogue(&cur, tex, primary);
}
/*** END FLOOD FILL ***/

/** glow fill algorithm, from the gosu forums **/ 
static void
glow_floodFill( int x, int y, rgba * seed_color, action_struct * cur, texture_info * tex, texture_info * tex2 )
{
    /* used to flood in both horizontal directions from the given point to form a line. */
    int fillL, fillR;     
    int i;

    /* initialize the flood directions */
    fillL = fillR = x;

    /* flood left until a new color is hit - or the edge of the image */
    do {
        /* for texture filling */
        if(tex2)
            cur->color = get_pixel_color(tex2, fillL % tex2->width, y % tex2->height);
        
        /* TWO VERSIONS of below */

        /* SLOW BUT MODERN VERSION */
        /*        set_pixel_color_with_style(cur, tex, fillL, y); */

        /* FAST but old version */
        set_pixel_color(&cur->color, tex, fillL, y);

        fillL--;
    } while( (fillL >= 0 ) && (cmp_color(get_pixel_color(tex, fillL, y), *seed_color)) );

    /* flood right until a new color is hit - or the edge of the image */
    do {
        //        for texture filling
        if(tex2)
            cur->color = get_pixel_color(tex2, fillR % tex2->width, y % tex2->height);
        
        /*        set_pixel_color_with_style(cur, tex, fillR, y); */
         
        set_pixel_color(&cur->color, tex, fillR, y);

        fillR++;
    } while( (fillR < cur->xmax - 1) && (cmp_color(get_pixel_color(tex, fillR, y), *seed_color)) );

    /* recurse to the line above and the line below at each point */
    for( i = fillL + 1; i < fillR; i++ ) {
        /* Flood above */
        if( ( y > 0 ) && ( cmp_color(get_pixel_color(tex, i, y - 1), *seed_color)  ) ) {
            
            glow_floodFill( i, y-1, seed_color, cur, tex, tex2 );
        }
        /* flood below */
        if( (y < cur->ymax - 1) && (cmp_color(get_pixel_color(tex, i, y + 1), *seed_color) )) {
            glow_floodFill( i, y+1, seed_color, cur, tex, tex2 );
        }
    }
}

void
glow_fill_do_action(int x, int y, texture_info * tex, VALUE hash_arg,
                    sync sync_mode, bool primary, action_struct * payload)
{
    action_struct cur;
    rgba seed_color;
    texture_info fill_image;
    texture_info * fill_image_ptr = NULL;
    
    if(!bound_by_rect(x, y, 0, 0, tex->width, tex->height)) return;

    draw_prologue(&cur, tex, 0, 0, 1024, 1024, &hash_arg, sync_mode, primary, &payload);

    /* fill hates alpha_blend so let's turn it off */
    payload->pen.alpha_blend = false;

    if(is_a_hash(hash_arg)) {
        VALUE try_image = get_from_hash(hash_arg, "texture");
        if(is_gosu_image(try_image)) {
            get_texture_info(try_image, &fill_image);
            fill_image_ptr = &fill_image;
        }
    }

    seed_color = get_pixel_color(tex, x, y);

    /* last argument is pointer to texture fill data (if it exists) or NULL (if it doesn't) */
    glow_floodFill( x, y, &seed_color, &cur, tex, fill_image_ptr );

    draw_epilogue(&cur, tex, primary);
}
/** end of glow fill **/

/** scanfill algorithm **/
/* the stack */
#define stackSize 16777218
int stack[stackSize];
int stackPointer;

static bool
pop(int * x, int * y, int h) 
{ 
    if(stackPointer > 0) 
        { 
            int p = stack[stackPointer]; 
            *x = p / h; 
            *y = p % h; 
            stackPointer--; 
            return true; 
        }     
    else 
        { 
            return false;
        }    
}    

static bool
push(int x, int y, int h) 
{ 
    if(stackPointer < stackSize - 1) 
        { 
            stackPointer++; 
            stack[stackPointer] = h * x + y; 
            return true;
        }     
    else 
        { 
            return false;
        }    
}     

static void
emptyStack() 
{ 
    int x, y; 
    while(pop(&x, &y, 0)); 
}

void
scan_fill_do_action(int x, int y, texture_info * tex, VALUE hash_arg,
                    sync sync_mode, bool primary, action_struct * payload)
{
    action_struct cur;
    rgba old_color;
    int y1;
    bool spanLeft, spanRight;

    if(!bound_by_rect(x, y, 0, 0, tex->width - 1, tex->height - 1)) return;

    /* NB: using XMAX_OOB etc since we dont know the drawing area yet; drawing area will be set by
       update_bounds() function in main loop */
    draw_prologue(&cur, tex, XMAX_OOB, YMAX_OOB, XMIN_OOB, YMIN_OOB, &hash_arg, sync_mode, primary, &payload);

    /* fill hates alpha_blend so let's turn it off */
    payload->pen.alpha_blend = false;
    
    old_color = get_pixel_color(tex, x, y);

    if(cmp_color(old_color, cur.color)) return;
    
    emptyStack();
    
    if(!push(x, y, tex->width - 1)) return;
    
    while(pop(&x, &y, tex->width - 1))
        {    
            y1 = y;
            while(y1 >= 0 && cmp_color(old_color, get_pixel_color(tex, x, y1))) y1--;
            y1++;
            spanLeft = spanRight = false;
            while(y1 < tex->height && cmp_color(old_color, get_pixel_color(tex, x, y1)) )
                {
                    set_pixel_color_with_style(payload, tex, x, y1);

                    /* update the drawing rectangle */
                    update_bounds(payload, x, y1, x, y1);

                    if(!spanLeft && x > 0 && cmp_color(old_color, get_pixel_color(tex, x - 1, y1))) 
                        {
                            if(!push(x - 1, y1, tex->width - 1)) return;
                            spanLeft = true;
                        }

                    else if(spanLeft && !cmp_color(old_color, get_pixel_color(tex, x - 1, y1)))
                        {
                            spanLeft = false;
                        }

                    if(!spanRight && x < tex->width - 1 && cmp_color(old_color,
                                                                     get_pixel_color(tex, x + 1, y1))) 
                        {
                            if(!push(x + 1, y1, tex->width - 1)) return;
                            spanRight = true;
                        }

                    else if(spanRight && x < tex->width - 1 && !cmp_color(old_color,get_pixel_color(tex, x + 1, y1)))
                        {
                            spanRight = false;
                        } 
                    y1++;
                }
        }
    draw_epilogue(&cur, tex, primary);
}
/** end of scanfill **/

/** bezier curve algorithm **/
static void
bezier_point(VALUE points, float u, float * x, float * y, int n, int format,
             int draw_offset_x, int draw_offset_y)
{
    int xy_index;
    double sumx = 0, sumy = 0;
 

    for(int k = 0; k < n; k++) {
        switch(format) {
        case POINT_FORMAT:
            
            sumx += NUM2DBL(point_x(get_from_array(points, k))) * bernstein(n - 1, k, u);
            sumy += NUM2DBL(point_y(get_from_array(points, k))) * bernstein(n - 1, k, u);
            break;
        case SIMPLE_FORMAT:
             
            xy_index = k * 2;
            sumx +=  NUM2DBL(get_from_array(points, xy_index)) * bernstein(n - 1, k, u);
            sumy +=  NUM2DBL(get_from_array(points, xy_index + 1)) * bernstein(n - 1, k, u);
            break;
        default:
            rb_raise(rb_eArgError, "pixel format must be either POINT_FORMAT or SIMPLE_FORMAT");
        }
    }

    *x = sumx + draw_offset_x;
    *y = sumy + draw_offset_y;
}

void
bezier_do_action(VALUE points, texture_info * tex, VALUE hash_arg, sync sync_mode,
                 bool primary, action_struct * payload)
{
    float u = 0.0;
    action_struct cur;
    float x1, y1, x2, y2;
    int first_x, first_y;
    int format;
    int num_point_pairs;
    bool closed = false;
    VALUE offset_val;
    int draw_offset_x, draw_offset_y;

    /* defaults to 200 (1 / 0.005) samples per curve */
    float step_size = 0.005;
    
    draw_prologue(&cur, tex, XMAX_OOB, YMAX_OOB, XMIN_OOB, YMIN_OOB, &hash_arg, sync_mode, primary, &payload);

    /* calculate offset */
    offset_val = get_image_local(tex->image, DRAW_OFFSET);

    draw_offset_x = NUM2INT(get_from_array(offset_val, 0));
    draw_offset_y = NUM2INT(get_from_array(offset_val, 1));

    if(is_a_hash(hash_arg)) {

        /* if the polyline is 'closed' make the last point the first */
        if(RTEST(get_from_hash(hash_arg, "closed")) || RTEST(get_from_hash(hash_arg, "close"))) {

            /* so that our additional point is not persistent */
            points = rb_obj_dup(points);
            closed = true;
        }
        
        /* number of points to sample */
        if(RTEST(get_from_hash(hash_arg, "sample_size"))) {
            VALUE c = get_from_hash(hash_arg, "sample_size");
            Check_Type(c, T_FIXNUM);
            step_size = 1.0 / (float)FIX2INT(c);
        }
    }

    if(is_a_point(get_from_array(points, 0))) {
        format = POINT_FORMAT;

        if(closed)
            rb_ary_push(points, get_from_array(points, 0));
        
        num_point_pairs = RARRAY_LEN(points);
    }
    else {
        format = SIMPLE_FORMAT;

        /* ensure points are given in pairs */
        if(RARRAY_LEN(points) % 2)
            rb_raise(rb_eArgError, "bezier needs an even number of points. got %d\n", (int)RARRAY_LEN(points));

        if(closed) {
            rb_ary_push(points, get_from_array(points, 0));
            rb_ary_push(points, get_from_array(points, 1));
        }
        
        num_point_pairs = RARRAY_LEN(points) / 2;
    }

    if(num_point_pairs > 17)
        rb_raise(rb_eArgError, "too many points for bezier curve. 17 points is current maximum. got %d\n",
                 num_point_pairs);

    /* get the first point */
    bezier_point(points, 0, &x1, &y1, num_point_pairs, format, draw_offset_x, draw_offset_y);

    /* save it so we can link up with last point properly if the curve is 'closed' */
    first_x = x1;
    first_y = y1;

    while(u <= 1) {
        bezier_point(points, u, &x2, &y2, num_point_pairs, format, draw_offset_x, draw_offset_y);
 
        line_do_action(x1, y1, x2, y2, tex, hash_arg, no_sync, false, payload);

        /* update drawing rectangle */
        update_bounds(payload, x1, y1, x2, y2);

        x1 = x2;
        y1 = y2;

        u += step_size;
    }

    /* sometimes beziers dont close properly, so we'll ensure it's closed */
    if(closed)
        line_do_action(x2, y2, first_x, first_y, tex, hash_arg, no_sync, false, payload);

    draw_epilogue(&cur, tex, primary);
}
/** end of bezier **/

/** each_pixel  **/
static void
set_color_array(VALUE ary, rgba * color)
{
    set_array_value(ary, 0, rb_float_new(color->red));
    set_array_value(ary, 1, rb_float_new(color->green));
    set_array_value(ary, 2, rb_float_new(color->blue));
    set_array_value(ary, 3, rb_float_new(color->alpha));
}
    
void
each_pixel_do_action(int x1, int y1, int x2, int y2, VALUE proc, texture_info * tex, VALUE hash_arg,
                     sync sync_mode, bool primary, action_struct * payload)
{
    action_struct cur;
    int arity;
    VALUE rb_pix = rb_ary_new2(4);
    
    draw_prologue(&cur, tex, x1, y1, x2, y2, &hash_arg, sync_mode, primary, &payload);

    arity = FIX2INT(rb_funcall(proc, rb_intern("arity"), 0));

    for(int y = y1; y < y2 + 1; y++)
        for(int x = x1; x < x2 + 1; x++) {
            rgba pix = get_pixel_color(tex, x, y);

            set_color_array(rb_pix, &pix);
            
            /* invoke the block */
            switch(arity) {
            case 1:
                rb_funcall(proc, rb_intern("call"), 1, rb_pix);
                break;
            case 3:
                rb_funcall(proc, rb_intern("call"), 3, rb_pix, INT2FIX(x), INT2FIX(y));
                break;
            default:
                rb_raise(rb_eArgError, "permissible arities for each are 1 & 3. Got %d\n",
                         arity);

            }

            pix = convert_rb_color_to_rgba(rb_pix);

            set_pixel_color(&pix, tex, x, y);
        }

    draw_epilogue(&cur, tex, primary);
}
/** end each_pixel iterator **/

/** splice algorithm **/
void
splice_do_action(int x0, int y0, int cx1, int cy1, int cx2, int cy2, texture_info * splice_tex,
                 texture_info * tex, VALUE hash_arg, sync sync_mode,
                 bool primary, action_struct * payload)
{
    action_struct cur;
    int xbound;
    int ybound;
    rgba chromakey;
    float * image_buf = NULL;
    bool inverse_chroma = false;
    bool same_image = false;
    bool has_chroma = false;

    constrain_boundaries(&cx1, &cy1, &cx2, &cy2, splice_tex->width, splice_tex->height);
    xbound = cx2 - cx1 + 1;
    ybound = cy2 - cy1 + 1;

    draw_prologue(&cur, tex, x0, y0,
                  x0 + xbound, y0 + ybound, &hash_arg, sync_mode, primary, &payload);


    if(has_optional_hash_arg(hash_arg, "chroma_key")) {
        VALUE c = get_from_hash(hash_arg, "chroma_key");
        chromakey = convert_rb_color_to_rgba(c);
        has_chroma = true;
    }
    else if(has_optional_hash_arg(hash_arg, "chroma_key_not")) {
        chromakey = convert_rb_color_to_rgba(get_from_hash(hash_arg, "chroma_key_not"));
        inverse_chroma = true;
        has_chroma = true;
    }

    if(splice_tex->image == tex->image)
        same_image = true;

    /* NB: we do not use this in the general case since it's almost 1.5 times as slow.
       It is necessary for splicing from/to the same region of pixels though.
    */
    if(same_image) 
        image_buf = get_image_chunk(splice_tex, cx1, cy1, cx2, cy2);
    
    for(int y = 0; y < ybound; y++)
        for(int x = 0; x < xbound; x++) {
            
            if(!same_image)
                payload->color = get_pixel_color(splice_tex, cx1 + x, cy1 + y);
            else 
                payload->color = get_pixel_color_from_chunk(image_buf, xbound, ybound, x, y);
                
            if(has_chroma) {
                bool chroma_match = cmp_color(payload->color, chromakey);
                
                /* look at released 0.2.0 code to see how USED to do this.
                   this is now a simplified boolean expression (XOR) */
                if(chroma_match == inverse_chroma)
                    set_pixel_color_with_style(payload, tex, x0 + x, y0 + y);
            }
            else
                set_pixel_color_with_style(payload, tex, x0 + x, y0 + y);
        }

    if(same_image)
        free(image_buf);

    draw_epilogue(&cur, tex, primary);
}
/** end splice **/



