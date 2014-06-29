/* contains basic macros to facilitate ruby 1.8 and ruby 1.9 compatibility */

#ifndef GUARD_COMPAT_H
#define GUARD_COMPAT_H

#include <ruby.h>

/* macros for backwards compatibility with 1.8 */
#ifndef RCLASS_M_TBL
# define RCLASS_M_TBL(c) (RCLASS(c)->m_tbl)
#endif

#ifndef RCLASS_SUPER
# define RCLASS_SUPER(c) (RCLASS(c)->super)
#endif

#ifndef RCLASS_IV_TBL
# define RCLASS_IV_TBL(c) (RCLASS(c)->iv_tbl)
#endif

#endif
