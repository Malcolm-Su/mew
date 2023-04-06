#ifndef MEW_CALLBACK_H
#define MEW_CALLBACK_H

#include <functional>
#include <memory>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

namespace mew {

class TcpConnection;
class Buffer;
class EventLoop;

using EventTask = std::function<void()>;
using LoopTask = std::function<void()>;
using TimerTask = std::function<void()>;


using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

using TcpConnectionTask = std::function<void (const TcpConnectionPtr&, Buffer*)>;
using TcpMessageTask    = std::function<void (const TcpConnectionPtr&, Buffer*)>;
using TcpCloseTask      = std::function<void (const TcpConnectionPtr&)>;

} // namespace mew
#endif // MEW_CALLBACK_H