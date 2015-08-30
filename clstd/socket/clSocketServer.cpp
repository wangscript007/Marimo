#include "clstd.h"
#include "clString.H"
#include "thread/clThread.h"
#include "clSocket.h"
#include "clSocketServer.h"

#define SOCKET_ERROR_LOG(_STAT, _MSG) if (_STAT == SOCKET_ERROR) { CLOG_ERROR(_MSG); }
#define MAX_RECV_BUF 4096

#pragma comment(lib, "Ws2_32.lib")

namespace clstd
{
  TCPServer::TCPServer()
    : m_ServerSocket (0)
  {
  }
  
  TCPServer::~TCPServer()
  {
  }
  
  SocketResult TCPServer::OpenPort(CLUSHORT port)
  {
  	WSADATA		Data;
  	SOCKADDR_IN serverSockAddr;
  	
  //	
  //
 
  	int status = WSAStartup(CLMAKEWORD(1, 1), &Data);
  	if (status != 0) {
  		CLOG_ERROR("ERROR: WSAStartup unsuccessful\r\n");
    }
  
  	// zero the sockaddr_in structure
  	memset(&serverSockAddr, 0, sizeof(serverSockAddr));
  
  	// specify the port portion of the address
  	serverSockAddr.sin_port = htons(port);
  	// specify the address family as Internet
  	serverSockAddr.sin_family = AF_INET;
  	// specify that the address does not matter
  	serverSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  
  	// create a socket  socket(ͨ�ŷ���������,�׽��ֵ�����,�׽���ʹ�õ��ض�Э��)
  	m_ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
  	if (m_ServerSocket == INVALID_SOCKET) {
      _ChkWSACleanup(status);
  		CLOG_ERROR("ERROR: socket unsuccessful\r\n");
      return SocketResult_CreateFailed;
    }
  
  	// associate the socket with the address
  	status = bind(m_ServerSocket, (LPSOCKADDR)&serverSockAddr, sizeof(serverSockAddr));
    if (status == SOCKET_ERROR) { 
      _ChkWSACleanup(status);
      CLOG_ERROR("ERROR: bind unsuccessful\r\n"); 
      return SocketResult_CanotBind;
    }
  
  //FD_ISSET
  	return SocketResult_Ok;
  }
  
  int TCPServer::Close(u32 nMilliSec)
  {
    int status = 0;
    if(m_ServerSocket)
    {
      CLOG("Close server socket...\r\n");
      status = closesocket(m_ServerSocket);
      SOCKET_ERROR_LOG(status, "Error for closing server socket...\r\n");

      // �ȴ��߳��˳�
      if(nMilliSec != 0) {
        Wait(nMilliSec);
      }

      m_ServerSocket = 0;
      _ChkWSACleanup(status);
    }
    return status;
  }

  i32 TCPServer::Send(SOCKET sock, CLLPCVOID pData, u32 nLen)
  {
    i32 nSent = 0;
    do {
      const int result = send(sock, (const char*)pData + nSent, nLen - nSent, 0);
      if(result < 0) {
        return result;
      }
      nSent += result;
    } while(nSent < (i32)nLen);
    return nSent;
  }

  i32 TCPServer::Recv(SOCKET sock, CLLPVOID pData, u32 nLen)
  {
    return recv(sock, (char*)pData, nLen, 0);
  }
  
  i32 TCPServer::Run()
  {
    int status;

    // allow the socket to take connections listen(�ѽ�������δ���ӵ��׽��ֺ�,���Ӷ��е���󳤶�)
    status = listen(m_ServerSocket, 1);
    if(status == SOCKET_ERROR)
    {
      CLOG_ERROR("ERROR: listen unsuccessful\r\n");
      return (i32)SocketResult_CanotListen;
    }

    CLOG("Waiting for connection...\r\n");

    MainLoop();
    return 0;
  }

