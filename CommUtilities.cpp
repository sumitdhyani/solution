#include <memory>
#include <thread>
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include "CommUtilities.h"



typedef boost::system::error_code CommError;
typedef boost::asio::io_context IOService;
typedef IOService::strand Strand;
typedef boost::asio::ip::tcp::socket VanillaSocket;
typedef boost::asio::ip::tcp::acceptor Acceptor;
typedef boost::asio::ip::tcp::endpoint Endpoint;
typedef boost::asio::deadline_timer Scheduler;

void getIOFunctions(const uint16_t& numThreads, const IOFunctionsCallback<CommError>& commFunctionCallback)
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

int main()
{
  return 0;
}