/***************************************************************************/

//SERVER SIDE
#include <mmsystem.h;>

#pragma warning( disable : 4047)
#include <stdio.h;>
#include <winsock.h;>
#include <windows.h;>
#include <winbase.h;>
#include <process.h;>
#include <time.h;>

DWORD WINAPI ListenThread();	//thread prototypes
DWORD WINAPI ClientThread();
DWORD WINAPI GrantSession();
DWORD WINAPI socketHandler(void *pVoid);
DWORD WINAPI ChatHandler(void *pVoid);
HANDLE SuspendSelect(SOCKET sel);
char *getUser(SOCKET curr);
SOCKET getSocket(SOCKET who,int which);
SOCKET getChatSocket(SOCKET who,int which);
DWORD WINAPI TimeOut();
BOOL BiddingWar();
int LeaveSystem(SOCKET who);

void SendToAll();
void rearrangeList();
void Register();

typedef struct tagREQUEST
{
	SOCKET Socket;
	int recvd;
	int sent;
} REQUEST;

HANDLE TIMER,timerThread,adminLock;
char timeout[]="timeout";
DWORD grantAddr,timeoutAddr;
BOOL adminSess=1,adminReq=0;
CRITICAL_SECTION registerUsers, adminUpdate;
SOCKET	listenSocket,sessReqSocket,mainSock;	//main socket
SOCKET adminSocket,adminDuplex,adminChat;
SOCKADDR_IN saServer;
BOOL grantIsAlive=0,interrupt=0;
char indexCh[]="indexUpdate";
char chatrecv[1000];
BOOL runONCE=0;
int thrdId=0;
char soundbuffer[16000];	//send size
SOCKET requestList[40];
HANDLE	grantThread,chatThread;
HANDLE suspend;
typedef struct
{
	SOCKET reqSock;
} LIS;

LIS rlist[40];
typedef struct			//custom state table structure
{
	HANDLE threadHandle;
	char userName[20];
	int tableIndex;
	time_t entryTime;
	SOCKET socketClient;
	SOCKET duplex;
	SOCKET chat;
	BOOL granted;
} stateTable;

stateTable reqQ[40];		//state table
char szBuf[256];			//requests
char soundBuffer[16000];	//sound data 128 KB
char *checkMsg,*parse,*getUserName,*retrieveUserName;

int nRet,nLen,i,listLen=0; //error check::addr len::looper::req table size
int index=0,saveIndex;
int reqindex=0;
int adminMap;
BOOL forever=TRUE;
BOOL execONCE=FALSE;
void DistributeAudio(SOCKET selected, stateTable reqQ[],int bytesRead);
void DistributeChat(SOCKET selected, stateTable reqQ[],int bytesRead);
void UpdateList(LIS rlist[]);
int Match(REQUEST *lpReq, char *pMsg,int id);

void StreamServer(short nPort)
{
	HANDLE hlistenThread;
	DWORD ThreadAddr;

	listenSocket = socket(AF_INET,			// Address family
						  SOCK_STREAM,		// Socket type
						  IPPROTO_TCP);		// Protocol
	if (listenSocket == INVALID_SOCKET)
	{
		printf("Error at socket()");
		return;
	}
	// Fill in the address structure

	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = INADDR_ANY;	// Let WinSock supply address
	saServer.sin_port = htons(nPort);		// Use port from command line
	// bind the name to the socket

	nRet = bind(listenSocket,				// Socket
				(LPSOCKADDR)&saServer;,		// Our address
				sizeof(struct sockaddr));	// Size of address structure
	if (nRet == SOCKET_ERROR)
	{
		printf("Error at bind()");
		closesocket(listenSocket);
		return;
	}

	nLen = sizeof(SOCKADDR);
	nRet = gethostname(szBuf, sizeof(szBuf));
	if (nRet == SOCKET_ERROR)
	{
		printf("No host");
		closesocket(listenSocket);
		return;
	}
	// Show the server name and port number

	printf("\nServer named %s waiting on port %d\n",szBuf, nPort);
	// Set the socket to listen

	printf("\nlisten()");
	nRet = listen(listenSocket,					// Bound socket
				  SOMAXCONN);					// Number of connection request queue
	if (nRet == SOCKET_ERROR)
	{
		printf("Error at Listen()");
		closesocket(listenSocket);
		return;
	}
	hlistenThread = CreateThread(0,
								 0,
								 (LPTHREAD_START_ROUTINE)ListenThread,
								 0,
								 0,
								 &ThreadAddr;);

	WaitForSingleObject(hlistenThread, INFINITE);
}