  int TCPServer::MainLoop()
  {
    fd_set ReadSet;
    fd_set ExceptSet;
  	int result = 0;

  	while(1)
  	{
      FD_ZERO(&ReadSet);
      FD_SET(m_ServerSocket, &ReadSet);

      FD_ZERO(&ExceptSet);
      FD_SET(m_ServerSocket, &ExceptSet);

      ASSERT(m_ClientList.size() < FD_SETSIZE - 1); // ServerSocket Ҫռ��һ��
      for(SocketList::iterator it = m_ClientList.begin();
        it != m_ClientList.end(); ++it)
      {
        ASSERT(*it != NULL);
        FD_SET(*it, &ReadSet);
        FD_SET(*it, &ExceptSet);
      }

      result = select(0, &ReadSet, 0, &ExceptSet, 0);
      ASSERT(ExceptSet.fd_count == 0);

      if(result == 0)
      {
        // Time Out
      }
      else if(result == SOCKET_ERROR)
      {
        if(FD_ISSET(m_ServerSocket, &ReadSet))
        {
          break;
        }
        
        CLBREAK;
      }
      else if(result != 0)
      {
        if(FD_ISSET(m_ServerSocket, &ReadSet))
        {
          SOCKADDR_IN clientSockAddr;
          int addrLen = sizeof(SOCKADDR_IN);

          // accept the connection request when one is received
          SOCKET client = accept(m_ServerSocket, (LPSOCKADDR)&clientSockAddr, &addrLen);
          if(m_ClientList.size() < FD_SETSIZE - 1) {
            CLOG("Got the connection(%d)...\r\n", client);
            OnEvent(client, SE_ACCEPT);
            m_ClientList.push_back(client);
          }
          else {
            closesocket(client);
          }
        }
        
        for(SocketList::iterator it = m_ClientList.begin();
          it != m_ClientList.end();)
        {
          if(FD_ISSET(*it, &ReadSet))
          {
            u32 dwPeek;
            result = recv(*it, (char*)&dwPeek, sizeof(u32), MSG_PEEK);

            if(result == 0) // �˿��Ѿ��ر�
            {
              // ���ַ�ʽ��
              // ����ͻ�����Debug�³��ֶϵ㲢�رգ�������޷��յ�close��Ϣ
              OnEvent(*it, SE_CLOSE);

              result = closesocket(*it);
              CLOG("Close socket(%d).\r\n", *it);

              it = m_ClientList.erase(it);
              continue;
            }
            else {
              OnEvent(*it, SE_READ);
            }
          }
          ++it;
        } // for
      }
  	}

    // �˳�ʱ�����ͻ��˶˿�
    for(SocketList::iterator it = m_ClientList.begin();
      it != m_ClientList.end(); ++it)
    {
      result = closesocket(*it);
      if(result == SOCKET_ERROR) {
        CLOG_ERROR("Error for closing socket(%d)...\r\n", *it);
      }
    }
    m_ClientList.clear();

  	return result;
  }

  //void TCPServer::OnEvent(SOCKET sock, SocketEvent eEvent)
  //{
  //}
  //
  //////////////////////////////////////////////////////////////////////////
  UDPSocket::UDPSocket()
    : m_Socket(0)
  {
  }

  UDPSocket::~UDPSocket()
  {
  }

  SocketResult UDPSocket::OpenPort(u32 dwFlags, u32 port)
  {
    WSADATA		Data;
    m_dwFlags = dwFlags;

    int status = WSAStartup(CLMAKEWORD(1, 1), &Data);
    if (status != 0) {
      CLOG_ERROR("ERROR: WSAStartup unsuccessful\r\n");
    }

    m_Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_Socket == INVALID_SOCKET) {
      _ChkWSACleanup(status);
      CLOG_ERROR("ERROR: socket unsuccessful\r\n");
      return SocketResult_CreateFailed;
    }

