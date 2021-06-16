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

using namespace std;

TcpSocket *vSocket = NULL;

map<String, TcpSocket *> users;
map<TcpSocket *, String> users_;
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

String rHost;
unsigned short rPort;

String vHost;
unsigned short vPort;

unsigned int connectToRealServerTimeout;
unsigned int heart;
unsigned long long heartRest = 0;

bool printMessage = false;

pthread_mutex_t tellMutex;
void tellToVirtualServer(ByteArray &b)
{
	if(vSocket)
	{
		pthread_mutex_lock(&tellMutex);
		if(!(vSocket->write(b)))
		{
			cout<<"tell to virtual server fail!"<<endl;
		}
		pthread_mutex_unlock(&tellMutex);
	}
}

pthread_mutex_t disconnectMutex;
void onDisconnected(TcpSocket *tcpSocket)
{
	pthread_mutex_lock(&disconnectMutex);
	if(tcpSocket == vSocket)
	{
		cout<<"proxy server disconnected."<<endl;
		
		// kill all virtual user
		pthread_mutex_lock(&usersMutex);
		for(map<String, TcpSocket *>::iterator it = users.begin();
			it != users.end();)
		{
			it->second->setDisconnectedCallBack(NULL);
			TcpSocket *temp = it->second;
			users_.erase(users_.find(it->second));
			users.erase(it++);
			delete temp;
		}
		pthread_mutex_unlock(&usersMutex);
		
		cout<<"ready to reconnect proxy server..."<<endl;
		vSocket->connectToHost(vHost, vPort);
	}
	else
	{
		cout<<"one virtual user disconnect from real server!"<<endl;
		pthread_mutex_lock(&usersMutex);
		map<TcpSocket *, String>::iterator it = users_.find(tcpSocket);
		if(it != users_.end())
		{
			ByteArray message = "d:"+
				ByteArray::fromString(it->second, CODEC_UTF8)+"#";
			tellToVirtualServer(message);
			users.erase(users.find(it->second));
			users_.erase(it);
		}
		delete tcpSocket;
		pthread_mutex_unlock(&usersMutex);
	}
	pthread_mutex_unlock(&disconnectMutex);
}

pthread_mutex_t connectMutex;
void onConnected(TcpSocket *tcpSocket)
{
	pthread_mutex_lock(&connectMutex);
	if(tcpSocket == vSocket)
	{
		cout<<"proxy server connected."<<endl;
	}
	else
	{
		cout<<"one virtual user connect to real server succeed."<<endl;
	}
	pthread_mutex_unlock(&connectMutex);
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
	if(tcpSocket == vSocket)
	{
		messageRead(data);
	}
	else
	{
		pthread_mutex_lock(&usersMutex);
		map<TcpSocket *, String>::iterator it = users_.find(tcpSocket);
		if(it != users_.end())
		{
			String id = it->second;
			unsigned long long len = data.size();
			if(printMessage)
			{
				cout<<"real server send message to user "<<id<<"."<<endl;
				cout<<data.toString(CODEC_UTF8)<<endl;
			}
			ByteArray sendMessage = "m:"+ByteArray::fromString(id, CODEC_UTF8)+
				";"+ByteArray::fromString(String::fromNumber(len), CODEC_UTF8)+
				"#"+data;
			tellToVirtualServer(sendMessage);
		}
		pthread_mutex_unlock(&usersMutex);
#ifdef _WIN32
		Sleep(1);
#else
		usleep(1000);
#endif
	}
}

void onConnectError(TcpSocket *tcpSocket, int errorStatus)
{
	if(tcpSocket == vSocket)
	{
		cout<<"connect to proxy server fail. ready to reconnect..."<<endl;
#ifdef _WIN32
		Sleep(500);
#else
		usleep(500000);
#endif
		vSocket->connectToHost(vHost, vPort);
	}
}

