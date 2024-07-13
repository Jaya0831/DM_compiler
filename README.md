# DM_compiler

## Prerequisite

- C99 and C++17 compiler
- Native build system (e.g. Make, Ninja)
- `librdmacm` and `libibverbs`

## Build

1. Configure CMake
   ```sh
   cmake -B build
   ```
   or with your choice of generator (native build system) e.g.
   ```sh
   cmake -B build -G Ninja
   ```
2. Build
   ```sh
   cmake --build build
   ```
