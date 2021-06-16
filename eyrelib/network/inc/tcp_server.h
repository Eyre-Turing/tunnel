#ifndef TCP_SERVER_H
#define TCP_SERVER_H

/*
 * For start a tcp server easily.
 * All message is ByteArray, so need Eyre Turing lib framework.
 * Compile need -lpthread.
 * MinGW compile need -lws2_32.
 *
 * Author: Eyre Turing.
 * Last edit: 2021-06-13 18:02.
 */

#ifdef _WIN32
#include <winsock.h>
#include <queue>
#else
#include <sys/socket.h>
#endif

#include <map>
#include <pthread.h>

#define TCP_SERVER_CLOSED	0
#define TCP_SERVER_RUNNING	1

#define TCP_SERVER_READYTORUN			0	//aka succeed.
#define TCP_SERVER_BIND_ERROR			(-1)	//aka the port you want maybe occupied.
#define TCP_SERVER_SOCKETFD_ERROR		(-2)
#define TCP_SERVER_LISTEN_ERROR		(-3)
#define TCP_SERVER_CREATETHREAD_ERROR	(-4)

class TcpSocket;
#include "tcp_socket.h"

class TcpServer
{
public:
	typedef void (*NewConnecting)(TcpServer *server, TcpSocket *client);
	typedef void (*StartSucceed)(TcpServer *server);
	typedef void (*Closed)(TcpServer *server);
	
	TcpServer();
	virtual ~TcpServer();
	
	//return error type, if return TCP_SERVER_READYTORUN {aka 0} is succeed.
	int start(unsigned short port, int family=AF_INET, unsigned long addr=INADDR_ANY, int backlog=5);
	void abort();
	
	void setNewConnectingCallBack(NewConnecting newConnecting);
	void setStartSucceedCallBack(StartSucceed startSucceed);
	void setClosedCallBack(Closed closed);

	int runStatus() const;
	
	class Thread
	{
	public:
		static void *selectThread(void *s);
	};
	
	friend class TcpSocket;
	
private:
#ifdef _WIN32
	WSADATA m_wsadata;
	
	/*
	 * Windows system remove a sockfd need --fd in for loop,
	 * so removeClient() don't remove sockfd, removeClient() append
	 * sockfd which need remove to m_waitForRemoveSockfds, and
	 * selectThread will remove the sockfd.
	 */
	std::queue<int> m_waitForRemoveSockfds;
	pthread_mutex_t m_waitForRemoveSockfdsMutex;
	SOCKET m_sockfd;
#else
	int m_sockfd;
#endif
	int m_runStatus;
	
	fd_set m_readfds;
	pthread_mutex_t m_readfdsMutex;
	
	pthread_t m_listenThread;
	
	NewConnecting m_onNewConnecting;
	StartSucceed m_onStartSucceed;
	Closed m_onClosed;

#ifdef _WIN32
	std::map<SOCKET, TcpSocket *> m_clientMap;
#else
	std::map<int, TcpSocket *> m_clientMap;
#endif
	pthread_mutex_t m_clientMapMutex;
	
	/*
	 * Will create a TcpSocket* which use clientSockfd to send an recv message,
	 * and add pair(clientSockfd, created TcpSocket*) to m_clientMap.
	 * And add clientSockfd to m_readfds.
	 */
#ifdef _WIN32
	TcpSocket *appendClient(SOCKET clientSockfd);
#else
	TcpSocket *appendClient(int clientSockfd);
#endif
	pthread_mutex_t m_readfdsMutexInAppend;
	
	/*
	 * Will close clientSockfd, remove clientSockfd from m_readfds
	 * and call back onDisconnected.
	 * Note: this function don't auto delete the TcpSocket*.
	 */
#ifdef _WIN32
	bool removeClient(SOCKET clientSockfd);
#else
	bool removeClient(int clientSockfd);
#endif
	pthread_mutex_t m_readfdsMutexInRemove;
};

#endif	//TCP_SERVER_H
