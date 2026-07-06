# FastInAHurry
Header-only C++ library with a performance-first approach. Not every component is guaranteed to be standards-compliant.

# Readiness
Some of the library is production-ready, and some is not:

### Handles

| Header | Completion | Production-Ready? | Notes |
| --- | --- | --- | --- |
| **[UniquePtr][UniquePtr]** | 90% | **Yes** | Ready |
| **[SharedPtr][SharedPtr]** | 60% | **Alpha** | Functional but untested at scale |

### Memory

| Header | Completion | Production-Ready? | Notes |
| --- | --- | --- | --- |
| **[BumpAllocator][BumpAllocator]** | 80% | **Alpha** | Depends on BumpArena |
| **[BumpArena][BumpArena]** | 75% | **Alpha** | Core arena backing BumpAllocator |

### Data Structures

| Header | Completion | Production-Ready? | Notes |
| --- | --- | --- | --- |
| **[Vector][Vector]** | 90% | **Yes** | Ready |
| **[SPSCQueue][SPSCQueue]** | 80% | **Alpha** | Still needs a few optimizations |
| **[MPSCQueue][MPSCQueue]** | 80% | **Alpha** | Still needs a few optimizations |
| **[ThreadSafeQueue][ThreadSafeQueue]** | 70% | **Alpha** | Mutex-backed queue |
| **[Orderbook][Orderbook]** | 60% | **Alpha** | Domain-specific; API may change |

### Threading

| Header | Completion | Production-Ready? | Notes |
| --- | --- | --- | --- |
| **[ThreadPool][ThreadPool]** | 85% | **Alpha** | Technically ready, but can be made significantly more performant |
| **[SpinMutex][SpinMutex]** | 50% | **No** | Do not use |

### Math

| Header | Completion | Production-Ready? | Notes |
| --- | --- | --- | --- |
| **[AutoDiff][AutoDiff]** | 30% | **No** | Early WIP |
| **[FiniteDiff][FiniteDiff]** | 40% | **No** | Early WIP |
| **[Matrix][Matrix]** | 40% | **No** | Early WIP |
| **[NewtonRaphson][NewtonRaphson]** | 40% | **No** | Early WIP |

### I/O

| Header | Completion | Production-Ready? | Notes |
| --- | --- | --- | --- |
| **[TcpClient][TcpClient]** | 80% | **Alpha** | TCP client |
| **[TcpServer][TcpServer]** | 80% | **Alpha** | TCP server |
| **[Udp][Udp]** | 80% | **Alpha** | UDP client and server |
| **[Config][Config]** | 60% | **Alpha** | Config helper |

### Utils

| Header | Completion | Production-Ready? | Notes |
| --- | --- | --- | --- |
| **[SPSCLogger][SPSCLogger]** | 80% | **Alpha** | Lock-free async logger built on SPSCQueue |
| **[Logger][Logger]** | 70% | **Alpha** | Thread-safe logger |
| **[Timer][Timer]** | 80% | **Alpha** | Wall-clock timer |
| **[TSCTimer][TSCTimer]** | 70% | **Alpha** | RDTSC-based timer |
| **[TimeStamp][TimeStamp]** | 85% | **Alpha** | Uses system_clock::now |
| **[TomlParser][TomlParser]** | 40% | **Alpha** | Barebones, do not use. |
| **[Types][Types]** | 95% | **Yes** | Typedef aliases |
| **[XorBitant][XorBitant]** | 85% | **Alpha** | XOR-based pseudo-random number generator |

[UniquePtr]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/handle/UniquePtr.hh
[SharedPtr]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/handle/SharedPtr.hh
[Error]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/error/Error.hh
[BumpAllocator]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/memory/BumpAllocator.hh
[BumpArena]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/memory/BumpArena.hh
[Vector]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/structs/Vector.hh
[SPSCQueue]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/structs/SPSCQueue.hh
[MPSCQueue]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/structs/MPSCQueue.hh
[ThreadSafeQueue]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/structs/ThreadSafeQueue.hh
[Orderbook]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/structs/Orderbook.hh
[ThreadPool]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/thread/ThreadPool.hpp
[SpinMutex]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/thread/SpinMutex.hpp
[AutoDiff]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/math/AutoDiff.hpp
[FiniteDiff]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/math/FiniteDiff.hpp
[Matrix]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/math/Matrix.hpp
[NewtonRaphson]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/math/NewtonRaphson.hpp
[Tcp]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/io/Tcp.hh
[TcpClient]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/io/TcpClient.hh
[TcpServer]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/io/TcpServer.hh
[Udp]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/io/Udp.hh
[Socket]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/io/Socket.hh
[Config]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/io/Config.hh
[Cassandra]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/io/Cassandra.hh
[SPSCLogger]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/utils/SPSCLogger.hh
[Logger]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/utils/Logger.hh
[SimpleLogger]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/utils/SimpleLogger.hh
[Timer]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/utils/Timer.hh
[TSCTimer]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/utils/TSCTimer.hh
[TimeStamp]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/utils/TimeStamp.hh
[TomlParser]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/utils/TomlParser.hh
[Types]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/utils/Types.hh
[TypeFlags]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/utils/TypeFlags.hh
[XorBitant]: https://gitgud.boo/xbazzi/fastinahurry/src/branch/master/include/fiah/utils/XorBitant.hh
