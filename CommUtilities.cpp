#include <memory>
#include <thread>
#include <string>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include "CommUtilities.h"


namespace CommUtilities
{
  typedef boost::system::error_code CommErr;
  typedef boost::asio::io_context IOService;
  typedef IOService::strand Strand;
  typedef boost::asio::ip::tcp::socket VanillaSocket;
  typedef boost::asio::ip::tcp::acceptor Acceptor;
  typedef boost::asio::ip::tcp::endpoint Endpoint;
  typedef boost::asio::deadline_timer Scheduler;
  typedef boost::asio::ip::tcp::resolver Resolver;

  namespace
  {
    CommFunctions<CommErr> getCommFunctions(const std::shared_ptr<VanillaSocket>& sock)
    {
      using CloseHandler = CommFunctions<CommErr>::CloseHandler;
      using ReadHandler = CommFunctions<CommErr>::ReadHandler;
      using WriteHandler = CommFunctions<CommErr>::WriteHandler;
      auto read =
      [sock](char* const& readBuff, const SizeType& len, const ReadHandler& readHandler)
        {
            sock->async_read_some(boost::asio::buffer(readBuff, len), [readHandler](const CommErr& err, const size_t& len){
              std::optional<CommErr> error = err? std::optional<CommErr>(err) : std::nullopt;
              readHandler(len, error);
            });
        };

      auto write =
      [sock](const char* writeBuff, const SizeType& len, const WriteHandler& writeHandler)
      {
          sock->async_write_some(boost::asio::buffer(writeBuff, len), [writeHandler](const CommErr& err, const size_t& len){
            std::optional<CommErr> error = err? std::optional<CommErr>(err) : std::nullopt;
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

    using ConnCallback = std::function<bool(const std::optional<std::shared_ptr<VanillaSocket>>&, const CommErr&)>;

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
        [sock, asycAcceptLoop, acceptor, connCallback](const CommErr& err)
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
                const AcceptanceHandler<CommErr>& callback)
    {
      ConnCallback connCallback =
      [callback](const std::optional<std::shared_ptr<VanillaSocket>>& sock, const CommErr& err)
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
                                   
    void onResolve(const CommErr& err,
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
                          (const CommErr& err)
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
      std::function<void(const CommErr&, const Resolver::results_type&)> addressResolutionCallback =
      [resolver, ioService, ip, port, connCallback]
      (const CommErr& ec, const Resolver::results_type& results)
      {
        onResolve(ec, results, ioService, ip, port, connCallback);
      };

      resolver->async_resolve(ip, std::to_string(port), addressResolutionCallback);
    }


    void connect(const std::shared_ptr<IOService>& ioService,
                 const std::string& ip,
                 const uint16_t& port,
                 const SizeType buffSize,
                 const ConnectHandler<CommErr>& callback)
    {
      ConnCallback connCallback =
      [callback](const std::optional<std::shared_ptr<VanillaSocket>>& sock, const CommErr& err)
      {
        std::optional<CommFunctions<CommErr>> commFunctions = std::nullopt; 
        std::optional<CommErr> error = std::nullopt;
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

  void getIOFunctions(const uint16_t& numThreads, const IOFunctionsCallback<CommErr>& commFunctionCallback)
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

    ListenFunc<CommErr> listenFunc =
    [ioservice](const uint16_t& port,
       const ConnType& connType,
       const SizeType& buffSize,
       const AcceptanceHandler<CommErr>& acceptHandler)
    {
      listen(ioservice, port, buffSize, acceptHandler);
    };

    ConnectFunc<CommErr> connectFunc =
    [ioservice](const uint16_t& port,
       const std::string& ip,
       const SizeType& buffSize,
       const ConnType& connType,
       const ConnectHandler<CommErr>& connectHandler)
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