DWORD WINAPI ListenThread()
{

	DWORD		ThreadAddr;		//Thread start address
	HANDLE		dwClientThread;	//Client Thread Handle
	SOCKADDR_IN	SockAddr[50];	//Array of socket addresses
	int	nLen;
	REQUEST *lpReq;

	while(1)
	{	// Block on accept()
		nLen = sizeof(SOCKADDR_IN);
		printf("\naccept()");
		printf("\nIndex :");
		printf("%d",index);
		mainSock = accept(listenSocket,(LPSOCKADDR)&SockAddr;[index],&nLen;);

		lpReq=malloc(sizeof(REQUEST));
		if(lpReq==NULL)
		{
			printf("\nMemory unavailable for request\n");
			continue;
		}
		if (mainSock == INVALID_SOCKET)
		{
			printf("Error at accept(): ListenThread\n");
			break;
		}

		lpReq->Socket=mainSock;
		lpReq->sent=0;
		lpReq->recvd=0;

		// connection OK

		// Start a client/worker thread to handle this request
		dwClientThread = CreateThread(0,
									  0,
									  (LPTHREAD_START_ROUTINE)ClientThread,
									  lpReq,
									  0,
									  &ThreadAddr;);

		if (!dwClientThread)
		{
			printf("Could not create thread");
			exit(1);
		}

	}

	return 0;
}

DWORD WINAPI ClientThread(void *pVoid)
{
	REQUEST *lpReq=(REQUEST *)pVoid;
	char *checkStr;
	DWORD	chatAddr;		//Thread start address
	int sock2;
	char szBuf[250];			//very volatile :remove

	memset(szBuf, 0, sizeof(szBuf));
	nRet = recv(lpReq->Socket,
				szBuf,
				sizeof(szBuf),
				0);
	if (nRet == INVALID_SOCKET)
	{
		printf("\nError at recv()");
		return 0;
	}

	checkStr=strstr(szBuf,"Register");
	if(checkStr!=NULL)
	{
		EnterCriticalSection(®isterUsers;);
		Register(lpReq,checkStr);
		memset(szBuf, 0, sizeof(szBuf));	//remove

		if(runONCE==0)
		{
			grantThread=CreateThread(0,
									 0,
									 (LPTHREAD_START_ROUTINE)GrantSession,
									 0,
									 0,
									 &grantAddr;);
			runONCE=1;
		}

		/*timerThread= CreateThread(0,
								  0,
								  (LPTHREAD_START_ROUTINE)TimeOut,
								  0,
								  0,
								  &timeoutAddr;);/**/

		index++;
		LeaveCriticalSection(®isterUsers;);

		nRet=send(lpReq->Socket,"Accepted",sizeof("Accepted"),0);
		return 0;
	}


	checkStr=strstr(szBuf,"Duplex");
	if(checkStr!=NULL)
	{
		sock2=Match(lpReq,checkStr,1);
		memset(szBuf, 0, sizeof(szBuf));	//remove
		if(sock2==-1)
			printf("\nError mapping Duplex Socket");
		return 0;
	}

	checkStr=strstr(szBuf,"Chat");
	if(checkStr!=NULL)
	{
		sock2=Match(lpReq,checkStr,2);
		memset(szBuf, 0, sizeof(szBuf));	//remove
		if(sock2==-1)
		{
			printf("\nError mapping chat socket");
			return 0;
		}
		else
		{
			send(lpReq->Socket,"ChatReady",sizeof("ChatReady"),0);
			chatThread=CreateThread(0,
									0,
									(LPTHREAD_START_ROUTINE)ChatHandler,
									lpReq,
									0,
									&chatAddr;);



			return 0;
		}
		return 0;
	}

	else
	{
		printf("\nUnrecognized request");
	}
	return 0;
}

DWORD WINAPI ChatHandler(void *pVoid)
{
	REQUEST *lpReq=(REQUEST *)pVoid;
	int nRet=0;
	char *intchk;
	SOCKET save=0;

	printf("\nChat Handler started");

	while(1)
	{
		memset(chatrecv, 0,sizeof(chatrecv));
		nRet=recv(lpReq->Socket, chatrecv, sizeof(chatrecv), 0);
		if(nRet==SOCKET_ERROR)
		{
			printf("\nInvalid socket: %d Terminating ",lpReq->Socket);
			return 1;
		}

		intchk=strstr(chatrecv,"interruptaudio");
		if(intchk!=NULL)
		{
			interrupt=1;
			continue;
		}

		intchk=strstr(chatrecv,"reqAudioSess");
		if(intchk!=NULL)
		{
			if(reqindex==0)
			{
				rlist[reqindex].reqSock=getSocket(lpReq->Socket,3);
				ResumeThread(grantThread);
				printf("\nStarted Session Grant");
				reqindex++;
				send(lpReq->Socket,"AckRequest",sizeof("AckRequest"),0);
				continue;
			}
			rlist[reqindex].reqSock=getSocket(lpReq->Socket,3);
			reqindex++;
			send(lpReq->Socket,"AckRequest",sizeof("AckRequest"),0);
		}

		intchk=strstr(chatrecv,"adminComment");
		if(intchk!=NULL)
		{
			if((reqindex==0)&&(adminReq==0))	//audio dist thread is suspended
				ResumeThread(grantThread);

			adminReq=1;
			WaitForSingleObject(adminLock, INFINITE);
			send(lpReq->Socket,"AckRequest",sizeof("AckRequest"),0);

		}
	}
	return 0;
}

