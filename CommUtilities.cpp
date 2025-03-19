#include <memory>
#include <thread>
#include <string>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include "CommUtilities.h"


namespace CommUtilities
{
  typedef boost::system::error_code BoostCommError;
  typedef boost::asio::io_context IOService;
  typedef IOService::strand Strand;
  typedef boost::asio::ip::tcp::socket VanillaSocket;
  typedef boost::asio::ip::tcp::acceptor Acceptor;
  typedef boost::asio::ip::tcp::endpoint Endpoint;
  typedef boost::asio::deadline_timer Scheduler;
  typedef boost::asio::ip::tcp::resolver Resolver;

  namespace
  {
    CommFunctions<BoostCommError> getCommFunctions(const std::shared_ptr<VanillaSocket>& sock)
    {
      using CloseHandler = CommFunctions<BoostCommError>::CloseHandler;
      using ReadHandler = CommFunctions<BoostCommError>::ReadHandler;
      using WriteHandler = CommFunctions<BoostCommError>::WriteHandler;
      auto read =
      [sock](char* const& readBuff, const SizeType& len, const ReadHandler& readHandler)
        {
            sock->async_read_some(boost::asio::buffer(readBuff, len), [readHandler](const BoostCommError& err, const size_t& len){
              std::optional<BoostCommError> error = err? std::optional<BoostCommError>(err) : std::nullopt;
              readHandler(len, error);
            });
        };

      auto write =
      [sock](const char* writeBuff, const SizeType& len, const WriteHandler& writeHandler)
      {
          sock->async_write_some(boost::asio::buffer(writeBuff, len), [writeHandler](const BoostCommError& err, const size_t& len){
            std::optional<BoostCommError> error = err? std::optional<BoostCommError>(err) : std::nullopt;
            writeHandler(len, error);
          });
      };

      auto close =
      [sock](const CloseHandler& closehandler)
      {
        try
        {
          sock->close();
        }
        catch(const std::exception&)
        {
        }
        catch(...)
        {

        }
        
        closehandler(std::nullopt);
      };

      return {read, write, close};
    }

    using ConnCallback = std::function<bool(const std::optional<std::shared_ptr<VanillaSocket>>&, const BoostCommError&)>;

    void startAcceptingConnections(const std::shared_ptr<IOService>& ioService,
                                   const uint16_t& port,
                                   const ConnCallback& connCallback)
    {

      std::function<void()> asycAcceptLoop =
      [ioService,
       acceptor = std::make_shared<Acceptor>(*ioService, Endpoint(boost::asio::ip::tcp::v4(), port)),
       connCallback,
       &asycAcceptLoop]
      ()
      {
        auto sock = std::make_shared<VanillaSocket>(*ioService);
        auto onAcceptOrReject =
        [sock, asycAcceptLoop, acceptor, connCallback](const BoostCommError& err)
        {
          if (connCallback(sock, err) && !err)
          {
            asycAcceptLoop();
          }
        };

        acceptor->async_accept(*sock, onAcceptOrReject);
      };

      asycAcceptLoop();
    }
  
  
    
  

    void listen(const std::shared_ptr<IOService>& ioService,
                const uint16_t& port,
                const SizeType& buffSize,
                const AcceptanceHandler<BoostCommError>& callback)
    {
      ConnCallback connCallback =
      [callback](const std::optional<std::shared_ptr<VanillaSocket>>& sock, const BoostCommError& err)
      {
        if (err)
        {
          callback(std::nullopt, err);
          return false;
        }

        callback(getCommFunctions(sock.value()), std::nullopt);
        return true;
      };

      startAcceptingConnections(ioService, port, connCallback);
    }
  
    void startAttemptingConnection(const std::shared_ptr<IOService>& ioService,
                                   const std::string& ip,
                                   const uint16_t& port,
                                   const ConnCallback& connCallback);
                                   
    void onResolve(const BoostCommError& err,
                   const Resolver::results_type& results,
                   const std::shared_ptr<IOService>& ioService,
                   const std::string& ip,
                   const uint16_t& port,
                   const ConnCallback& connCallback)
    {
      if (err)
      {
        return;
      }

      auto sock = std::make_shared<VanillaSocket>(*ioService);
      sock->async_connect(Endpoint(boost::asio::ip::address(), port),
                          [sock, ip, port, ioService, connCallback]
                          (const BoostCommError& err)
                          {
                            if (connCallback(sock, err))
                            {
                              startAttemptingConnection(ioService, ip, port, connCallback);
                            }
                          });
    }

    void startAttemptingConnection(const std::shared_ptr<IOService>& ioService,
                                   const std::string& ip,
                                   const uint16_t& port,
                                   const ConnCallback& connCallback)
    {
      auto resolver = std::make_shared<Resolver>(*ioService);
      std::function<void(const BoostCommError&, const Resolver::results_type&)> addressResolutionCallback =
      [resolver, ioService, ip, port, connCallback]
      (const BoostCommError& ec, const Resolver::results_type& results)
      {
        onResolve(ec, results, ioService, ip, port, connCallback);
      };

      resolver->async_resolve(ip, std::to_string(port), addressResolutionCallback);
    }


    void connect(const std::shared_ptr<IOService>& ioService,
                 const std::string& ip,
                 const uint16_t& port,
                 const SizeType buffSize,
                 const ConnectHandler<BoostCommError>& callback)
    {
      ConnCallback connCallback =
      [callback](const std::optional<std::shared_ptr<VanillaSocket>>& sock, const BoostCommError& err)
      {
        std::optional<CommFunctions<BoostCommError>> commFunctions = std::nullopt; 
        std::optional<BoostCommError> error = std::nullopt;
        if (err)
        {
           error = err;
        }

        if(!error)
        {
          commFunctions = getCommFunctions(sock.value()); 
        }

        return callback(commFunctions, error);
      };

      startAttemptingConnection(ioService, ip, port, connCallback);
    }
  }  

  void getIOFunctions(const uint16_t& numThreads, const IOFunctionsCallback<BoostCommError>& commFunctionCallback)
  {
    auto ioservice = std::make_shared<IOService>();
    auto work = boost::asio::make_work_guard(ioservice);

    std::thread* threads = nullptr;
    if (numThreads > 1)
    {
      threads = new std::thread[numThreads - 1];
      for (uint16_t i = 0; i < numThreads - 1; ++i)
      {
        threads[i] = std::thread([ioservice](){ ioservice->run(); });
      }
    }

    ListenFunc<BoostCommError> listenFunc =
    [ioservice](const uint16_t& port,
       const ConnType& connType,
       const SizeType& buffSize,
       const AcceptanceHandler<BoostCommError>& acceptHandler)
    {
      listen(ioservice, port, buffSize, acceptHandler);
    };

    ConnectFunc<BoostCommError> connectFunc =
    [ioservice](const uint16_t& port,
       const std::string& ip,
       const SizeType& buffSize,
       const ConnType& connType,
       const ConnectHandler<BoostCommError>& connectHandler)
    {
      connect(ioservice, ip, port, buffSize, connectHandler);
    };

    commFunctionCallback({listenFunc, connectFunc});

    ioservice->run();

    if (numThreads > 1)
    {
      for (uint16_t i = 0; i < numThreads - 1; ++i)
      {
        threads[i].join();
      }

      delete[] threads;
    }
    
  }
}

int main()
{
  return 0;
}