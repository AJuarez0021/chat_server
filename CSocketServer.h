#ifndef __CSOCKET_SERVER_H
#define __CSOCKET_SERVER_H

#include <winsock2.h>
#include <string>
#include <cassert>

using namespace std;


class CSocketServer
{
public:
 CSocketServer(unsigned short uPort=1111,const string& strIP="127.0.0.1"){  
  m_uPort=uPort;
  m_strIP=strIP;   
  m_Socket=NULL;
 }
 
 ~CSocketServer(){
 
 }
 bool Connect(){
  WSADATA m_wsaData;
  int iResult =WSAStartup(MAKEWORD(2,2),&m_wsaData);
  if (iResult != NO_ERROR)
   return false;
  m_Socket=CreateSocket(); 
  if (m_Socket == INVALID_SOCKET) {
   WSACleanup();
   return false;
  }
  m_info.sin_addr.s_addr=inet_addr(m_strIP.c_str());	   	   	   
  m_info.sin_family=AF_INET;
  m_info.sin_port=htons(m_uPort);
  return true;
 }


 unsigned short GetPort(){
  return m_uPort;
 }

 string GetIP(){
  return m_strIP;
 }

 void SetIP(const string& strIP){
  m_strIP=strIP;
 }
 void Set_Port(unsigned short uPort){
  m_uPort=uPort;
 }
 
 bool Bind(){
  if(bind(m_Socket,(SOCKADDR *)&m_info,sizeof(m_info))==SOCKET_ERROR)
	return false;

  return true;
 }

 bool Select(HWND hWnd,UINT uMsg){
  if(WSAAsyncSelect(m_Socket, hWnd,uMsg, FD_CLOSE | FD_READ | FD_ACCEPT) == SOCKET_ERROR)
   return false;

  return true;
 }
 
 bool Listen(int iBackLog){
  if(listen(m_Socket,iBackLog)==SOCKET_ERROR)
   return false;

  return true;
 }
 SOCKET CreateSocket(int af=AF_INET,int iType=SOCK_STREAM,int iProtocol=IPPROTO_TCP){
  return socket(af,iType,iProtocol);
 }

 bool Send(SOCKET s,const string& strText)
 {
  if(send(s,strText.c_str(),strText.length()+1,0)==SOCKET_ERROR)
   return false;

  return true;
 }

  bool Send(const string& strText)
 {
  if(send(m_Socket,strText.c_str(),strText.length()+1,0)==SOCKET_ERROR)
   return false;

  return true;
 }
 string Recv(SOCKET s){
  string strRes;
  strRes=recv(s,m_strBuffer,300,0)==SOCKET_ERROR ? "Error al leer datos" : m_strBuffer;
  return strRes;  
 }
 string Recv(){
  string strRes;
  strRes=recv(m_Socket,m_strBuffer,300,0)==SOCKET_ERROR ? "Error al leer datos" : m_strBuffer;
  return strRes;  
 }
 SOCKET Accept(){
  return accept(m_Socket,(SOCKADDR *)&m_info,0);
 }
 void Accept2(){
   m_Socket=accept(m_Socket,(SOCKADDR *)&m_info,0);
 }
 void Disconnect(){
  closesocket(m_Socket);
  WSACleanup( ); 
 }
 void CloseSocket(SOCKET s)
 {
  closesocket(s);
   
 }
private:
 sockaddr_in m_info;
 unsigned short m_uPort;
 SOCKET m_Socket;
 string m_strIP;
 char m_strBuffer[300];
};

#endif