Mira
- compiler/tools/disagg/lib/Dialect/FunctionUtils.cpp
- runtime/libcommon/lib/cache.c
- 


Discussion:
- For the `store` operation, do we need to fetch the corresponding object if it is not in the cache?
- chunk per thread?
- cache size
- Run & recompile?
    - cache size
    - offloading decision


Version 1.0:
- Only caching (No offloading)
- Only consider the `struct` type