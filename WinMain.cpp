//Chat-Servidor
//Permite los siguientes comandos:
//shutdown  --> Apagar PC
//reboot    --> Reiniciar PC
//show tray --> Visualizar barra de tareas
//hide tray --> Ocultar barra de tareas
//Programa compilado con Visual C++ 6.0

#include "CSocketServer.h"
#include "resource.h"
#include "hyperlink.h"


#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"ws2_32.lib")


LRESULT CALLBACK DlgProc(HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK ConnectProc(HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK AboutProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
void AddText(HWND hWnd,UINT nID,string strText);
void SetDlgIcon(UINT nIDIcon,HWND hWnd);
void CenterDlg(HWND hWnd);
void ExecuteCommand(string strText);
void SetText(HWND hWnd,UINT nID,string strText);
void SaveText(HWND hWnd,UINT nID,LPCTSTR strFileOut,short iMax);
BOOL Save(HWND hWnd, LPSTR lpszFiltro, LPSTR lpszTitulo);
BOOL Open(HWND hWnd,LPSTR lpszFilter, LPSTR lpszTitle);
int GetMax(HWND hWnd,UINT nID);
string GetIP();

CSocketServer s;
string g_strName;
string g_strIP;

bool bConnect;
TCHAR szBufferOut[MAX_PATH];
TCHAR szBufferIn[MAX_PATH];

#ifndef SHTDN_REASON_MAJOR_OPERATINGSYSTEM 
#define SHTDN_REASON_MAJOR_OPERATINGSYSTEM 0x00020000
#endif

#ifndef SHTDN_REASON_FLAG_PLANNED
#define SHTDN_REASON_FLAG_PLANNED 0x80000000
#endif

#ifndef SHTDN_REASON_MINOR_UPGRADE  
#define SHTDN_REASON_MINOR_UPGRADE 0x00000003
#endif

#define SHUTDOWN    1
#define REBOOT      0
#define IDM_ABOUT 100

int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hPrevInst,LPSTR lpCmdLine,int nShowCmd)
{

 return DialogBox(hInst,MAKEINTRESOURCE(IDD_MAIN),NULL,(DLGPROC)DlgProc);
} 


LRESULT CALLBACK ConnectProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
 static char *strIP=NULL;
 static char *strName=NULL;
 switch(uMsg){
  case WM_INITDIALOG:	   	
	   
	   SetText(hWnd,IDC_EDIT_HOST,GetIP());
	   SetText(hWnd,IDC_EDIT_PORT,"1111");
	   return 0;
  case WM_COMMAND:
	   switch(LOWORD(wParam)){
	     case ID_OK:
			 {
			  BOOL bResultPort;
			  UINT uResultIP,uResultName;
              unsigned short uPort;
			  int iLength;
              
			  iLength=(int)SendDlgItemMessage(hWnd,IDC_EDIT_HOST,WM_GETTEXTLENGTH,0,0);
              strIP=new char[iLength+1];			  
			  if(!strIP){
	        	MessageBox(hWnd,TEXT("Error no hay memoria suficiente"),TEXT("Servidor"),MB_ICONERROR);
		        EndDialog(hWnd,1);
			  }	
			  uResultIP=GetDlgItemText(hWnd,IDC_EDIT_HOST,strIP,iLength+1);		
			  uPort=(unsigned short)GetDlgItemInt(hWnd,IDC_EDIT_PORT,&bResultPort,FALSE);		
			  iLength=(int)SendDlgItemMessage(hWnd,IDC_EDIT_NAME,WM_GETTEXTLENGTH,0,0);		
              strName=new char[iLength+1];		
			  if(!strName){
                MessageBox(hWnd,TEXT("Error no hay memoria suficiente"),TEXT("Servidor"),MB_ICONERROR);
		        EndDialog(hWnd,1);
			  }
			  
              uResultName=GetDlgItemText(hWnd,IDC_EDIT_NAME,strName,iLength+1);			 		
			  if(bResultPort && uResultIP && uResultName){
			   s.SetIP(strIP);
               s.Set_Port(uPort);
			   g_strName=strName;
			   g_strIP=strIP;			   
			   bConnect=true;
			   EndDialog(hWnd,0);
			   
			  }			
			  else
			   MessageBox(hWnd,TEXT("Error: Necesita introducir el nombre, puerto y el host"),TEXT("Servidor"),MB_ICONERROR);
			 }
			  break;
		 case ID_CANCEL:		
			  EnableWindow(GetDlgItem(hWnd,ID_CONECTAR),TRUE);
	          EnableWindow(GetDlgItem(hWnd,ID_DESCONECTAR),FALSE);
	          EnableWindow(GetDlgItem(hWnd,ID_SEND),FALSE);
	          EnableWindow(GetDlgItem(hWnd,ID_CLEAR),FALSE);
			  EnableWindow(GetDlgItem(hWnd,ID_SAVE),FALSE);
			  bConnect=false;
			  EndDialog(hWnd,0);
			  break;
	   }
	   break;
  case WM_CLOSE:
  case WM_DESTROY:	 
	   if(strIP!=0){
		delete [] strIP;

		strIP=NULL;
	   }
	   if(strName!=0){
		delete [] strName;

	    strName=NULL;
	   }
	   EndDialog(hWnd,0);
	   return 0;
  default:
	   return 0;
 }
 return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}


