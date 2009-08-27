/* object2module.c */
/* (C) John Mair 2009
 * This program is distributed under the terms of the MIT License
 *                                                                */

#include <ruby.h>
#include "compat.h"

#ifdef RUBY_19
# include <ruby/st.h>
#else
# include <st.h>
#endif

/* class creation. from class.c in 1.9.1 */
#ifdef RUBY_19
static VALUE
class_alloc(VALUE flags, VALUE klass)
{
    rb_classext_t *ext = ALLOC(rb_classext_t);
    NEWOBJ(obj, struct RClass);
    OBJSETUP(obj, klass, flags);
    obj->ptr = ext;
    RCLASS_IV_TBL(obj) = 0;
    RCLASS_M_TBL(obj) = 0;
    RCLASS_SUPER(obj) = 0;
    RCLASS_IV_INDEX_TBL(obj) = 0;
    return (VALUE)obj;
}
#endif

/* a modified version of include_class_new from class.c */
static VALUE
j_class_new(VALUE module, VALUE sup)
{

#ifdef RUBY_19
    VALUE klass = class_alloc(T_ICLASS, rb_cClass);
#else
    NEWOBJ(klass, struct RClass);
    OBJSETUP(klass, rb_cClass, T_ICLASS);
#endif

    if (BUILTIN_TYPE(module) == T_ICLASS) {
        module = KLASS_OF(module);
    }

    if (!RCLASS_IV_TBL(module)) {

        RCLASS_IV_TBL(module) = (struct st_table *)st_init_numtable();
    }

    /* assign iv_tbl, m_tbl and super */
    RCLASS_IV_TBL(klass) = RCLASS_IV_TBL(module);
    RCLASS_SUPER(klass) = sup;
    if(TYPE(module) != T_OBJECT) {

        RCLASS_M_TBL(klass) = RCLASS_M_TBL(module);
    }
    else {
        RCLASS_M_TBL(klass) = RCLASS_M_TBL(CLASS_OF(module));
    }

    /* */

    if (TYPE(module) == T_ICLASS) {
        KLASS_OF(klass) = KLASS_OF(module);
    }
    else {
        KLASS_OF(klass) = module;
    }

    if(TYPE(module) != T_OBJECT) {
        OBJ_INFECT(klass, module);
        OBJ_INFECT(klass, sup);
    }
    return (VALUE)klass;
}

VALUE
rb_to_module(VALUE self)
{
    VALUE rclass, chain_start, jcur, klass;

    switch(BUILTIN_TYPE(self)) {
    case T_MODULE:
        return self;
    case T_CLASS:
        klass = self;
        break;
    case T_OBJECT:
    default:
        klass = rb_singleton_class(self);            
    }
            
    chain_start = j_class_new(klass, rb_cObject);

    KLASS_OF(chain_start) = rb_cModule;
    RBASIC(chain_start)->flags = T_MODULE;

    jcur = chain_start;
    for(rclass = RCLASS_SUPER(klass); rclass != rb_cObject;
        rclass = RCLASS_SUPER(rclass)) {
                
        RCLASS_SUPER(jcur) = j_class_new(rclass, rb_cObject);
        jcur = RCLASS_SUPER(jcur);
    }

    RCLASS_SUPER(jcur) = (VALUE)NULL;

    return chain_start;
}

VALUE
rb_reset_tbls(VALUE self)
{
    RCLASS_IV_TBL(self) = (struct st_table *) 0;
    RCLASS_M_TBL(self) = (struct st_table *) st_init_numtable();
    return Qnil;
}

/* cannot simply forward to gen_include as need to invoke 'extended' hook */
VALUE
rb_gen_extend(int argc, VALUE * argv, VALUE self)
{
    int i;

    if (argc == 0) rb_raise(rb_eArgError, "wrong number of arguments (0 for 1)");
    
    rb_singleton_class(self);

    for(i = 0; i < argc; i++) {
        VALUE mod = rb_to_module(argv[i]); 
        rb_funcall(mod, rb_intern("extend_object"), 1, self);
        rb_funcall(mod, rb_intern("extended"), 1, self);
        
        /* only redirect if argv[i] is not a module */
        if(argv[i] != mod) rb_reset_tbls(mod);
    }

    return self;
}

VALUE
rb_gen_include(int argc, VALUE * argv, VALUE self)
{
    int i;

    if (argc == 0) rb_raise(rb_eArgError, "wrong number of arguments (0 for 1)");

    for(i = 0; i < argc; i++) {
        VALUE mod = rb_to_module(argv[i]); 
        rb_funcall(mod, rb_intern("append_features"), 1, self);
        rb_funcall(mod, rb_intern("included"), 1, self);
        
        if(argv[i] != mod) rb_reset_tbls(mod);       
    }

    return self;
}
 

void Init_object2module()
{

    rb_define_method(rb_cObject, "to_module", rb_to_module , 0);
    rb_define_method(rb_cObject, "gen_extend", rb_gen_extend, -1);
    rb_define_method(rb_cModule, "gen_include", rb_gen_include, -1);
    rb_define_method(rb_cModule, "reset_tbls", rb_reset_tbls, 0);
}

