/* gen_eval.h */

#ifndef GUARD_GEN_EVAL_H
#define GUARD_GEN_EVAL_H

#include <ruby.h>

VALUE rb_gen_eval(int argc, VALUE * argv, VALUE self);
VALUE rb_capture(VALUE self);
VALUE retrieve_hidden_self(VALUE duped_context);
void set_hidden_self(VALUE duped_context, VALUE hidden_self);

/* change self to hidden self if __hidden_self__ defined */
#define ADJUST_SELF(X)                                                    \
  do {                                                                    \
      if(!NIL_P(retrieve_hidden_self((X))))                               \
          (X) = retrieve_hidden_self((X));                                \
  } while(0)
        
#endif
