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
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include "bnet/asyncprocessor.h"

#define LOG std::cerr << __FILE__ << ":" <<  __LINE__ << " " <<  __FUNCTION__ << "---"

namespace bnet
{

/**
��������
0   ���ݰ���δ������������������
>0  ��ʾ�����Ѿ����������Ҹ�ֵ��ʾ���ݰ��ĳ���
<0  ��ʾ����
 */
typedef boost::function<int (const char*, std::size_t)> ProtoParserFunc;

int PrefixLenParser(const char* buf, std::size_t size);

class TCPChannel : public boost::noncopyable
{
protected:
    TCPChannel(AsyncProcessor* processor);
    virtual ~TCPChannel();

public:
    void SetProtoParser(const ProtoParserFunc& parser)
    {
        parser_ = parser;
    }

    virtual int ProcessPacket(const std::string& packet) = 0;

    void InitRead()
    {
        AsyncReadSome(temp_recvbuf_, 2048, boost::bind(&TCPChannel::HandleInput, this, _1, _2));
    }

    void HandleInput(const boost::system::error_code& error, std::size_t bytes_transferred)
    {
        if(!error)
        {
            //parse protocol
            if(bytes_transferred > 0)
            {
                LOG << "recv msg size: " << bytes_transferred << std::endl;

                recvbuf_.append(temp_recvbuf_, bytes_transferred);
                while(true)
                {
                    if(!parser_)
                    {
                        LOG << "protocol parser not set!!!" << std::endl;
                        Close();

                        return;
                    }
                    int ret = parser_(recvbuf_.data(), recvbuf_.size());
                    if(ret >0)
                    {
                        std::string packet(recvbuf_.data(), ret);
                        LOG << "process a full packet size: " << packet.size() << std::endl;
                        //�����
                        ProcessPacket(packet);
                        //������С
                        recvbuf_ = recvbuf_.substr(ret);
                        LOG << "recvbuf size: " << recvbuf_.size() << std::endl;
                    }
                    else if(ret ==0)
                    {
                        AsyncReadSome(temp_recvbuf_, 2048, boost::bind(&TCPChannel::HandleInput, this, _1, _2));
                        break;
                    }
                    else
                    {
                        //������ʧ�ܣ��ص����Ӱ�
                        LOG << "packet parse fail" << std::endl;
                        break;
                    }
                }
            }
            else
            {
                //fixme
                LOG << "Fatal error, " << bytes_transferred << std::endl;
                Close();
            }
        }
        else
        {
            //fixme
            LOG << error.message() << std::endl;
            Close();
        }
    }

    void HandleOutput(const boost::system::error_code& error, std::size_t bytes_transferred)
    {
        if(!error)
        {
            //����buffer����
            sendbuf_.substr(bytes_transferred);
            if(!sendbuf_.empty())
            {
                //����������ݣ���������
                AsyncWriteSome(sendbuf_.data(), sendbuf_.size(),
                boost::bind(&TCPChannel::HandleOutput, this, _1, _2));
            }
        }
        else
        {
            LOG << "send msg error: " << error.message() << std::endl;
        }
    }

    /**
     *  �첽��������
     *  @param[in] buffer.
     *  @param[in] size.
     *  @return void.
    */
    void Write(const char* buffer, std::size_t size)
    {
        //���ӵ�sendbuf����
        sendbuf_.append(buffer, size);
        //�����������е�����
        AsyncWriteSome(sendbuf_.data(), sendbuf_.size(),
                       boost::bind(&TCPChannel::HandleOutput, this, _1, _2));
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

protected:
    boost::asio::ip::tcp::socket& socket()
    {
        return socket_;
    }

    virtual void OnConnect();
    virtual void OnClose(const boost::system::error_code& ec);

private:

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
            LOG << "socket is not opened!!!" << std::endl;
        }
    }

    template<typename WriteHandler>
    void AsyncWriteSome(const char* buffer, std::size_t size, WriteHandler handler)
    {
        if (socket_.is_open())
        {
            socket_.async_write_some(boost::asio::buffer(buffer, size),handler);
        }
        else
        {
            //throw Exception
            LOG << "socket is not opened!!!" << std::endl;
        }
    }

protected:
    AsyncProcessor* ower_processor_;
    boost::asio::ip::tcp::socket socket_;
    bool connected_;
    std::string remote_address_;
    uint16_t remote_port_;
    std::string recvbuf_;
    //��ʱ���հ�buf
    char temp_recvbuf_[2048];
    std::string sendbuf_;
    ProtoParserFunc parser_;
};

}
#endif // BNET_TCPCHANNEL_H_
