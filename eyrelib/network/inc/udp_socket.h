#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

/*
 * Compile need -lpthread.
 * MinGW compile need -lws2_32.
 * 
 * Author: Eyre Turing.
 * Last edit: 2021-01-10 14:51.
 */

#ifdef _WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#endif	//_WIN32

#include <pthread.h>
#include "byte_array.h"

class UdpSocket
{
public:
	typedef void (*Read)(UdpSocket *s, String ip, unsigned short port, ByteArray data);
	
	UdpSocket();
	virtual ~UdpSocket();
	
	bool start(unsigned short port, int family=AF_INET, unsigned long addr=INADDR_ANY);
	void unbind();
	
	bool send(const char *addr, unsigned short port, const ByteArray &data) const;
	bool send(const char *addr, unsigned short port,
				const char *data, unsigned int size=0xffffffff) const;
	
	void setReadCallBack(Read read);
	
	bool isBound() const;
	
	class Thread
	{
	public:
		static void *readThread(void *s);
	};
	
private:
#ifdef _WIN32
	WSADATA m_wsadata;
	SOCKET m_sockfd;
#else
	int m_sockfd;
#endif
	bool m_isBound;
	unsigned short m_port;
	
	pthread_t m_readThread;
	
	Read m_onRead;
};

#endif	//UDP_SOCKET_H
