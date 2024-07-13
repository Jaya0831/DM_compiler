add_library(dm_compiler_rt_compute
    src/alloc.c
    src/context.c
    src/rdma.c
    src/type.c
)
target_include_directories(dm_compiler_rt_compute
    PUBLIC
        include
        ${IBVERBS_INCLUDE_DIRS}
    PRIVATE
        src
)
target_link_libraries(dm_compiler_rt_compute RDMA::RDMAcm ${IBVERBS_LIBRARIES})

add_executable(dm_compiler_rt_compute_tests
    tests/main.c
)
target_include_directories(dm_compiler_rt_compute_tests
    PRIVATE
        tests
)
target_link_libraries(dm_compiler_rt_compute_tests dm_compiler_rt_compute)
