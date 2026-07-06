// Compile-check translation unit 

// Error types
#include "fiah/error/Error.hh"

// Handles
// SharedPtr.hh excluded: duplicates default_deleter (WIP practice file)
#include "fiah/handle/UniquePtr.hh"

// Utils
#include "fiah/utils/Timer.hh"
#include "fiah/utils/TimeStamp.hh"
#include "fiah/utils/TSCTimer.hh"
#include "fiah/utils/Logger.hh"
#include "fiah/utils/SimpleLogger.hh"
#include "fiah/utils/TomlParser.hh"
#include "fiah/utils/XorBitant.hh"
#include "fiah/utils/SPSCLogger.hh"

// IO
#include "fiah/io/Socket.hh"
#include "fiah/io/Tcp.hh"
#include "fiah/io/TcpClient.hh"
#include "fiah/io/TcpServer.hh"
#include "fiah/io/Udp.hh"
#include "fiah/io/Config.hh"

// Math
#include "fiah/math/AutoDiff.hpp"
#include "fiah/math/FiniteDiff.hpp"
#include "fiah/math/NewtonRaphson.hpp"

// Structs
#include "fiah/structs/Orderbook.hh"
#include "fiah/structs/SPSCQueue.hh"
#include "fiah/structs/ThreadSafeQueue.hh"
#include "fiah/structs/Vector.hh"
#include "fiah/structs/MPSCQueue.hh"

// Threads
#include "fiah/thread/SpinMutex.hpp"
#include "fiah/thread/ThreadPool.hpp"

// Memory 
#include "fiah/memory/BumpAllocator.hh"
#include "fiah/memory/BumpArena.hh"