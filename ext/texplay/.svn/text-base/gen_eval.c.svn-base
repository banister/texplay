/* gen_eval.c */
/* (C) John Mair 2009
 * This program is distributed under the terms of the MIT License
 *                                                                */

#include <ruby.h>
#include "object2module.h"
#include "compat.h"

VALUE
retrieve_hidden_self(VALUE duped_context)
{
    VALUE thread_id, unique_name, hidden_self;
    
    /* retrieve hidden self (if it exists) */
    thread_id = rb_funcall(rb_obj_id(rb_thread_current()), rb_intern("to_s"), 0);
    unique_name = rb_str_plus(rb_str_new2("__hidden_self__"), thread_id);
    
    hidden_self = rb_ivar_get(duped_context, rb_to_id(unique_name));

    return hidden_self;
}

void
set_hidden_self(VALUE duped_context, VALUE hidden_self)
{
    VALUE thread_id, unique_name;
    
    /* generate a unique (thread safe) name for the hidden self  */
    thread_id = rb_funcall(rb_obj_id(rb_thread_current()), rb_intern("to_s"), 0);
    unique_name = rb_str_plus(rb_str_new2("__hidden_self__"), thread_id);

    /* store self in hidden var in duped context */
    rb_ivar_set(duped_context, rb_to_id(unique_name), hidden_self);
}

VALUE
rb_capture(VALUE self) {
    VALUE hidden_self;
    
    rb_need_block();
    
    hidden_self = retrieve_hidden_self(self);

    /* 2 cases: (1) if rb_gen_eval is active then instance_eval wrt hidden_self
                (2) otherwise simply yield to the block
    */
    if(!NIL_P(hidden_self))
        rb_obj_instance_eval(0, 0, hidden_self);
    else
        rb_yield(Qnil);

    return hidden_self;
}

/** ruby 1.9 funcs **/
#ifdef RUBY_19
void
redirect_iv_for_object(VALUE obj, VALUE dest)
{
    if(TYPE(obj) != T_OBJECT)
        rb_raise(rb_eArgError, "must provide a T_OBJECT");

    if (!(RBASIC(dest)->flags & ROBJECT_EMBED) && ROBJECT_IVPTR(dest)) {
        rb_raise(rb_eArgError, "im sorry gen_eval does not yet work with this type of ROBJECT");
    }
    if (RBASIC(obj)->flags & ROBJECT_EMBED) {
        rb_raise(rb_eArgError, "im sorry gen_eval does not yet work with ROBJECT_EMBED types");
    }
    else {
        ROBJECT(dest)->as.heap.ivptr = ROBJECT(obj)->as.heap.ivptr;
        ROBJECT(dest)->as.heap.numiv = ROBJECT(obj)->as.heap.numiv;
        ROBJECT(dest)->as.heap.iv_index_tbl = ROBJECT(obj)->as.heap.iv_index_tbl;
        RBASIC(dest)->flags &= ~ROBJECT_EMBED;
    }
}

void
release_iv_for_object(VALUE obj)
{
    if(TYPE(obj) != T_OBJECT)
        rb_raise(rb_eArgError, "must provide a T_OBJECT");
    
    ROBJECT(obj)->as.heap.ivptr = (void *) 0;
    ROBJECT(obj)->as.heap.numiv = 0;
    ROBJECT(obj)->as.heap.iv_index_tbl = (void *) 0;
    RBASIC(obj)->flags &= ~ROBJECT_EMBED;
}
#endif
/** end of ruby 1.9 funcs **/

VALUE
rb_gen_eval(int argc, VALUE * argv, VALUE self) {
    VALUE duped_context;
    VALUE is_a_module;
    VALUE context;
    VALUE result;
    
    rb_need_block();
        
    context = rb_funcall(rb_block_proc(), rb_intern("__context__"), 0);

    /* using Class instead of Object (where possible) because Object's iv_tbl setup in 1.9 is weird */
#ifdef RUBY_19
    if(TYPE(context) == T_OBJECT)
        duped_context = rb_funcall(rb_cObject, rb_intern("new"), 0);
    else 
        duped_context = rb_funcall(rb_cClass, rb_intern("new"), 0);

#else

    duped_context = rb_funcall(rb_cClass, rb_intern("new"), 0);

#endif    
    
    
    /* the duped_context shares the context's iv_tbl.
       2 cases: (1) external iv_tbl, (2) local iv_tbl
       
       NOTE: we do not need to save original iv_tbl before replacing it, a brand new Class
       instance does not yet have an iv_tbl (the pointer is set to 0) 
    */
    if(FL_TEST(context, FL_EXIVAR)) 
        RCLASS_IV_TBL(duped_context) = (struct st_table *) rb_generic_ivar_table(context);
    else {
#ifdef RUBY_19
        if(TYPE(context) == T_OBJECT)
            redirect_iv_for_object(context, duped_context);
        else {
            RCLASS_IV_TBL(duped_context) = (struct st_table *) RCLASS_IV_TBL(context);
        }
#else
        RCLASS_IV_TBL(duped_context) = (struct st_table *) RCLASS_IV_TBL(context);
#endif        

        
    }
        
    /* ensure singleton exists */
    rb_singleton_class(context);
    
    /* set up the class hierarchy for our dup_context */
    KLASS_OF(duped_context) = rb_singleton_class_clone(context);

    /* if no args then default to mixing in 'self' */
    if(argc == 0) {
        argc = 1;
        argv = &self;
    }

    /* mix the objects (or self) into the duped context */
    rb_gen_extend(argc, argv, duped_context);

    /* store self in hidden var in duped context */
    set_hidden_self(duped_context, self);

    is_a_module = rb_funcall(duped_context, rb_intern("is_a?"), 1, rb_cModule);

    /* eval block wrt duped_context */
    if(is_a_module == Qtrue)
        result = rb_mod_module_eval(0, 0, duped_context);
    else
        result = rb_obj_instance_eval(0, 0, duped_context);

    /* clean up goes below */

    /* release context's iv_tbl from duped_context. */
#ifdef RUBY_19
    if(TYPE(duped_context) == T_OBJECT)
        release_iv_for_object(duped_context);
    else {
        RCLASS_IV_TBL(duped_context) = (struct st_table *) 0;
    }
#else
    RCLASS_IV_TBL(duped_context) = (struct st_table *) 0;
#endif

    /* delete hidden self */
    set_hidden_self(duped_context, Qnil);
    
    return result;
}

void
Init_gen_eval() {

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
}

    
    

    
