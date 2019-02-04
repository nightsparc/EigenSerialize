# EigenSerialize
Test repo for Eigen matrix serialization with boost

Somewhere in the code an Eigen type-trait gets instantiated which tries to access an incomplete
type of boost::serialization. Totally strang. Works with C++11/14...??!

    usr/local/include/Eigen/src/Core/util/ForwardDeclarations.h:32:
    error: incomplete type ‘Eigen::internal::traits<boost::serialization::U>’ used in nested name specifier
    enum { has_direct_access = (traits<Derived>::Flags & DirectAccessBit) ? 1 : 0,

The error occures whenever `boost/serialization/shared_ptr.hpp` is included - regardless if a shared_ptr is serialized or not. See line 15 in the main.cpp:

```C++
#include <boost/serialization/shared_ptr.hpp>
``` 

Seems to be compiler-related:
- GCC7/8 failes with the mesage above (regardless if `--std=c++17` or `--std=c++20`)
- GCC6 works with `--std=c++17`
- Clang6 works with `--std=c++17`
