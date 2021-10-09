#include "eyre_turing_lib.h"
#include "eyre_turing_network.h"
#include <iostream>
#include <map>
#include <vector>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/*
 * 添加了控制窗口支持（不支持Windows）
 * 可以在本级目录下使用: ./control.sh 控制本服务
 * 本代码主要添加了管理端口，可以使用管理端口关闭该服务（管理端口连接发送 任何信息 即可关闭服务 ）
 */

using namespace std;

TcpServer *serverToClient = NULL;
TcpServer *serverToUser = NULL;
#ifndef _WIN32
TcpServer *manager = NULL;	// 服务管理
bool beKilled = false;	// 被管理端口叫关闭
#endif

TcpSocket *virtualClient = NULL;

map<String, TcpSocket *> users;
pthread_mutex_t usersMutex;

ByteArray head;
ByteArray buffer;
String sender;
unsigned long long restBufferSize = 0;

struct Message
{
	String type;
	String id;
	ByteArray data;
};
vector<Message> m_messages;
pthread_mutex_t messagesMutex;

bool printMessage = false;

void onNewConnection(TcpServer *server, TcpSocket *client);
void onStartSucceed(TcpServer *server);
void onClosed(TcpServer *Server);
void onDisconnected(TcpSocket *tcpSocket);
void onRead(TcpSocket *tcpSocket, ByteArray data);

pthread_mutex_t tellMutex;
void tellToVirtualClient(ByteArray &b)
{
	if(virtualClient)
	{
		pthread_mutex_lock(&tellMutex);
		virtualClient->write(b);
		pthread_mutex_unlock(&tellMutex);
	}
}

pthread_mutex_t serverConnectMutex;
void onNewConnecting(TcpServer *server, TcpSocket *client)
{
	pthread_mutex_lock(&serverConnectMutex);
	if(server == serverToClient)
	{
		cout<<"virtual server is connected. virtual client coming now."<<endl;
		if(virtualClient)
		{
			cout<<"virtual client connected before. "
				"kill and new virtual client connect."<<endl;
			virtualClient->setDisconnectedCallBack(NULL);
			virtualClient->abort();
		}
		virtualClient = client;
		client->setDisconnectedCallBack(onDisconnected);
		client->setReadCallBack(onRead);
	}
	else if(server == serverToUser)
	{
		char id[32];
		sprintf(id, "%p", client);
		
		cout<<"proxy server is connected. user "<<id<<" coming now."<<endl;
		
		/*
		 * tell virtual client that user connected.
		 * format:
		 * c:id#
		 */
		if(virtualClient)
		{
			pthread_mutex_lock(&usersMutex);
			users[String::fromUtf8(id)] = client;
			pthread_mutex_unlock(&usersMutex);
			client->setDisconnectedCallBack(onDisconnected);
			client->setReadCallBack(onRead);
			
			cout<<"now user count: "<<users.size()<<endl;
		
			ByteArray sendMessage = "c:"+ByteArray(id)+"#";
			tellToVirtualClient(sendMessage);
			cout<<"told to virtual client."<<endl;
		}
		else
		{
			cout<<"but virtual client disconnect. kill this user."<<endl;
			client->abort();
		}
	}
	else
	{
#ifdef _WIN32
		cout<<"unknow server is connected."<<endl;
#else
		if(server == manager)
		{
			cout<<"manager comming."<<endl;
			client->setDisconnectedCallBack(onDisconnected);
			client->setReadCallBack(onRead);
		}
#endif
	}
	pthread_mutex_unlock(&serverConnectMutex);
}

pthread_mutex_t serverStartMutex;
void onStartSucceed(TcpServer *server)
{
	pthread_mutex_lock(&serverStartMutex);
	if(server == serverToClient)
	{
		cout<<"virtual server started."<<endl;
	}
	else if(server == serverToUser)
	{
		cout<<"proxy server started."<<endl;
	}
	else
	{
#ifdef _WIN32
		cout<<"unknow server started."<<endl;
#else
		if(server == manager)
		{
			cout<<"proxy manager started."<<endl;
		}
#endif
	}
	pthread_mutex_unlock(&serverStartMutex);
}

void onClosed(TcpServer *server)
{
	
}