LRESULT CALLBACK DlgProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
 
 static string strNick;
 static bool bFlag=true;
 static bool bDisconnect=false;
 HMENU hMenu;
 switch(uMsg){
  case WM_INITDIALOG:
	   SetDlgIcon(IDI_ICON1,hWnd);
	   
	   CenterDlg(hWnd);	   
       EnableWindow(GetDlgItem(hWnd,ID_CONECTAR),TRUE);
	   EnableWindow(GetDlgItem(hWnd,ID_DESCONECTAR),FALSE);
	   EnableWindow(GetDlgItem(hWnd,ID_SEND),FALSE);
	   EnableWindow(GetDlgItem(hWnd,ID_CLEAR),FALSE);
	   EnableWindow(GetDlgItem(hWnd,ID_SAVE),FALSE);
	   SetText(hWnd,IDC_STATIC_STATUS,"Sin conexion");		   	   	  
	   hMenu = GetSystemMenu (hWnd, FALSE);	   
	   AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
       AppendMenu(hMenu, MF_STRING, IDM_ABOUT, "Acerca de...");
	   return 0;
   case WM_SYSCOMMAND:
        switch(LOWORD(wParam)){
		  
		  case IDM_ABOUT:       
			   DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_ABOUT),hWnd,(DLGPROC)AboutProc);
			   break;
 
		}
		break;
  case WM_COMMAND:
	   switch(LOWORD(wParam)){
	     case ID_CONECTAR:
			  DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_INFORMATION),hWnd,(DLGPROC)ConnectProc);			 
			  if(bConnect){				
				if(s.Connect()){
			     s.Bind();
			     s.Select(hWnd,WM_USER);			  
			     s.Listen(10);			   
			     EnableWindow(GetDlgItem(hWnd,ID_DESCONECTAR),TRUE);			    
			     EnableWindow(GetDlgItem(hWnd,ID_CONECTAR),FALSE);				
				 EnableWindow(GetDlgItem(hWnd,ID_SAVE),TRUE);
			     SetText(hWnd,IDC_STATIC_STATUS,"Conectado");	
			     AddText(hWnd,IDC_LIST,"Bienvenido " + g_strName + "!");	
				 AddText(hWnd,IDC_LIST,"Conectado con IP: " + g_strIP);	
				}
			
			  }
			  break;
		 case ID_DESCONECTAR:					  
			  s.Disconnect();		
			  EnableWindow(GetDlgItem(hWnd,ID_CONECTAR),TRUE);
			  EnableWindow(GetDlgItem(hWnd,ID_SEND),FALSE);
			  EnableWindow(GetDlgItem(hWnd,ID_CLEAR),FALSE);
              EnableWindow(GetDlgItem(hWnd,ID_DESCONECTAR),FALSE);
			  EnableWindow(GetDlgItem(hWnd,ID_SAVE),TRUE);
			  SetText(hWnd,IDC_STATIC_STATUS,"Desconectado");		
			  bDisconnect=true;
			  break;
		 case ID_CLEAR:
			  SetText(hWnd,IDC_EDIT_TEXT,"");
			  SetFocus(GetDlgItem(hWnd,IDC_EDIT_TEXT));
			  break;
		 case ID_SAVE:
			  if(Save(hWnd,"Archivos *.txt\0*.txt\0","Guardar Conversacion")){
			   int iMax=0;
	           iMax=GetMax(hWnd,IDC_LIST);	   
	           if(iMax!=LB_ERR)	
	            SaveText(hWnd,IDC_LIST,szBufferOut,iMax);
			  }
			  break;
		 case ID_SEND:
			 {
              int iLength;
			  char *strText=NULL;
			  string str;
               
			  iLength=(int)SendDlgItemMessage(hWnd,IDC_EDIT_TEXT,WM_GETTEXTLENGTH,0,0);
			  strText=new char[iLength+1];
			  if(!strText){
			   MessageBox(hWnd,"Error no hay memoria suficiente","Servidor",MB_ICONERROR);
		       EndDialog(hWnd,1);
			  }
			  GetDlgItemText(hWnd,IDC_EDIT_TEXT,strText,iLength+1);              			 
			  s.Send(strText);
			  str="";
			  str+=g_strName;
			  str+="-> "; 
			  str+=strText;
              AddText(hWnd,IDC_LIST,str);
			  SetText(hWnd,IDC_EDIT_TEXT,"");
			  SetFocus(GetDlgItem(hWnd,IDC_EDIT_TEXT));
			  if(strText!=0){
			   delete [] strText;

			   strText=NULL;
			  }
			 
			  
			 }
			  break;
	   }
	   break;
  case WM_USER:		
	   switch(WSAGETSELECTEVENT(lParam)){
	     case FD_READ:
			 {
			  if(bFlag){
                bFlag=false;
                strNick=s.Recv();
			  }
              else{
			   string strText;	
			   string strRecv=s.Recv();
			   strText="";
			   strText+=strNick; 
			   strText+="-> ";
			   strText+=strRecv.c_str();
			   ExecuteCommand(strRecv);
			   AddText(hWnd,IDC_LIST,strText);
			  }
			 }
			  break;
		 case FD_ACCEPT:
			 {			  
			  s.Accept2();
			  SetText(hWnd,IDC_STATIC_STATUS,"Servidor Conectado - Cliente conectado");
			  EnableWindow(GetDlgItem(hWnd,ID_SEND),TRUE);						  
			  AddText(hWnd,IDC_LIST,"Cliente conectado con IP: " + s.GetIP());
			  string str="";			   
			  str+=g_strName;             	
			  s.Send(str);
			  bFlag=true;
			  EnableWindow(GetDlgItem(hWnd,ID_SEND),TRUE);
			  EnableWindow(GetDlgItem(hWnd,ID_CLEAR),TRUE);

			 }
			  break;
		 case FD_CLOSE:		
			  EnableWindow(GetDlgItem(hWnd,ID_CONECTAR),TRUE);
			  EnableWindow(GetDlgItem(hWnd,ID_SEND),FALSE);	
			  EnableWindow(GetDlgItem(hWnd,ID_CLEAR),TRUE);			   
              EnableWindow(GetDlgItem(hWnd,ID_DESCONECTAR),FALSE);
			  EnableWindow(GetDlgItem(hWnd,ID_SAVE),TRUE);			 
			  AddText(hWnd,IDC_LIST,strNick + "-> Se ha cerrado la conexion");
			  SetText(hWnd,IDC_STATIC_STATUS,"Se ha cerrado la conexion");
			  s.Disconnect();
			  break;
	   }
	   break;
  case WM_CLOSE:	              				
  case WM_DESTROY:   
	  if(!bDisconnect){
		if(bConnect){		 	
		 s.Disconnect();
		}
	  }
	   EndDialog(hWnd,0);	   
	   PostQuitMessage(0);
	   return 0;
  default: 
	   return 0;
 }
 return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}