void handleEvent()
{
	pthread_mutex_lock(&messagesMutex);
	unsigned int len = m_messages.size();
	for(unsigned int i=0; i<len; ++i)
	{
		Message m = m_messages[i];
		pthread_mutex_lock(&usersMutex);
		if(m.type == "c")
		{
			cout<<"new user "<<m.id<<" connected."<<endl;
			TcpSocket *target = new TcpSocket();
			target->setDisconnectedCallBack(onDisconnected);
			target->setConnectedCallBack(onConnected);
			target->setReadCallBack(onRead);
			if(target->connectToHost(rHost, rPort))
			{
				cout<<"can not connect to real server!"<<endl;
				
				ByteArray sendMessage = "d:"+
					ByteArray::fromString(m.id, CODEC_UTF8)+"#";
				tellToVirtualServer(sendMessage);
				delete target;
			}
			else
			{
				unsigned int timeout = connectToRealServerTimeout;
				while(target->connectStatus()==TCP_SOCKET_DISCONNECTED && timeout)
				{
					--timeout;
#ifdef _WIN32
					Sleep(1);
#else
					usleep(1000);
#endif
				}
				if(!timeout)
				{
					cout<<"connect to real server timeout!"<<endl;
					
					ByteArray sendMessage = "d:"+
						ByteArray::fromString(m.id, CODEC_UTF8)+"#";
					tellToVirtualServer(sendMessage);
					delete target;
				}
				else
				{
					users[m.id] = target;
					users_[target] = m.id;
				}
			}
		}
		else if(m.type == "d")
		{
			cout<<"user "<<m.id<<" disconnected."<<endl;
			map<String, TcpSocket *>::iterator it = users.find(m.id);
			if(it != users.end())
			{
				if(it->second)
				{
					users_.erase(users_.find(it->second));
					it->second->setDisconnectedCallBack(NULL);
					delete it->second;
				}
				users.erase(it);
			}
		}
		else if(m.type == "m")
		{
			if(printMessage)
			{
				cout<<"user "<<m.id<<" send message."<<endl;
				cout<<m.data.toString(CODEC_UTF8)<<endl;
			}
			map<String, TcpSocket *>::iterator it = users.find(m.id);
			if(it != users.end())
			{
				TcpSocket *target = it->second;
				if(target)
				{
					target->write(m.data);
#ifdef _WIN32
					Sleep(1);
#else
					usleep(1000);
#endif
				}
			}
		}
		pthread_mutex_unlock(&usersMutex);
	}
	m_messages.clear();
	pthread_mutex_unlock(&messagesMutex);
	
	if(vSocket->connectStatus() == TCP_SOCKET_DISCONNECTED)
	{
#ifdef _WIN32
		heartRest = heart*100;
#else
		heartRest = heart*1000;
#endif
	}
	else
	{
		if(heartRest)
		{
			--heartRest;
		}
		else
		{
			ByteArray sendMessage = "a#";
			tellToVirtualServer(sendMessage);
#ifdef _WIN32
			heartRest = heart*100;
#else
			heartRest = heart*1000;
#endif
		}
	}
}

int main(int argc, char *argv[])
{
	for(int i=1; i<argc; ++i)
	{
		if(strcmp(argv[i], "-v")==0 || strcmp(argv[i], "--version")==0)
		{
			cout<<"Proxy client version: 1."<<endl;
			return 0;
		}
		else if(strcmp(argv[i], "-p")==0 || strcmp(argv[i], "--printMessage")==0)
		{
			printMessage = true;
		}
	}
	
	IniSettings config("config.ini", CODEC_UTF8);
	
	rHost = config.value("real/host", "0.0.0.0");
	rPort = config.value("real/port", "0").toUInt();
	
	vHost = config.value("virtual/host", "0.0.0.0");
	vPort = config.value("virtual/port", "0").toUInt();
	
	connectToRealServerTimeout = config.value("real/connectTimeout", "3000").toUInt();
	heart = config.value("virtual/heart", "10").toUInt();
	
	cout<<"tunnel client running."<<endl;
	cout<<"real server: (ip: "<<rHost<<", port: "<<rPort<<")."<<endl;
	cout<<"proxy server: (ip: "<<vHost<<", port: "<<vPort<<")."<<endl;
	cout<<"connect to real server timeout: "<<connectToRealServerTimeout<<" msec."<<endl;
	cout<<"heart per "<<heart<<" sec."<<endl;
	
	pthread_mutex_init(&tellMutex, NULL);
	pthread_mutex_init(&disconnectMutex, NULL); 
	pthread_mutex_init(&connectMutex, NULL);
	pthread_mutex_init(&messagesMutex, NULL);
	pthread_mutex_init(&usersMutex, NULL);
	
	vSocket = new TcpSocket();
	
	vSocket->setDisconnectedCallBack(onDisconnected);
	vSocket->setConnectedCallBack(onConnected);
	vSocket->setReadCallBack(onRead);
	vSocket->setConnectErrorCallBack(onConnectError);
	
	if(vSocket->connectToHost(vHost, vPort))
	{
		cout<<"connect to proxy server fail!"<<endl;
		return 0;
	}
	
	while(true)
	{
		handleEvent();
#ifdef _WIN32
		Sleep(1);
#else
		usleep(1000);
#endif
	}
	
	return 0;
}
