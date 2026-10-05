# Host-only SDK mocks

`include/` supplies the minimal ESP-IDF/FreeRTOS declarations used by the native
regression tests. Stub implementations and fault injection live in each test
translation unit, close to its assertions.

Only [../run.sh](../run.sh) adds this directory to compiler search paths.
Production and hardware-test builds must use the real SDK headers instead.
These mocks validate explicit software contracts, not electrical behavior or
the SDK's full scheduler/interrupt semantics.