DWORD WINAPI GrantSession()
{
	int nRet=0,totaldata=0,audiodata=479000,m=0;
	char *sessOwner,lec[]="Lecturer";

	SOCKET save=0;
	BOOL forcedExit=0;

	TIMER=CreateEvent(NULL,TRUE,TRUE,timeout);
	adminLock=CreateEvent(NULL,TRUE,TRUE,NULL);
	while(1)
	{
		adminSess=BiddingWar();		//has the admin requested audience

		if(((rlist[0].reqSock==NULL)||(rlist[0].reqSock==0))&&(adminReq==0))
		{
			printf("\nSelf Suspend");
			SuspendThread(grantThread);
			grantIsAlive=0;
			continue;
		}

		if(adminSess==1)
		{
			SOCKET temp;
			temp=getChatSocket(adminSocket,1);
			nRet=send(temp,"AudioSessPerms",sizeof("AudioSessPerms"),0);
			sessOwner=&lec;
		}
		else
		{
			SOCKET temp;
			ResetEvent(adminLock);
			temp=getChatSocket(rlist[0].reqSock,1);
			nRet=send(temp,"AudioSessPerms",sizeof("AudioSessPerms"),0);
			sessOwner=getUser(rlist[0].reqSock);
		}

		if(sessOwner!=NULL)						//Alert everyone who is talking
		{
			char format[25]="SessionOwner:";
			strcat(format,sessOwner);
			for(m=0; m<=index; m++)
			{
				send(reqQ[m].chat,format,sizeof(format),0);
			}
		}
		//Very fine tuning needed here
		while(totaldata < audiodata)		//? Jump
		{
			if(interrupt==1)	//sender does not want to use all h/is/er time slice
			{
				interrupt=0;
				totaldata=0;
				printf("\nSession Stopped");
				for(m=0; m<=index; m++)
				{
					send(reqQ[m].chat,"sessionInterrupt",sizeof("sessionInterrupt"),0);
				}
				break;
			}
			if(adminSess==1)
			{
				nRet=recv(adminSocket, soundbuffer, sizeof(soundbuffer), 0);
			}
			else
			{
				nRet=recv(rlist[0].reqSock, soundbuffer, sizeof(soundbuffer), 0);
			}

			if(nRet==SOCKET_ERROR)
			{
				if(adminSess==0)
					LeaveSystem(rlist[0].reqSock);	//invalidate this user's entries
				else
					LeaveSystem(adminSocket);

				printf("\nConnection closed by peer");
				printf("Sock Error: %d:", WSAGetLastError());
				break;
			}

			totaldata+=nRet;
			printf("\n");
			printf("%d", totaldata);
			if(adminSess==1)
			{
				DistributeAudio(adminSocket, reqQ, nRet);
			}
			else
			{
				DistributeAudio(rlist[0].reqSock, reqQ, nRet);
			}
		}
		if(adminSess==0)
		{
			UpdateList(rlist);
			SetEvent(adminLock);
		}
		totaldata=0;	//ready for more data

		if(adminSess==1)
		{
			adminReq=0;		//allow users to talk
		}
		if((adminSess==1)&&(rlist[0].reqSock!=0))
		{
			adminSess=0;		//allow users to talk if any have requested
		}

	}
}

void Register(REQUEST *lpReq, char *pMsg)							//totally automatic
{
	char *adminChk;
	reqQ[index].socketClient=lpReq->Socket;		//save socket
	parse=strstr(pMsg,"xx");
	adminChk=strstr(parse, "Admin");
	if(adminChk!=NULL)
	{
		adminSocket=lpReq->Socket;
	}
	if(parse!=NULL)
	{
		parse+=2;					//get rid of 'xx'
		strcpy(reqQ[index].userName,parse);
		reqQ[index].tableIndex=index;
		reqQ[index].granted=FALSE;
	}

	else
		printf("\nUnknown Request");
}

int Match(REQUEST *lpReq, char *pMsg, int id)
{
	char *parse,*match;
	int h=0;
	parse=strstr(pMsg,"xx");
	if(parse!=NULL)
	{
		for(h=0;h
			match=strstr(parse, reqQ[h].userName);	//compiler misbehaving
			if(match!=NULL)
			{
				if(id==1)
				{
					reqQ[h].duplex=lpReq->Socket;
					send(lpReq->Socket,"RegDuplex",sizeof("RegDuplex"),0);	//ack
					return 1;
				}
				if(id==2)
				{
					reqQ[h].chat=lpReq->Socket;
					send(lpReq->Socket,"ChatConn",sizeof("ChatConn"),0);	//ack
					return 1;
				}
			}
			else
				continue;
		}
	}

	return -1;			//should not arrive here return err
}

void DistributeAudio(SOCKET selected, stateTable reqQ[],int bytesRead)
{
	int i=0,retval=0;

	for(i=0; i<=index; i++)
	{
		//retval=send(reqQ[i].duplex, soundbuffer,bytesRead,0);
		if(reqQ[i].socketClient!=selected)
		{
			//send audio to duplex socket
			send(reqQ[i].duplex, soundbuffer,sizeof(soundbuffer),0);
		}
		else
			continue;		//recvd from this socket: skip/**/
	}
}

void UpdateList(LIS rlist[])
{
	int j=0;
	printf("\nUpdating list");
	for(j=0;j
