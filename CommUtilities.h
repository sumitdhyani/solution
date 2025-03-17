#include <functional>
#include <optional>
#include <tuple>

enum class ConnType
{
  TCP,
  UDP,
  WEB_SOCKET,
};

template <class CommError>
using ErrHandler = std::function<void(const CommError&)>;

template <class CommError>
struct CommFunctions
{
  using CloseHandler = std::function<void(const CommError&)>;
  using ReadHandler = std::function<void(const uint32_t&, const ErrHandler<CommError>&)>;
  using WriteHandler = ReadHandler;

  std::function<void(const uint32_t&, const ReadHandler&)> read;
  std::function<void(const uint32_t&, const WriteHandler&)> write;
  std::function<void(const CloseHandler&)> close;
};

template <class CommError>
using AcceptHandler = std::function<bool(const std::optional<CommFunctions<CommError>>&, const std::optional<CommError&>)>;

template <class CommError>
using ConnectHandler = AcceptHandler<CommError>;

template <class CommError>
using ListenFunc = std::function<void(const uint16_t&, const ConnType&, const AcceptHandler<CommError>&)>;

template <class CommError>
using ConnectFunc = std::function<void(const uint16_t&, const ConnType&, const ConnectHandler<CommError>&)>;

template <class CommError>
using IOFunctions = std::tuple<ListenFunc<CommError>, ConnectFunc<CommError>>;

template <class CommError>
using IOFunctionsCallback = std::function<void(const IOFunctions<CommError>&)>;

template <class CommError>
void getCommFunctions(const uint16_t& numThreads, const IOFunctionsCallback<CommError>&);