    if(TEST_FLAG(m_dwFlags, PM_Broadcast))
    {
      char bOpt = true;
      status = setsockopt(m_Socket, SOL_SOCKET, SO_BROADCAST, &bOpt, sizeof(bOpt));   
    }

    if(TEST_FLAG(dwFlags, PM_Recv))
    {
      SOCKADDR_IN RecvSockAddr;
      InlSetZeroT(RecvSockAddr);

      RecvSockAddr.sin_port = htons(port);
      RecvSockAddr.sin_family = AF_INET;
      RecvSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

      status = bind(m_Socket, (LPSOCKADDR)&RecvSockAddr, sizeof(RecvSockAddr));
      if (status == SOCKET_ERROR) { 
        _ChkWSACleanup(status);
        CLOG_ERROR("ERROR: bind unsuccessful\r\n"); 
        return SocketResult_CanotBind;
      }
    }
    return SocketResult_Ok;
  }

  int UDPSocket::Close(u32 nMilliSec)
  {
    int status = 0;

    if(m_Socket)
    {
      CLOG("Close UDP socket...\r\n");
      status = closesocket(m_Socket);
      SOCKET_ERROR_LOG(status, "Error for closing UDP socket...\r\n");
    

      // �ȴ��߳��˳�
      if(nMilliSec != 0) {
        Wait(nMilliSec);
      }

      m_Socket = NULL; // �˳�ǰ�߳̿��ܻ���ҪSocket

      _ChkWSACleanup(status);
    }
    return status;
  }

  i32 UDPSocket::Send(CLLPCSTR szIPAddress, u32 wPort, CLLPCVOID pData, u32 nLen)
  {
    i32 result = Send(inet_addr(szIPAddress), wPort, pData, nLen);
    return result;
  }

  i32 UDPSocket::Send(u32_ptr uIPAddress, u32 wPort, CLLPCVOID pData, u32 nLen)
  {
    if(TEST_FLAG_NOT(m_dwFlags, PM_Send)) {
      return -1;
    }

    sockaddr_in RecvAddr;
    RecvAddr.sin_family = AF_INET;
    RecvAddr.sin_port = htons((u_short)wPort);
    RecvAddr.sin_addr.s_addr = (unsigned long)uIPAddress;

    int result = sendto(m_Socket, (const char*)pData, nLen, 0, (SOCKADDR*) &RecvAddr, sizeof(RecvAddr));
    return result;
  }

  i32 UDPSocket::Recv(CLLPCVOID pData, u32 nLen, u32_ptr* uIPAddress, u32* wPort)
  {
    if(TEST_FLAG_NOT(m_dwFlags, PM_Recv)) {
      return -1;
    }
    sockaddr_in SenderAddr;
    int SenderAddrSize = sizeof(SenderAddr);
    int result = recvfrom(m_Socket, (char*)pData, nLen, 0, (SOCKADDR *)&SenderAddr, &SenderAddrSize);

    if(uIPAddress) {
      *uIPAddress = *(u32_ptr*)&SenderAddr.sin_addr;
    }

    if(wPort) {
      *wPort = SenderAddr.sin_port;
    }
    return result;
  }

  i32 UDPSocket::Run()
  {
    if(TEST_FLAG(m_dwFlags, PM_Recv)) {
      MainLoop();
    }
    return 0;
  }

  int UDPSocket::MainLoop()
  {
    fd_set ReadSet;
    int result = 0;

    while(1)
    {
      FD_ZERO(&ReadSet);
      FD_SET(m_Socket, &ReadSet);

      result = select(0, &ReadSet, 0, 0, 0);
      if(result == 0)
      {
        // Time Out
      }
      else if(result == SOCKET_ERROR)
      {
        break;
      }
      else if(result != 0)
      {
        OnEvent(m_Socket, SE_READ);
      }
    }
    return result;
  }


  //////////////////////////////////////////////////////////////////////////
} // namespace clstd