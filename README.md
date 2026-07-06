# FastInAHurry
Header-only C++ library with a performance-first approach. Not every component is guaranteed to be standards-compliant.

# Readiness
Some of the library is production-ready, and some is not:


| Directory / File                              | Completion Estimate | Production-Ready?              | Basis                                                                                                         |
| --------------------------------------------- | ------------------- | ------------------------------ | ------------------------------------------------------------------------------------------------------------- |
| **[SpinMutex][1]**                            | 50%                 | **No**                      | Do not use. |
| **[ThreadPool][2]**                            | 85%                 | **Alpha**     |                Technically ready, but can be made significantly more performant.    |                                   
| **[UniquePtr][3]**                                    | 90%                  | **Yes**                      | Ready
| **[SPSCQueue][4]**                                 | 80%                  | **Alpha**                      | Still needs a few optimizations.                       |
| **[MPSCQueue][5]**                                 | 80%                  | **Alpha**                      | Still needs a few optimizations.                       |
| **[Vector][6]**                                 | 90%                  | **Yes**                      | Ready |

[1]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/thread/SpinMutex.hpp ""
[2]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/thread/ThreadPool.hpp ""
[3]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/handle/UniquePtr.hpp ""
[4]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/structs/SPSCQueue.hpp ""
[5]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/structs/MPSCQueue.hpp ""
[6]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/structs/Vector.hpp ""

