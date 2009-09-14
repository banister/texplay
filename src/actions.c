#include "texplay.h"
#include "utils.h"
#include "actions.h"
#include <assert.h>
#include <math.h>
#ifdef __APPLE__
# include <glut.h>
#else
# include <GL/glut.h>
#endif


/* small helper functions */
static void process_common_hash_args(action_struct * cur, VALUE * hash_arg, sync sync_mode, bool primary);
static void update_bounds(action_struct * cur, int xmin, int ymin, int xmax, int ymax);
static void update_lazy_bounds(action_struct * cur, texture_info * tex);
static void set_local_bounds(action_struct * cur, int xmin, int ymin, int xmax, int ymax, texture_info * tex);

static void draw_prologue(action_struct * cur, texture_info * tex,
                          int xmin, int ymin, int xmax, int ymax, VALUE * hash_arg, sync sync_mode,
                          bool primary, action_struct ** payload_ptr);
static void draw_epilogue(action_struct * cur, texture_info * tex, bool primary);

static void prepare_fill_texture(action_struct * cur);
static void prepare_color_control(action_struct * cur);
static void set_pixel_color_with_style(action_struct * payload, texture_info * tex,
                                       int x, int y);
/* end helpers */


/** line_do_action, bresenham's algorithm **/
void
line_do_action(int x1, int y1, int x2, int y2, texture_info * tex, VALUE hash_arg,
               sync sync_mode, bool primary, action_struct * payload)
{
    int x, y, W, H, F;
    int xinc, yinc;
    action_struct cur;
    int thickness = 1;

    draw_prologue(&cur, tex, x1, y1,
                  x2, y2, &hash_arg, sync_mode, primary, &payload);

    if(has_optional_hash_arg(hash_arg, "thickness")) 
        thickness = NUM2INT(get_from_hash(hash_arg, "thickness"));

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
            if(F < 0)
                F += 2 * H;
            else {
                F += 2 * (H - W);
                y += yinc;
            }
            x += xinc;

            if(thickness <= 1) {
                set_pixel_color_with_style(payload, tex, x, y);
            }
            else {
                set_hash_value(hash_arg, "fill", Qtrue);
                circle_do_action(x, y, thickness / 2, tex, hash_arg, no_sync, false, payload);
            }
        }
    }
    else {
        F = 2 * W - H;
        while(y != y2 ) {
            if(F < 0)
                F += 2 * W;
            else {
                F += 2 * (W - H);
                x += xinc;
            }
            y += yinc;

            if(thickness <= 1) {
                set_pixel_color_with_style(payload, tex, x, y);
            }
            else {
                set_hash_value(hash_arg, "fill", Qtrue);
                circle_do_action(x, y, thickness / 2, tex, hash_arg, no_sync, false, payload);
            }
            
        }
    }
			
    draw_epilogue(&cur, tex, primary);
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
    action_struct cur;
    int x1, y1, x2, y2;
    int format;
    int num_point_pairs;
    int k;
    VALUE offset_val;
    int draw_offset_x;
    int draw_offset_y;
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
    int n;
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

    for(n = 0; n < num_sides; n++) {
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
        int y;
        if(y1 > y2) SWAP(y1, y2);

        for(y = y1; y < y2; y++)
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

                    if(!spanLeft && x > 1 && cmp_color(old_color, get_pixel_color(tex, x - 1, y1))) 
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
    int k;
    int xy_index;
    double sumx = 0, sumy = 0;
 

    for(k = 0; k < n; k++) {
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

    if(num_point_pairs > 50)
        rb_raise(rb_eArgError, "too many points for bezier curve. 13 points is current maximum. got %d\n",
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
    int x, y, arity;
    VALUE rb_pix = rb_ary_new2(4);
    
    draw_prologue(&cur, tex, x1, y1, x2, y2, &hash_arg, sync_mode, primary, &payload);

    arity = FIX2INT(rb_funcall(proc, rb_intern("arity"), 0));

    for(y = y1; y < y2 + 1; y++)
        for(x = x1; x < x2 + 1; x++) {
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
    int x, y;
    float * image_buf = NULL;
    bool inverse_chroma = false;
    bool has_chroma = false;
    bool same_image = false;

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
    
    for(y = 0; y < ybound; y++)
        for(x = 0; x < xbound; x++) {
            
            if(!same_image)
                payload->color = get_pixel_color(splice_tex, cx1 + x, cy1 + y);
            else 
                payload->color = get_pixel_color_from_chunk(image_buf, xbound, ybound, x, y);
                
            if(has_chroma) {
                bool chroma_match = cmp_color(payload->color, chromakey);
                
                /* look at released 0.2.0 code to see how USED to do this.
                   this is now a simplified boolean expression */
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

    /* process the color_control block or transform (if there is one) */
    prepare_color_control(cur);

    /* process the filling texture (if there is one) */
    prepare_fill_texture(cur);

    /* does the user want to blend alpha values ? */
    if(get_from_hash(*hash_arg, "alpha_blend") == Qtrue)
        cur->pen.alpha_blend = true;

}

static void
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

static void
update_bounds(action_struct * cur, int xmin, int ymin, int xmax, int ymax)
{
    if(xmin > xmax) SWAP(xmin, xmax);
    if(ymin > ymax) SWAP(ymin, ymax);
    
    cur->xmin = MIN(cur->xmin, xmin);
    cur->ymin = MIN(cur->ymin, ymin);
    cur->xmax = MAX(cur->xmax, xmax);
    cur->ymax = MAX(cur->ymax, ymax);
}

static void
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

static void
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

    finished_pixel.alpha = blended_pixel.alpha;
    

    return finished_pixel;
}

static void
set_pixel_color_with_style(action_struct * payload, texture_info * tex, int x, int y)
{

    rgba blended_pixel;

    blended_pixel = payload->color;

    /* for color_control transform */
    if(payload->pen.has_color_control_transform)
        blended_pixel = apply_color_control_transform(payload, tex, x, y);
    
    /*    for texture fill  */
    if(payload->pen.has_source_texture)
        blended_pixel = get_pixel_color(&payload->pen.source_tex,
                                         x % payload->pen.source_tex.width,
                                         y % payload->pen.source_tex.height);

    /* for color_control block */
    if(payload->pen.has_color_control_proc)
        blended_pixel = exec_color_control_proc(payload, tex, x,  y, blended_pixel);
    

    /*  TO DO: do bitwise pixel combinations here */
    
    /*  TO DO: refactor into its own helper function
        & rewrite using sse2 */
    if(payload->pen.alpha_blend)
        blended_pixel = apply_alpha_blend(payload, tex, x,  y, blended_pixel);
    

    set_pixel_color(&blended_pixel, tex, x, y);
}