pthread_mutex_t socketDisconnectMutex;
void onDisconnected(TcpSocket *tcpSocket)
{
	pthread_mutex_lock(&socketDisconnectMutex);
	if(tcpSocket == virtualClient)
	{
		cout<<"virtual client disconnected."<<endl;
		virtualClient = NULL;
		
		// kill all user
		pthread_mutex_lock(&usersMutex);
		for(map<String, TcpSocket *>::iterator it = users.begin();
			it != users.end();)
		{
			it->second->setDisconnectedCallBack(NULL);
			it->second->abort();
			users.erase(it++);
		}
		pthread_mutex_unlock(&usersMutex);
		cout<<"kill all user, now user count: "<<users.size()<<endl;
	}
	else
	{
		char id[32];
		sprintf(id, "%p", tcpSocket);
		pthread_mutex_lock(&usersMutex);
		users.erase(users.find(id));
		pthread_mutex_unlock(&usersMutex);
		
		cout<<"user "<<id<<" disconnected. now user count: "
			<<users.size()<<endl;
		
		if(virtualClient)
		{
			/*
			 * tell virtual client that user disconnected.
			 * format:
			 * d:id#
			 */
			ByteArray sendMessage = "d:"+ByteArray(id)+"#";
			tellToVirtualClient(sendMessage);
		}
	}
	pthread_mutex_unlock(&socketDisconnectMutex);
}

void messageRead(ByteArray &message)
{
	while(message.size())
	{
		if(restBufferSize)
		{
			unsigned long long len = message.size();
			if(restBufferSize > len)
			{
				buffer += message;
				message = "";
				restBufferSize -= len;
			}
			else
			{
				buffer += message.mid(0, restBufferSize);
				message = message.mid(restBufferSize);
				restBufferSize = 0;
			}
			if(!restBufferSize)
			{
				Message m = {"m", sender, buffer};
				pthread_mutex_lock(&messagesMutex);
				m_messages.push_back(m);
				pthread_mutex_unlock(&messagesMutex);
				sender = "";
				buffer = "";
			}
			else
			{
				continue;
			}
		}
		message = head+message;
		head = "";
		int endSymPos = message.indexOf("#");
		if(endSymPos == -1)
		{
			head += message;
			message = "";
		}
		else
		{
			ByteArray _head = message.mid(0, endSymPos);
			vector<ByteArray> headInfo = _head.split(":");
			pthread_mutex_lock(&messagesMutex);
			if(headInfo[0] == "c")	// client connected
			{
				// c:id
				if(headInfo.size() == 2)
				{
					String id = String::fromUtf8(headInfo[1]);
					Message m = {"c", id, ""};
					m_messages.push_back(m);
				}
			}
			else if(headInfo[0] == "d")	// client disconnected
			{
				// d:id
				if(headInfo.size() == 2)
				{
					String id = String::fromUtf8(headInfo[1]);
					Message m = {"d", id, ""};
					m_messages.push_back(m);
				}
			}
			else if(headInfo[0] == "m")	// send message
			{
				// m:id;length
				if(headInfo.size() == 2)
				{
					vector<ByteArray> info = headInfo[1].split(";");
					if(info.size() == 2)
					{
						sender = String::fromUtf8(info[0]);
						restBufferSize = String::fromUtf8(info[1]).toUInt64();
					}
				}
			}
			else if(headInfo[0] == "a")
			{
				// a
				Message m = {"a", "", ""};
				m_messages.push_back(m);
			}
			pthread_mutex_unlock(&messagesMutex);
			message = message.mid(endSymPos+1);
		}
	}
}

void onRead(TcpSocket *tcpSocket, ByteArray data)
{
	if(tcpSocket == virtualClient)
	{
		messageRead(data);
	}
	else if (tcpSocket->server() == serverToUser)
	{
		char id[32];
		sprintf(id, "%p", tcpSocket);
		unsigned long long len = data.size();
		if(printMessage)
		{
			cout<<"user "<<id<<" send message."<<endl;
			cout<<data.toString(CODEC_UTF8)<<endl;
		}
		ByteArray sendMessage = "m:"+ByteArray(id)+";"+
			ByteArray::fromString(String::fromNumber(len), CODEC_UTF8)+"#"+data;
		tellToVirtualClient(sendMessage);
#ifdef _WIN32
		Sleep(1);
#else
		usleep(1000);
#endif
	}
	else
	{
#ifdef _WIN32
		cout << "receive message but unknow sender." << endl;
#else
		cout << "be killed." << endl;
		beKilled = true;
#endif
	}
}

