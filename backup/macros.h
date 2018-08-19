#ifndef MACROS_H
#define MACROS_H

#ifndef USING_NAMESPACE_FASTSTL
#define USING_NAMESPACE_FASTSTL using namespace faststl
#endif

#ifndef USING_NAMESPACE_FASTSTL_BEGIN
#define USING_NAMESPACE_FASTSTL_BEGIN namespace faststl{
#endif

#ifndef USING_NAMESPACE_FASTSTL_END
#define USING_NAMESPACE_FASTSTL_END }
#endif


#include <iostream>
#ifndef __THROW_BAD_ALLOC
#define __THROW_BAD_ALLOC std::cerr << "out of memory" << std::endl;exit(1)
#endif

#endif // MACROS_H