bool ExitWin32(int iFlag)
{
    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx( &osvi );
    if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT){        
        HANDLE hToken;
        TOKEN_PRIVILEGES tkp;

        if (!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
            return false;

        LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

        tkp.PrivilegeCount = 1;
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

        if (GetLastError() != ERROR_SUCCESS)
            return false;
    }

   if(iFlag==SHUTDOWN){
	if (!ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE,SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_UPGRADE | SHTDN_REASON_FLAG_PLANNED))           
      return false; 
   }
   else if(iFlag==REBOOT){
     if (!ExitWindowsEx(EWX_REBOOT | EWX_FORCE, SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_UPGRADE | SHTDN_REASON_FLAG_PLANNED))           
      return false; 
   }
   else
	 return false;

   return true;
}


void ShowBar(bool bFlag)
{
 HWND hShellTrayWnd = FindWindow(TEXT("Shell_TrayWnd"), NULL);
 if(bFlag)
  ShowWindow(hShellTrayWnd,SW_SHOW);
 else
  ShowWindow(hShellTrayWnd,SW_HIDE);
}

void ExecuteCommand(string strText)
{
 if(strText=="reboot")
  ExitWin32(REBOOT);
 if(strText=="shutdown")
  ExitWin32(SHUTDOWN);
 if(strText=="hide tray")
  ShowBar(false);
 if(strText=="show tray")
  ShowBar(true);
}


string GetIP()
{

      WORD wVersionRequested;
      WSADATA wsaData;
      char Host[255];
      string ip="127.0.0.1";
      PHOSTENT hostinfo;
      wVersionRequested = MAKEWORD( 2, 0 );

      if (WSAStartup( wVersionRequested, &wsaData ) == 0 ){
        if(gethostname (Host, sizeof(Host)) == 0){
           if((hostinfo = gethostbyname(Host)) != NULL){
              ip = inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list);						
            }
         }
         WSACleanup( );
      }
	  

	  return ip;

}

