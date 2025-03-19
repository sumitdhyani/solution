#pragma once
#include <functional>
#include <optional>
#include <tuple>
namespace CommUtilities
{
  typedef uint64_t SizeType;
  enum class ConnType
  {
    TCP,
    UDP,
    WEB_SOCKET,
  };

  template <class CommError>
  struct CommFunctions
  {
    using CloseHandler = std::function<void(const std::optional<CommError>&)>;
    using ReadHandler = std::function<void(const SizeType&, std::optional<CommError>&)>;
    using WriteHandler = ReadHandler;

    std::function<void(char* const&, const SizeType&, const ReadHandler&)> read;
    std::function<void(const char*, const SizeType&, const WriteHandler&)> write;
    std::function<void(const CloseHandler&)> close;
  };

  template <class CommError>
  using AcceptanceHandler = std::function<bool(const std::optional<CommFunctions<CommError>>&, const std::optional<CommError>&)>;

  template <class CommError>
  using ConnectHandler = AcceptanceHandler<CommError>;

  template <class CommError>
  using ListenFunc = std::function<void(const uint16_t&, const ConnType&, const SizeType&, const AcceptanceHandler<CommError>&)>;

  template <class CommError>
  using ConnectFunc = std::function<void(const uint16_t&, const std::string&, const SizeType&, const ConnType&, const ConnectHandler<CommError>&)>;

  template <class CommError>
  using IOFunctions = std::tuple<ListenFunc<CommError>, ConnectFunc<CommError>>;

  template <class CommError>
  using IOFunctionsCallback = std::function<void(const IOFunctions<CommError>&)>;

  template <class CommError>
  void getIOFunctions(const uint16_t& numThreads, const IOFunctionsCallback<CommError>&);
}