# FastInAHurry
Header-only C++ library with useful data structures and utilities for low-latency applications.

# Readiness
Some of the library is production-ready; some is not. Here is a table with the latest completion estimates:


| Directory / File                              | Completion Estimate | Production-Ready?              | Basis                                                                                                         |
| --------------------------------------------- | ------------------- | ------------------------------ | ------------------------------------------------------------------------------------------------------------- |
| **[SpinMutex][1]**                            | 85%                 | **Beta**                      | Production worthy, but barely. Still needs ISA-specific handling. |
| **[ThreadPool][2]**                            | 85%                 | **Alpha**     |                Technically ready, but can be made significantly more performant.    |                                   
| **[UniquePtr][3]**                                    | 95%                  | **Yes**                      | Ready to go.                                                             |
| **[SPSCQueue][4]**                                 | 60%                  | **Alpha**                      | Only use this queue to play around. Still needs a few optimizations.                       |
| **[memory/][5]**                                  | 85%                 | **Beta**                      | Some of these might be faster than glibc, some might be slower. There is a lot of potential for speedups through vectorization and other optimizations. Use at your own risk while I add different code paths for different sizes.                                             |

[1]: https://github.com/xbazzi/fastinahurry/tree/master/include/fiah/thread/SpinMutex.hpp "fastinahurry/include/fiah at master · xbazzi/fastinahurry · GitHub"
[2]: https://github.com/xbazzi/fastinahurry/tree/master/include/fiah/thread/ThreadPool.hpp "fastinahurry/examples at master · xbazzi/fastinahurry · GitHub"
[3]: https://github.com/xbazzi/fastinahurry/tree/master/include/fiah/handle/UniquePtr.hpp "fastinahurry/tests at master · xbazzi/fastinahurry · GitHub"
[4]: https://github.com/xbazzi/fastinahurry/tree/master/include/fiah/structs/SPSCQueue.hh "GitHub - xbazzi/fastinahurry: Low-latency library"
[5]: https://github.com/xbazzi/fastinahurry/tree/master/include/fiah/memory/ "GitHub - xbazzi/fastinahurry: Low-latency library"

