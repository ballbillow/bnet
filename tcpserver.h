//
// Copyright (c) billowqiu (billowqiu@163.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//  See www.boost.org/libs/asio for documentation.
//

#ifndef BNET_TCPSERVER_H_
#define BNET_TCPSERVER_H_

#include <stdint.h>
#include <set>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include "bnet/processorpool.h"
namespace bnet 
{

class AsyncProcessor;
class TCPConnection;

class TCPServer : public boost::noncopyable
{
public:
    TCPServer(AsyncProcessor* processor, uint16_t port);
    virtual ~TCPServer();

public:
    /** 
     *  ���ô���I/O���߳�����������Start֮ǰ����
     *  @param[in] numthreads.
     *  0,���е�I/O���ڵ�ǰ�߳���
     *  >0,��accept�������⣬����I/Oͨ��round-robin�㷨ѡȡһ��processor����
     *  @return void.
    */
    void SetThreadNum(std::size_t numthreads);
    void Start();
    void Stop();
    void CloseAcceptor();
    void DestroyConnection(TCPConnection* pSocketSession);
    
private:
    void AsyncAccept();
    void HandleAccept(TCPConnection* pSession, 
                      const boost::system::error_code& error);

protected:
    virtual TCPConnection* CreateConnection(AsyncProcessor* processor) = 0;

private:
    AsyncProcessor* base_processor_;    //��Ҫ��Ϊaccept
    ProcessorPool processor_pool_;
    boost::asio::ip::tcp::acceptor acceptor_;    
    std::set<TCPConnection*> sessions_set_;
};
}

#endif // BNET_TCPSERVER_H_
