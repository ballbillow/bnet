//
// Copyright (c) billowqiu (billowqiu@163.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//  See www.boost.org/libs/asio for documentation.
//

#ifndef BNET_TCPCHANNEL_H_
#define BNET_TCPCHANNEL_H_
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include "bnet/asyncprocessor.h"

namespace bnet
{

class TCPChannel : public boost::noncopyable
{
protected:
    TCPChannel(AsyncProcessor* processor);
    virtual ~TCPChannel ();
    
public:
    template<typename _Type, typename ReadHandler>
    void Read(_Type& buffer, ReadHandler handler)
    {
        AsyncRead(reinterpret_cast<char*>(&buffer), sizeof(_Type), handler);
    }
    
    /** 
     *  ��ȡsize���ȵ����ݵ�buffer
     *  @param[in/out] buffer.
     *  @param[in/out] size.
     *  @param[in/out] handler.
     *  @return void.
    */
    template<typename ReadHandler>
    void Read(char* buffer, std::size_t size, ReadHandler handler)
    {
        AsyncRead(buffer, size, handler);
    }
    /** 
     *  ����֤��ȡsize��С���ݣ������ݾͻ�ص�
     *  @param[in/out] buffer.
     *  @param[in/out] size.
     *  @param[in/out] handler.
     *  @return void.
    */
    template<typename ReadHandler>
    void ReadSome(char* buffer, std::size_t size, ReadHandler handler)
    {
        AsyncReadSome(buffer, size, handler);
    }

    template<typename _Type, typename WriteHandler>
    void Write(const _Type& buffer, WriteHandler handler)
    {
        AsyncWrite(reinterpret_cast<const char*> (&buffer), sizeof (_Type), handler);
    }
    /** 
     *  �첽��������
     *  @param[in] buffer.
     *  @param[in] size.
     *  @param[in] handler��������ɺ�Ļص�����
     *  @return void.
    */
    template<typename WriteHandler>
    void Write(const char* buffer, std::size_t size, WriteHandler handler)
    {
        AsyncWrite (buffer, size, handler);
    }

    /** 
     *  ͬ����������
     *  @param[in] buffer.
     *  @param[in] size.
     *  @return int.
    */
    std::size_t SyncWrite(const char* buffer, std::size_t size, boost::system::error_code& ec);

    bool Connected() const;
    virtual void Close();
private:

    /** 
     *  ��ȡsize��С���ݣ��Ż����handler
     *  @param[in/out] buffer.
     *  @param[in/out] size.
     *  @param[in/out] handler.
     *  @return void.
    */
    template<typename ReadHandler>
    void AsyncRead(char* buffer, std::size_t size, ReadHandler handler)
    {
        if (socket_.is_open())
        {
            boost::asio::async_read(socket_, boost::asio::buffer(buffer, size),
                                    boost::asio::transfer_at_least(size),
                                    handler);
        }
        else
        {
            //throw Exception
        }
    }
    
    /** 
     *  ��ȡ���ݣ�����֤��ȡsize��С������
     *  @param[in/out] buffer.
     *  @param[in/out] size.
     *  @param[in] handler.
     *  @return void.
    */
    template<typename ReadHandler>
    void AsyncReadSome(char* buffer, std::size_t size, ReadHandler handler)
    {
        if (socket_.is_open())
        {
            socket_.async_read_some(boost::asio::buffer(buffer, size),handler);
        }
        else
        {
            //throw Exception
        }
    }
    template<typename WriteHandler>
    void AsyncWrite (const char* buffer, std::size_t size, WriteHandler handler)
    {
        if (socket_.is_open())
        {
            boost::asio::async_write(socket_, boost::asio::buffer(buffer, size), handler);
        }
        else
        {
            //throw Exception
        }
    }

protected:
    boost::asio::ip::tcp::socket& socket()
    {
        return socket_; 
    }
    
    virtual void OnConnect();
    virtual void OnClose(const boost::system::error_code& ec);
    
protected:
    AsyncProcessor* ower_processor_;
    boost::asio::ip::tcp::socket socket_;
    bool connected_;
    std::string remote_address_;
    uint16_t remote_port_;
};

}
#endif // BNET_TCPCHANNEL_H_