void SetText(HWND hWnd,UINT nID,string strText)
{
 SetDlgItemText(hWnd,nID,strText.c_str());
}

void AddText(HWND hWnd,UINT nID,string strText)
{
 int index=SendDlgItemMessage(hWnd,nID,LB_ADDSTRING,0,(LPARAM)strText.c_str());
 SendDlgItemMessage(hWnd, nID, LB_SETITEMDATA, (WPARAM)index, (LPARAM)strText.length());
}

BOOL Save(HWND hWnd, LPSTR lpszFiltro, LPSTR lpszTitulo)
{
 OPENFILENAME  ofn; 
 ZeroMemory(&ofn, sizeof(OPENFILENAME));
 ofn.lStructSize = sizeof(OPENFILENAME);
 ofn.hwndOwner = hWnd;
 ofn.lpstrFilter = (lpszFiltro);
 ofn.lpstrFile = szBufferOut;
 ofn.nMaxFile = MAX_PATH;
 ofn.lpstrFileTitle = (lpszTitulo);
 ofn.lpstrDefExt = "txt";
 ofn.Flags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;
 return GetSaveFileName (&ofn);  
}

BOOL Open(HWND hWnd,LPSTR lpszFilter, LPSTR lpszTitle)
{
	static OPENFILENAME  ofn;   
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = (lpszFilter);
	ofn.lpstrFile = szBufferIn;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = (lpszTitle);
	ofn.Flags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;
	return GetOpenFileName (&ofn);	 
}

LRESULT CALLBACK AboutProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
 HYPCTRL hc;
 switch(uMsg){
  case WM_INITDIALOG:
	   SetDlgIcon(IDI_ICON1,hWnd);	  	   
	   InitHypCtrl(&hc);
	   hc.ulStyle	 = ulHover;
	   hc.szLink	 = TEXT("http://www.programacioncpp.irandohosting.0lx.net");
	   hc.szTooltip = TEXT("Visitar pagina Web");
	   CreateHypCtrl(hWnd, &hc, 20, 75, 0, 0);		
	   break;
  case WM_COMMAND:
	   switch(LOWORD(wParam)){
	     case ID_ABOUT_OK:
			  EndDialog(hWnd,0);
			  break;

	   }
	   break;
  case WM_CLOSE:
	   EndDialog(hWnd,0);
	   break;
  default: 
	   return 0;

 }
 return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

int GetMax(HWND hWnd,UINT nID)
{
 return (int)SendDlgItemMessage(hWnd,nID,LB_GETCOUNT,0,0); 
}

void SaveText(HWND hWnd,UINT nID,LPCTSTR strFileOut,short iMax)
{
 char *strText;
 DWORD dwTextLength=0;
 HANDLE hFile;
 hFile = CreateFile(strFileOut, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
 if(hFile != INVALID_HANDLE_VALUE){
  for(int index=0;index<iMax;index++){
   dwTextLength=(DWORD)SendDlgItemMessage(hWnd,nID,LB_GETTEXTLEN,(WPARAM)index,0);
   strText=new char[dwTextLength+1];
   if(strText!=NULL){
    DWORD dwWritten;
    SendDlgItemMessage(hWnd,nID,LB_GETTEXT,(WPARAM)index,(LPARAM)strText);
    WriteFile(hFile, strText, dwTextLength, &dwWritten, NULL);
	WriteFile(hFile, "\r\n", 2, &dwWritten, NULL);
    if(strText!=0)
     delete [] strText; 
   }
  }
  CloseHandle(hFile);
 }  
}


void CenterDlg(HWND hWnd)
{
    RECT r;
	GetClientRect(hWnd,&r);
	int m_Width		= r.right;
	int m_Height	= r.bottom;

	RECT ScreenRect;
	GetWindowRect (GetDesktopWindow(), &ScreenRect);
	SetWindowPos (hWnd, HWND_TOP, (((ScreenRect.right + ScreenRect.left) / 2) - (m_Width / 2)),
				 (((ScreenRect.bottom + ScreenRect.top) / 2) - (m_Height / 2)),
				  m_Width, m_Height, SWP_NOCOPYBITS | SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);
}

void SetDlgIcon(UINT nIDIcon,HWND hWnd)

{
 HICON hIcon;
 HINSTANCE hInst;
 hInst = (HINSTANCE) GetWindowLong( hWnd, GWL_HINSTANCE );
 hIcon=LoadIcon(hInst,MAKEINTRESOURCE(nIDIcon));
 SendMessage(hWnd,WM_SETICON,ICON_BIG, (LPARAM) hIcon);
 SendMessage(hWnd,WM_SETICON,ICON_SMALL,(LPARAM)hIcon);
}