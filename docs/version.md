### Version 0.1
- Single Threading
- Library for caching (50%) (Zeling & Yauming, July 31)
    - RDMA Backbone (100%)
    - Global Memory Allocation (100%)
        - No free needed yet
    - Caching (0%) (+ Srijan)
- Library used manually
- Compiler
    - Static Analysis (Jiayi & Xinpeng & Poh, Aug 31)
        - Identify all struct that has address dependecy.
            - Open Source Tool to identify data dependecy e.g., MLIR
            - Filter data dependency to get address dependecy. (Load/Store -> Load) based on some simple heuristic

### Version Alpha
- Connect the compiler and libary
    - Code Injection
        - Create a customized dialect (dm)
        - Convert original IR to dm.IR
        - Lowering dm.IR to the built-in dialects (call the library)

### Testing 
(FYP?)
- Single Thread B+ Tree / SkipList / Linked list / hash table / DAG

### Version 1.0
- Multi Threading
    - Reference Count / Lock
    - Global Allocation
        - Private vs Shared

### Version 2.0
- Offloading
- Prefetching