bool handleEvent()
{
	pthread_mutex_lock(&messagesMutex);
	unsigned int len = m_messages.size();
	for(unsigned int i=0; i<len; ++i)
	{
		Message m = m_messages[i];
		if(m.type == "m")
		{
			if(printMessage)
			{
				cout<<"real server send message to user "<<m.id<<"."<<endl;
				cout<<m.data.toString(CODEC_UTF8)<<endl;
			}
			pthread_mutex_lock(&usersMutex);
			map<String, TcpSocket *>::iterator it = users.find(m.id);
			TcpSocket *target = NULL; 
			if(it != users.end())
			{
				target = it->second;
			}
			pthread_mutex_unlock(&usersMutex);
			
			if(target)
			{
				target->write(m.data);
			}
#ifdef _WIN32
			Sleep(1);
#else
			usleep(1000);
#endif
		}
		else if(m.type == "a")
		{
			cout<<"virtual client alive."<<endl;
		}
		else if(m.type == "d")
		{
			cout<<"virtual client disconnect from real server."<<endl;
			TcpSocket *temp = NULL;
			pthread_mutex_lock(&usersMutex);
			map<String, TcpSocket *>::iterator it = users.find(m.id);
			if(it != users.end())
			{
				temp = it->second;
				//if(it->second)
				//{
				//	it->second->abort();
				//}
				//users.erase(it);
			}
			pthread_mutex_unlock(&usersMutex);
			if(temp)
			{
				temp->abort();
			}
		}
	}
	m_messages.clear();
	pthread_mutex_unlock(&messagesMutex);
	
#ifdef _WIN32
	return true;
#else
	return !beKilled;
#endif
}

int main(int argc, char *argv[])
{
	for(int i=1; i<argc; ++i)
	{
		if(strcmp(argv[i], "-v")==0 || strcmp(argv[i], "--version")==0)
		{
			cout<<"Proxy server version: 4."<<endl;
			return 0;
		}
		else if(strcmp(argv[i], "-p")==0 || strcmp(argv[i], "--printMessage")==0)
		{
			printMessage = true;
		}
	}
	
	IniSettings config("config.ini", CODEC_UTF8);
	
	unsigned short portForClient = config.value("listen/client", "0").toUInt();
	unsigned short portForUser = config.value("listen/user", "0").toUInt();
	
	cout<<"tunnel server running."<<endl;
	cout<<"port for client: "<<portForClient<<", port for user: "<<portForUser<<"."<<endl;
	
	pthread_mutex_init(&serverStartMutex, NULL);
	pthread_mutex_init(&serverConnectMutex, NULL);
	pthread_mutex_init(&socketDisconnectMutex, NULL);
	pthread_mutex_init(&tellMutex, NULL);
	pthread_mutex_init(&usersMutex, NULL);
	pthread_mutex_init(&messagesMutex, NULL);
	
	serverToClient = new TcpServer();
	
	serverToClient->setNewConnectingCallBack(onNewConnecting);
	serverToClient->setStartSucceedCallBack(onStartSucceed);
	serverToClient->setClosedCallBack(onClosed);
	
	if(serverToClient->start(portForClient))
	{
		//cout<<"virtual server start fail!"<<endl;
		fprintf(stderr, "virtual server start fail!\n");
		delete serverToClient;
		return -1;
	}
	
	serverToUser = new TcpServer();
	
	serverToUser->setNewConnectingCallBack(onNewConnecting);
	serverToUser->setStartSucceedCallBack(onStartSucceed);
	serverToUser->setClosedCallBack(onClosed);
	
	if(serverToUser->start(portForUser))
	{
		//cout<<"proxy server start fail!"<<endl;
		fprintf(stderr, "proxy server start fail!\n");
		delete serverToClient;
		delete serverToUser;
		return -1;
	}

#ifndef _WIN32
	unsigned short portForManager = config.value("listen/manager", "0").toUInt();

	manager = new TcpServer();
	
	manager->setNewConnectingCallBack(onNewConnecting);
	manager->setStartSucceedCallBack(onStartSucceed);
	manager->setClosedCallBack(onClosed);
	
	if(manager->start(portForManager))
	{
		//cout << "proxy manager start fail!" << endl;
		fprintf(stderr, "proxy manager start fail!\n");
		delete serverToClient;
		delete serverToUser;
		delete manager;
		return -1;
	}
#endif
	
	while(handleEvent())
	{
#ifdef _WIN32
		Sleep(1);
#else
		usleep(1000);
#endif
	}
	
	delete serverToClient;
	delete serverToUser;
#ifndef _WIN32
	delete manager;
#endif
	
	return 0;
}
