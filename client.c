//CLIENT SIDE CODE
/*
-------------BUG BASE-------------
Line 237 MainWndProc()(DeleteCriticalSection: Mutex object not freed)==OK


Line 62  WinMain	Relocate code to InitDialog==OK

Unresolved: Wrong Timeouts
			Socket Numbers 80->2000
			Win32  Registry==beta OK
			Record Alert
			HTTP headers
			Record progress
			Trimming

*/

#define _WIN32_LEAN_AND_MEAN
#pragma warning( disable : 4133)
#include "variables.h"
#include "resource.h"

char* filename();
void SaveAll();
void PlayFile();
void W32Registry();
char fileName[15];
int filecount=0;			//saved audio session
BOOL recordAll=0;
RECT clipBox;
HDC recDC,crecDC;
HBRUSH redBrush;
BOOL timeoutFlag=0;
HWND hRWnd,recBar;
HANDLE Permissions;
char aupermz[]="aupermz";
//method definitions

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	MMRESULT						err;
	WAVEFORMATEX					waveFormat;
	MIXERLINE						mixerLine;
	HANDLE							waveInThread;
	unsigned long					numSrc;
	WSADATA wsaData;
	int nRet;


	WORD wVersionRequested = MAKEWORD(1,1);
	redBrush= CreateSolidBrush(RGB(255, 0, 0));
	savedInst=hInstance;
	recordAll=0;		//***********
	freeBlocksCount=BLOCK_COUNT;

	InitializeCriticalSection(&adjustFreeBlocks;);
	Permissions = CreateEvent( NULL, TRUE, FALSE, aupermz);
	WaveFileHandle = CreateFile((LPCTSTR)filename(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if(WaveFileHandle==INVALID_HANDLE_VALUE)
	{
		int ree=GetLastError();
	}
	nRet = WSAStartup(wVersionRequested, &wsaData;);
	if (nRet)
	{
		WSACleanup();
		return 0;
	}

	// Check WinSock version
	if (wsaData.wVersion != wVersionRequested)
	{
		WSACleanup();
		return 0;
	}

	//move this to InitDialog

	InitCommonControls();

	waveInThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)waveInProc, 0, 0, &err;);
	if (!waveInThread)
	{
		sprintf(formatStr,"ERROR Can't create WAVE recording thread!", GetLastError());
		MessageBox(hWnd,formatStr,"Fatal Error",MB_OK|MB_ICONERROR);
	}

	// Clear out both of our WAVEHDRs. waveInPrepareHeader() expects the dwFlags field to
	  // be cleared
	 //
	/******SIMULATION START */

	ZeroMemory(&WaveHeader;[0], sizeof(WAVEHDR) * 2);

	// Initialize the WAVEFORMATEX for 8-bit, 8KHz, stereo.This is ICS, we dont have Gigabyte
	//   internet
	waveFormat=makeWaveFormat();
	 // Open the default WAVE In Device, specifying CALLBACK.
	  // After waveInOpen returns the handle, use waveInGetID to fetch its ID,
	  // and then waveInGetDevCaps to retrieve the actual name


	err = waveInOpen(&WaveInHandle;, WAVE_MAPPER, &waveFormat;, (DWORD)err, 0, CALLBACK_THREAD);
	if (err)
	{
		sprintf(formatStr,"ERROR : Can't open recording device! Close all recording applications and restart", GetLastError());
		//MessageBox(hWnd,formatStr,"Fatal Error",MB_OK|MB_ICONERROR);
		//ExitProcess(1);
	}
	else
	{
	    // Open the mixer associated with the WAVE In device opened above.
		err = mixerOpen(&MixerHandle;, (DWORD)WaveInHandle, 0, 0, MIXER_OBJECTF_HWAVEIN);
		if (err)
		{
			//checking card mixer support
			//printf("Device does not have mixer support! -- %08X\n", err);
			sprintf(formatStr,"No mixer support %08X",err);
			SetWindowText(hDiag,formatStr);
		}

		else
		{
			// This device should have a WAVEIN destination line.Get its ID identify
			// source lines available to record from

			mixerLine.cbStruct = sizeof(MIXERLINE);
			mixerLine.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
			err = mixerGetLineInfo((HMIXEROBJ)MixerHandle, &mixerLine;, MIXER_GETLINEINFOF_COMPONENTTYPE);
			if (err)
			{
				//printf("Device does not have a WAVE recording control! -- %08X\n", err);
				sprintf(formatStr,"Device does not have WAVE recording control -- %08X",err);
				SetWindowText(hDiag,formatStr);

				goto record;
			}

			// Get how many source lines are available from which to record.
			  // For example, there could
			  // be a Mic In (MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE),
			 //  Line In (MIXERLINE_COMPONENTTYPE_SRC_LINE),
			 //  and/or SPDIF In (MIXERLINE_COMPONENTTYPE_SRC_DIGITAL)

			numSrc = mixerLine.cConnections;

			//If the WAVEIN destination line has a mute switch, then make sure that
			   //it is not muted.
			mixerLine.dwSource = mixerLine.dwDestination;
			//set_mute(&mixerLine;, 1, ERRMSG_NONE);

			// Make sure that there is at least one WAVEIN source line available.
			//   If not, then just go with the
			//   WAVEIN master volume and mute above

			if (!numSrc)
			{
				sprintf(formatStr,"No WAVE inputs to adjust");
				SetWindowText(hDiag,formatStr);

				//printf("ERROR: There are no WAVE inputs to adjust!\n");
				goto record;
			}

record:		mixerClose(MixerHandle);
		}

		// we can record an audio file using waveInPrepareHeader(),
		// Allocate, prepare, and queue two buffers that the driver can use to record
		//   blocks of audio data. [double-buffering]
				WaveHeader[1].dwBufferLength = WaveHeader[0].dwBufferLength = waveFormat.nAvgBytesPerSec << 1;
		if (!(WaveHeader[0].lpData = (char *)VirtualAlloc(0, WaveHeader[0].dwBufferLength * 2, MEM_COMMIT, PAGE_READWRITE)))
		{
			sprintf(formatStr,"Error allocating memory for recording buffers,now quitting");
			MessageBox(hWnd,formatStr,"Fatal Error",MB_OK|MB_ICONERROR);
			ExitProcess(1);
		}
		else
		{
			// Fill in WAVEHDR fields for buffer starting address.
			//We've already filled in the size fields above
			WaveHeader[1].lpData = WaveHeader[0].lpData + WaveHeader[0].dwBufferLength;
			// Prepare the 2 WAVEHDR's
			if ((err = waveInPrepareHeader(WaveInHandle, &WaveHeader;[0], sizeof(WAVEHDR))))
			{
				//printf("Error preparing WAVEHDR 1! -- %08X\n", err);
				sprintf(formatStr,"Error preparing WAVEHEADER 1-- %08X",err);
				SetWindowText(hDiag,formatStr);

			}
			else
			{
				if ((err = waveInPrepareHeader(WaveInHandle, &WaveHeader;[1], sizeof(WAVEHDR))))
				{
					sprintf(formatStr,"Error preparing WAVEHEADER 1-- %08X",err);
		    		SetWindowText(hDiag,formatStr);
				}
				else
				{
					// Queue first WAVEHDR (recording hasn't started yet)
					if ((err = waveInAddBuffer(WaveInHandle, &WaveHeader;[0], sizeof(WAVEHDR))))
					{
						sprintf(formatStr,"Error queuing WAVEHEADER 1-- %08X",err);
						SetWindowText(hDiag,formatStr);

					}
					else
					{
						// Queue second WAVEHDR
						if ((err = waveInAddBuffer(WaveInHandle, &WaveHeader;[1], sizeof(WAVEHDR))))
						{
							sprintf(formatStr,"Error queing WAVEHEADER 2-- %08X",err);
							SetWindowText(hDiag,formatStr);

							DoneAll = 1;
						}
					}
				}
			}
		}
	}	/**/
	//SIMULATION END

	DialogBox(hInstance,
			  MAKEINTRESOURCE(IDD_DIALOG1),
			  NULL,
			  MainWndProc);

	return (0);
}

BOOL CALLBACK MainWndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	hWnd=hwnd;


	switch (uMsg)
	{
		case WM_INITDIALOG:			InitDialog(hwnd);
									break;
		case WM_CLOSE:				{
										freeBlocks(waveBlocks);
										waveOutClose(HWaveOut);
										CloseHandle(WaveFileHandle);
    									DestroyWindow(hwnd);
									}
									break;

		case WM_VSCROLL:			GetVolume();
									break;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDCANCEL:		EndDialog(hwnd,IDCANCEL);
									break;
				case IDC_BSEND:		GetChatText();
									break;
				case IDC_REQUEST:   {
										DWORD requestAddr;
										registry(hwnd);
										//W32Registry();
										CreateThread( NULL,		  //launch thread to recv audio
													  0,
													  (LPTHREAD_START_ROUTINE)Request,
													  NULL,
													  0,
													  &requestAddr;);

									}
									break;
				case IDC_STOP:		{
										EnableWindow(hRec,FALSE);
										EnableWindow(hSessReq,TRUE);
										stopRecord();
									}
									break;
				case IDC_RECORD:	{
										//registry(hwnd);
										Record();
									}
									break;
				case IDC_SAVECURR:	flashWrite();
									break;
				case IDC_SAVEALL:	SaveAll();
									break;
				case IDC_PLAY:		PlayFile();
									break;


				//Add command handlers for controls
			}
			break;

	}

	return FALSE;
}

//MEDIA METHODS
WAVEHDR* allocateBlocks(int size, int count)
{
    unsigned char* buffer;
    int i;
    WAVEHDR* blocks;
    DWORD totalBufferSize = (size + sizeof(WAVEHDR)) * count;

	// allocate memory for the entire set in one go
    if((buffer = HeapAlloc( GetProcessHeap(),
							HEAP_ZERO_MEMORY,
							totalBufferSize)) == NULL)
	{
        SetWindowText(hDiag, "Could not allocate memory for audio buffers");
        ExitProcess(1);
    }

    //set up the pointers

    blocks = (WAVEHDR*)buffer;
    buffer += sizeof(WAVEHDR) * count;
    for(i = 0; i < count; i++) {
        blocks[i].dwBufferLength = size;
        blocks[i].lpData = buffer;
        buffer += size;
    }

    return blocks;
}

void freeBlocks(WAVEHDR* blockArray)
{
	HeapFree(GetProcessHeap(), 0, blockArray);
}

void BufferAudio(HWAVEOUT hWaveOut, LPSTR data, int size)
{
    WAVEHDR* current;
    int remain;

	buffered+=size;
    current = &waveBlocks;[waveCurrentBlock];

	if(waveCurrentBlock==0)
	{
		EnableWindow(hSaveCurr, FALSE);
		//SendMessage(hProgress, PBM_SETPOS,(WPARAM) 0, 0);
	}

    while(size > 0)
	{
        //unprepare header to be used

        if(current->dwFlags & WHDR_PREPARED)
            waveOutUnprepareHeader(hWaveOut, current, sizeof(WAVEHDR));

        if(size < (int)(BLOCK_SIZE - current->dwUser))
		{
			if(waveCurrentBlock==58)
				break;
            memcpy(current->lpData + current->dwUser, data, size);
            current->dwUser += size;
            break;
        }

        remain = BLOCK_SIZE - current->dwUser;
		if(waveCurrentBlock==58)
				break;
        memcpy(current->lpData + current->dwUser, data, remain);
        size -= remain;
        data += remain;
        current->dwBufferLength = BLOCK_SIZE;

		//point to next block
        waveCurrentBlock++;
		SendMessage(hProgress, PBM_STEPIT, 0, 0);
        //waveCurrentBlock %= BLOCK_COUNT;

        current = &waveBlocks;[waveCurrentBlock];
        current->dwUser = 0;
    }
}

void PlayAudio(HWAVEOUT hWaveOut,int playindex)
{
    WAVEHDR* current;
	MMRESULT play;

	current = &waveBlocks;[playindex];
	waveOutPrepareHeader(hWaveOut, current, sizeof(WAVEHDR));
    play=waveOutWrite(hWaveOut, current, sizeof(WAVEHDR));
	if(play==MMSYSERR_NOERROR)
	{
		EnterCriticalSection(&adjustFreeBlocks;);
        freeBlocksCount--;
        LeaveCriticalSection(&adjustFreeBlocks;);
		buffered-=BLOCK_SIZE;	//Adjust buffered data
	}
}


int SockWrite(int Sock,char *Buff,int Len)
{
   int Answer,val=0;
   Answer=send(Sock,Buff,Len,0);

   return(Answer);
}


int SockRead(int Sock,char *Buff,int Len)
{
	int Answer,total=0,len=0;
    int val=0,loop=0;

   Answer=recv(Sock,Buff,Len,0);
   if(Answer==SOCKET_ERROR)
   {
	   sprintf(formatStr,"Socket Error %d: Could not receive",WSAGetLastError());
	   SetWindowText(formatStr,hChatSend);
   }

   SetWindowText(hChatRecv,Buff);
   return(Answer);

}

void postReq(int textaudio, SOCKET conn)
{
	SOCKET Socket=conn;
	int length;
	char recvbuf[20];
	memset(recvbuf, 0, sizeof(recvbuf));
	memset(sstr, 0, sizeof(sstr));
	if(textaudio==1)
	{
		length=30;
		strcpy(sstr,"Registerxx");
		strcat(sstr,usrName);
	}

	if(textaudio==2)
		length=80000;			//audio

	if(textaudio==3)			//entry : register
	{
		char *adminChk=strstr(usrName,"Administrator");
		if(adminChk!=NULL)
			strcpy(sstr,"adminCommentxx");
		else
			strcpy(sstr,"reqAudioSessxx");
		length=30;
		strcat(sstr,usrName);
	}

	if(textaudio==4)
		{
		length=30;
		strcpy(sstr,"Duplexxx");
		strcat(sstr,usrName);
	}

	if(textaudio==5)
		{
		length=30;
		strcpy(sstr,"Chatxx");
		strcat(sstr,usrName);
	}

	sprintf(request,"POST %s HTTP/1.1\n",Path);
    strcat(request,"Host: localhost\n");
	strcat(request,"Content-type: text/html\n");
	sprintf(rqs,"Content-length: %i\n",length);
    strcat(request,rqs);/**/
    strcat(request,"\n");
	strcat(request,sstr);

    SockWrite(Socket,request,strlen(request));
	if(regreq==0)		//read if sockets not initialized
		SockRead(Socket,recvbuf,20);

	//MessageBox(hWnd,recvbuf,"Raw Server Msg",MB_OK|MB_ICONINFORMATION);
}

SOCKET GetHTTP(LPCSTR lpServerName, LPCSTR lpFileName)
{
	SOCKET sock;
	IN_ADDR		iaHost;
	LPHOSTENT	lpHostEntry;

	iaHost.s_addr = inet_addr(lpServerName);
	if (iaHost.s_addr == INADDR_NONE)
	{
		lpHostEntry = gethostbyname(lpServerName);
	}
	else
	{
		lpHostEntry = gethostbyaddr((const char *)&iaHost;,
						sizeof(struct in_addr), AF_INET);
	}
	if (lpHostEntry == NULL)
	{
		MessageBox(hWnd,"Unable to resolve host,now quitting.Try connecting another time","Fatal Error",MB_OK|MB_ICONERROR);
		ExitProcess(1);
		return 0;
	}

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		MessageBox(hWnd,"Unable to create valid tcp socket,now quitting.","Fatal Error",MB_OK|MB_ICONERROR);
		ExitProcess(1);
		return 0;
	}

	// Find the port number for the HTTP service on TCP
	/*lpServEnt = getservbyname("http", "tcp");
	if (lpServEnt == NULL)
		saServer.sin_port = htons(80);
	else
		saServer.sin_port = lpServEnt->s_port;*/

	saServer.sin_port = htons(2000);

	saServer.sin_family = AF_INET;
	saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);

	nRet = connect(sock, (LPSOCKADDR)&saServer;, sizeof(SOCKADDR_IN));
	if (nRet == SOCKET_ERROR)
	{
		int err=WSAGetLastError();
		MessageBox(hWnd,"Unable to connect to host,now quitting","Fatal Error",MB_OK|MB_ICONERROR);
		closesocket(sock);
		ExitProcess(1);
		return 0;
	}
	return sock;
}


/*---------waveInProc() Background thread for handling msgs sent by audio driver
 Windows Low Level WAVE API passes messages regarding digital audio recording (such as MM_WIM_DATA, MM_WIM_OPEN, and
 MM_WIM_CLOSE) to this thread
 pass CALLBACK_THREAD and the ID of this thread to waveInOpen [opens device] /**/


DWORD WINAPI waveInProc(LPVOID arg)
{
	MSG		msg;
	int rcall=0;
	MMRESULT rerr;

	while (GetMessage(&msg;, 0, 0, 0) == 1)	//msg loop
	{
		switch (msg.message)
		{
			case MM_WIM_DATA:	//buffer filled by driver
			{
				   //msg.lParam contains a pointer to the WAVEHDR structure for the filled buffer.
				   //The WAVEHDR's dwBytesRecorded field specifies how many bytes of
				   //audio data are in the buffer.
				if (((WAVEHDR *)msg.lParam)->dwBytesRecorded)
				{
					if(count==29)		//ACTUALLY sent (count+1)*16000
					{
						stopRecord();	// after 480 KiloBytes
						SetWindowText(hDiag,"Recording Stopped");
						EnableWindow(hRec,FALSE);
						EnableWindow(hSessReq,TRUE);
						//continue;
					}
					SockWrite(Socket,((WAVEHDR *)msg.lParam)->lpData,((WAVEHDR *)msg.lParam)->dwBytesRecorded);
					SendMessage(hRecProg, PBM_STEPIT, 0, 0);
					count++;

				}


				if (InRecord)	//? true requeue buffer
				{
					rerr=waveInAddBuffer(WaveInHandle, (WAVEHDR *)msg.lParam, sizeof(WAVEHDR));

				}

				else //? !recording last filled buffer has been returned
				{
					SetWindowText(hDiag,"InRecord ->FALSE");
					++DoneAll;
				}

				continue;		//more msgs
			}

			case MM_WIM_OPEN:		//main thread opening audio device [mic]
			{
				SetWindowText(hDiag,"Opening Recording Device");
				DoneAll = 0;		//initialize
				continue;			//msgs
			}

			case MM_WIM_CLOSE:		//main thread closing audio device [mic]
			{
				MessageBox(hWnd,"Device Closed","Alert",MB_OKCANCEL|MB_ICONERROR);
				break;				//Terminate this thread (by returning)
			}
		}
	}

	return(0);		//Okay
}/**/
/*
DWORD WINAPI waveInProc(LPVOID arg)
{
	char blankbuf[16000];

	memset(blankbuf,0,sizeof(blankbuf));
	while(1)
	{
		if(InRecord==FALSE)
		{
			Sleep(2000);
			continue;
		}
		if(count==29)		//ACTUALLY sent (count+1)*16000
		{
			stopRecord();	// after 480 KiloBytes
			SetWindowText(hDiag,"Recording Stopped");
			EnableWindow(hRec,FALSE);
			EnableWindow(hSessReq,TRUE);
			//continue;
		}
		SockWrite(Socket,blankbuf,16000);
		SendMessage(hRecProg, PBM_STEPIT, 0, 0);
		count++;
		Sleep(500);
	}
	return(0);		//Okay
}

/**/





/*						set_mute()
   Uses the Mixer API to check if the specified line has a mute control, and
   if so, sets it to the specified value.

   mixerLine =		Pointer to a MIXERLINE filled in with information about
  					the desired line (ie, its name and ID and number of
  					channels).

   value =			The value to set mute control to (ie, 0 = mute, 1 = unmute).

   errmsg =			ERRMSG_PRINT display an error message whenever we
 					find that the line has no mute control. ERRMSG_NONE ignore

   The Mixer must be open and its handle stored in the global
  'MixerHandle'.
 /**/


void set_mute(MIXERLINE *mixerLine, DWORD val, BOOL errmsg)
{
	MIXERCONTROL					mixerControlArray;
	MIXERLINECONTROLS				mixerLineControls;
	MIXERCONTROLDETAILS_UNSIGNED	value[2];
	MIXERCONTROLDETAILS				mixerControlDetails;
	MMRESULT						err;

	/* Check control/line mute switch
	   fetch information about any mute control (if there is one). This will also
	   fetch us the control's ID which is needed later in order to set its value
	 */

	mixerLineControls.cbStruct = sizeof(MIXERLINECONTROLS);
	mixerLineControls.dwLineID = mixerLine->dwLineID;	//assign line ID for info
	mixerLineControls.cControls = 1;	//info for 1 line

	//[mute switch]for which type of control we're retrieving info.
	mixerLineControls.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;

	/* Give mixerGetLineControls() the address of the MIXERCONTROL struct to return info */
	mixerLineControls.pamxctrl = &mixerControlArray;
	mixerLineControls.cbmxctrl = sizeof(MIXERCONTROL);		//assign size of MIXERCONTROL

	/* Retrieve info only upon any mute control for this line */
	if ((err = mixerGetLineControls((HMIXEROBJ)MixerHandle, &mixerLineControls;, MIXER_GETLINECONTROLSF_ONEBYTYPE)))
	{
		if (errmsg == ERRMSG_PRINT)	//Error
			printf("%s has no mute control!\n", mixerLine->szName);
	}
	else
	{
		// Muting this line.
		mixerControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);

		/* Tell mixerSetControlDetails() which control whose value(s) we
		  want to set by putting the desired control's
		  ID number in dwControlID.
		/**/
		mixerControlDetails.dwControlID = mixerControlArray.dwControlID;

		/* Tell how many channels we're muting.ASSUMING that there are only 2 channels.
		 [first two channels]
		 */
		mixerControlDetails.cChannels = mixerLine->cChannels;
		if (mixerControlDetails.cChannels > 2) mixerControlDetails.cChannels = 2;
		if (mixerControlArray.fdwControl & MIXERCONTROL_CONTROLF_UNIFORM) mixerControlDetails.cChannels = 1;

		// ASSUMING that the mute switch doesn't have multiple parameters.
		mixerControlDetails.cMultipleItems = 0;
		mixerControlDetails.paDetails = &value;[0];
		mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
		value[0].dwValue = value[1].dwValue = val;

		//Set the value of the mute control for this line
		if ((err = mixerSetControlDetails((HMIXEROBJ)MixerHandle, &mixerControlDetails;, MIXER_SETCONTROLDETAILSF_VALUE)))
		{
			printf("Error #%d setting mute for %s!\n", err, mixerLine->szName);	//error
		}
	}
}

//------------------------PLAYBACK METHODS-----------------
/*						 WaveOutProc()
   WAVE callback. This is called by Windows every time that some event
   needs to be handled by the program.
 /**/

void CALLBACK WaveOutProc(HWAVEOUT waveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	if (uMsg == MM_WOM_DONE)	//buffer finished playing?
	{
		int* freeBlockCounter = (int*)dwInstance;
		EnterCriticalSection(&adjustFreeBlocks;);
		(*freeBlockCounter)++;
		LeaveCriticalSection(&adjustFreeBlocks;);
		wcount++;
		reqBuffer=dwParam1;
		played=1;

	}
}


WAVEFORMATEX makeWaveFormat()
{
	WAVEFORMATEX waveFormat;
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;	//pulse code modulation
	waveFormat.nChannels = 1;
	waveFormat.nSamplesPerSec = 8000;		//Telephone quality
	waveFormat.wBitsPerSample = 8;
	waveFormat.nBlockAlign = waveFormat.nChannels * (waveFormat.wBitsPerSample/8);
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	return waveFormat;
}

void stopRecord()
{
	InRecord = FALSE;
	if(recBar!=NULL)
	{
		ShowWindow(recBar,SW_HIDE);
		DestroyWindow(recBar);
	}

	first=TRUE;			//recorded 1 session
	if((count < 29) && (timeoutFlag==0))
	{
		send(chatSocket,"interruptaudio",sizeof("interruptaudio"),0);
		//count=0;
		//waveCurrentBlock=0;
		Sleep(5000);				//sometime for message to arrive
	}

	if(timeoutFlag==1)
		timeoutFlag=0;

	waveInReset(WaveInHandle);
	while (DoneAll < 2)
	{
		Sleep(100);
		break;
	}
}

void Record()
{
	MMRESULT err;


	//InRecord = TRUE;
	count=0;
	//recBar=CreateDialog
	recBar=CreateDialog(savedInst,MAKEINTRESOURCE(IDD_DIALOG3),hWnd, RecordProg);

	if(InRecord==TRUE)
	{
		MessageBox(hWnd, "Record in progress", "Alert", MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	InRecord = TRUE;
	if(first==TRUE)
	{
		//re-add wave buffers to driver queue
		if ((err = waveInAddBuffer(WaveInHandle, &WaveHeader;[0], sizeof(WAVEHDR))))
		{
			sprintf(formatStr,"Error REQUEING WAVEHEADER 1-- %08X",err);
			SetWindowText(hDiag,formatStr);
		}
		else
		{
			// Queue second WAVEHDR
			if ((err = waveInAddBuffer(WaveInHandle, &WaveHeader;[1], sizeof(WAVEHDR))))
			{
				sprintf(formatStr,"Error REQUEING WAVEHEADER 2-- %08X",err);
				SetWindowText(hDiag,formatStr);

				DoneAll = 1;
			}
		}
	}

	err = waveInStart(WaveInHandle);

	if (err!=MMSYSERR_NOERROR )
	{
		sprintf(formatStr,"Record error %08X",err);
		SetWindowText(hDiag,formatStr);
	}

}


DWORD WINAPI Request()
{
	unsigned int bufsize=0;
	int len=sizeof(int);

	if(regreq==0)
	{
		char *admin;
		admin=strstr(usrName,"Administrator");
		if(admin!=NULL)
			SetWindowText(hSessReq,"&Comment;");
		else
			SetWindowText(hSessReq,"&Request;");

		EnableWindow(hSessReq,FALSE);
		postReq(1,Socket);				//register
		postReq(4,recvSocket);			//set up second connection
		postReq(5,chatSocket);			//chat
		EnableWindow(hSessReq,TRUE);
		ResumeThread(recvAudioThrd);	//start listener
		regreq=1;
	}
	else
	{
		EnableWindow(hSessReq,FALSE);
		postReq(3,chatSocket);				//audio session request
		WaitForSingleObject(Permissions,INFINITE);	//wait for perms
		ResetEvent(Permissions);
		//Bring window to front
		AttachThreadInput( GetWindowThreadProcessId(GetForegroundWindow(),NULL),
						   GetCurrentThreadId(),
						   TRUE);

		SetForegroundWindow(hWnd);
		SetFocus(hWnd);
		Beep(500,400);
		MessageBox(hWnd,"Chat Alert", "Chat Alert", MB_OK|MB_ICONINFORMATION);
		EnableWindow(hRec,TRUE);


		//detach
		AttachThreadInput( GetWindowThreadProcessId(GetForegroundWindow(),NULL),
						   GetCurrentThreadId(),
						   FALSE);
	}
	return 0;
}

void InitDialog(HWND hwnd)
{


	DWORD recvAudioAddr,recvChatAddr;

	//copy handles and pointers for controls
	hWnd=hwnd;
	hDiag=GetDlgItem(hwnd,IDC_DIAG);			//diagnostics bar
	hStop=GetDlgItem(hwnd,IDC_STOP);			//stop button
	hRec=GetDlgItem(hwnd,IDC_RECORD);			//Record button
	hSessReq=GetDlgItem(hwnd,IDC_REQUEST);		//session request
	hChatRecv=GetDlgItem(hwnd,IDC_CRECV);		//chat recv edit
	hChatSend=GetDlgItem(hwnd,IDC_CSEND);		//chat send edit
	hProgress=GetDlgItem(hwnd,IDC_PROGRESS);	//progress bar
	hbSend=GetDlgItem(hwnd,IDC_BSEND);			//chat send button
	hTrack=GetDlgItem(hwnd, IDC_SLIDER1);
	hSaveCurr=GetDlgItem(hwnd,IDC_SAVECURR);
	hPlay=GetDlgItem(hwnd,IDC_PLAY);
	hSaveAll=GetDlgItem(hwnd,IDC_SAVEALL);
	hFileList=GetDlgItem(hwnd,IDC_FILELIST);
	hSpeaking=GetDlgItem(hwnd,IDC_SPEAK);

	//Create sockets
	Socket=GetHTTP("lab121machine33","/");
	recvSocket=GetHTTP("lab121machine33","/");
	chatSocket=GetHTTP("lab121machine33","/");

	crecDC=GetDC(hRec);
	recDC=CreateCompatibleDC(crecDC);
	GetClipBox(recDC,&clipBox;);

	SendMessage(hTrack, TBM_SETRANGE, (WPARAM) TRUE,(LPARAM) MAKELONG(iMin, iMax));
    SendMessage(hTrack, TBM_SETPOS,(WPARAM) TRUE,(LPARAM) 3);

	//W32Registry();


	recvAudioThrd = CreateThread(NULL,		  //launch thread to recv audio
								  0,
								  (LPTHREAD_START_ROUTINE)recvAudio,
								  NULL,
								  CREATE_SUSPENDED,
								  &recvAudioAddr;);

	chatListenThrd=CreateThread(NULL,		  //launch thread to recv audio
								0,
								(LPTHREAD_START_ROUTINE)chatTextRecv,
								NULL,
								0,
								&recvChatAddr;);

}

void registry(HWND hwnd)
{
	if(regreq==0)
	{
		DialogBox(savedInst,MAKEINTRESOURCE(IDD_DIALOG2),hwnd,UserNameProc);
		SetWindowText(hDiag,usrName);
	}
	else
		return;
}

BOOL CALLBACK UserNameProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	BOOL dRet=FALSE;
	switch(uMsg)
	{
		case WM_COMMAND:
			{
				switch(LOWORD(wParam))
				{

				   case IDOK:		{
										GetDlgItemText(hwnd,IDC_USERNAME,usrName,sizeof(usrName));
										dRet=TRUE;
										EndDialog(hwnd,IDCANCEL);
									}
									break;
				}
			}
			break;
	}
	return dRet;
}

BOOL CALLBACK RecordProg(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	BOOL dRet=FALSE;
	switch(uMsg)
	{
		case WM_INITDIALOG: {
								hRWnd=hwnd;
								hRecProg=GetDlgItem(hwnd,IDC_RECPROG);
								SendMessage(hRecProg,PBM_SETBARCOLOR,0,(LPARAM)RGB(255,0,0));
								SendMessage(hRecProg, PBM_SETRANGE, 0, MAKELPARAM(0, 29));
								SendMessage(hRecProg, PBM_SETSTEP, (WPARAM) 1, 0);
								break;
							}
	}

	return dRet;
}


DWORD WINAPI recvAudio()
{
	char buf[8192];
	int rec=0,total=0,count=0;
	BOOL runONCE=0;
	DWORD	err;
	DWORD	playBufAddr;

	SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 58));
    SendMessage(hProgress, PBM_SETSTEP, (WPARAM) 1, 0);
	WaveFormat=makeWaveFormat();
	//if ((err = waveOutOpen(&HWaveOut;, WAVE_MAPPER, &WaveFormat;, (DWORD)WaveOutProc, 0, CALLBACK_FUNCTION)))

OpenDevice:	//tag: device must be opened
	if ((err = waveOutOpen(&HWaveOut;, WAVE_MAPPER, &WaveFormat;, (DWORD)WaveOutProc, (DWORD)&adjustFreeBlocks;, CALLBACK_FUNCTION)))
	{
		sprintf(formatStr,"ERROR : Can't open Sound Device! Another program is using your sound card.Please close all programs accessing the soundcard and then restart", err);
		MessageBox(hWnd,formatStr,"Fatal Error",MB_OK|MB_ICONERROR);
		Sleep(7000);		//give user 7 seconds before second trial
		goto OpenDevice;
		//return 0;
	}
	waveBlocks = allocateBlocks(BLOCK_SIZE, BLOCK_COUNT);
    waveCurrentBlock = 0;

	SetWindowText(hDiag,"Listener Started");

	while(1)
	{
		nRet=recv(recvSocket,buf,sizeof(buf),0);
		if(nRet==SOCKET_ERROR)
		{
			MessageBox(hWnd,"Invalid recv audio socket","Fatal Error",MB_OKCANCEL|MB_ICONERROR);
			ExitProcess(1);
			return 1;
		}
		BufferAudio(HWaveOut,buf,nRet);
		if(runONCE==0)
		{
			playBuf=CreateThread( 0,
								  0,
								  (LPTHREAD_START_ROUTINE)playBuffer,
								  NULL,
								  CREATE_SUSPENDED,
								  &playBufAddr;);
			runONCE=1;
		}

		if(buffered>100000)			//buffered 100 KB of Audio Data
		{
			ResumeThread(playBuf);	//start playback
		}

		total+=nRet;

	}
}

void GetChatText()
{
	char formatted[1020];
	memset(sendtext,0,sizeof(sendtext));
	GetWindowText(hChatSend, sendtext, 1000);
	strcpy(formatted,"[USER: ");
	strcat(formatted,usrName);
	strcat(formatted,"]");
	strcat(formatted,sendtext);
	nRet=send(chatSocket,formatted,strlen(formatted),0);
	SetWindowText(hChatSend," ");							//clear
}

DWORD WINAPI chatTextRecv()
{
	int nRet=0;
	char *chk,*name;
	char recvbuf[1000];
	char recvd[2000];
	HDC hdc;

	strcpy(recvd,"Ready");
	hdc=GetDC(hChatRecv);

	while(1)
	{
		if(regreq==0)	//trap thread until socket init is complete
			continue;

		memset(recvbuf, 0, sizeof(recvbuf));
		nRet=recv(chatSocket,recvbuf,sizeof(recvbuf), 0);
		//MessageBox(hWnd, recvbuf,"Raw Server Msg <>", MB_OK|MB_ICONINFORMATION);
		chk=strstr(recvbuf,"SessionOwner");
		if(chk!=NULL)
		{
			name=strstr(chk,":");
			if(name!=NULL)
			{
				name++;
				SetWindowText(hSpeaking, name);
				continue;
			}

		}

        //******************
		chk=strstr(recvbuf,"AudioSessPerms");
		if(chk!=NULL)
		{
			SetEvent(Permissions);
			continue;
		}

		chk=strstr(recvbuf,"sessionInterrupt");
		if(chk!=NULL)
		{
			SetWindowText(hDiag,"STOPPED");
			SendMessage(hProgress, PBM_SETPOS,(WPARAM) 0, 0);
			continue;
		}

		chk=strstr(recvbuf,"Timeout");
		if(chk!=NULL)
		{
			MessageBox(hWnd,"Your Audio Chat session has expired","Timeout",MB_OK|MB_ICONERROR);
			EnableWindow(hRec, FALSE);
			timeoutFlag=1;
			stopRecord();
			EnableWindow(hSessReq, TRUE);
		}

		strcat(recvd,recvbuf);
		if(nRet!=SOCKET_ERROR)
		{
			SetWindowText(hChatRecv,recvd);
		}
	}
	return 0;
}

DWORD WINAPI playBuffer()
{
	int playindex=0,scount=1;

	while(playindex <= waveCurrentBlock)
	{
		if(playindex==(waveCurrentBlock-2))	//catching up? Slow down
		{
			Sleep(8000);
		}
		if(playindex==58)		//58 one transmission
		{
			//volatile
			PlayAudio(HWaveOut,playindex);

			if(recordAll==1)
				flashWrite();

			// end volatile
			if(recordAll==0)
				EnableWindow(hSaveCurr, TRUE);

			waveCurrentBlock=0;			//reset
			playindex=0;
			susp=1;
			buffered=0;					//fresh buffering
			SuspendThread(playBuf);		//leave the memory table
		}

		PlayAudio(HWaveOut,playindex);
		playindex++;

	}
	return 0;
}

void GetVolume()
{
	unsigned long volume=0;
	DWORD sliderPos;
	sliderPos=SendMessage(hTrack,TBM_GETPOS,0,0);

	switch((int)sliderPos)
	{
		case 1:	waveOutSetVolume(HWaveOut,(DWORD)2000UL);
				break;
		case 2:	waveOutSetVolume(HWaveOut,(DWORD)4000UL);
				break;
		case 3:	waveOutSetVolume(HWaveOut,(DWORD)6000UL);
				break;
		case 4:	waveOutSetVolume(HWaveOut,(DWORD)8000UL);
				break;
		case 5:	waveOutSetVolume(HWaveOut,(DWORD)0x4000A000UL);
				break;
		case 6:	waveOutSetVolume(HWaveOut,(DWORD)0x4000C000UL);
				break;
		case 7:	waveOutSetVolume(HWaveOut,(DWORD)0x4000E000UL);
				break;
		case 8:	waveOutSetVolume(HWaveOut,(DWORD)0x400010000UL);
				break;
	}
}

char* filename()
{
	sprintf(fileName,"audiofile%d.dat",filecount);
	filecount++;
	return fileName;
}

void flashWrite()
{
	WAVEHDR* current;
	DWORD written;
	BOOL ok;
	char *filedesc;
	int windex=0,playindex=0;

	EnableWindow(hSessReq, FALSE);
	EnableWindow(hSaveCurr, FALSE);
	filedesc = filename();
	WaveFileHandle = CreateFile(filedesc, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if(WaveFileHandle==NULL)
		MessageBox(hWnd,"Unable to create file","Fatal Error",MB_OK|MB_ICONERROR);

	while(windex<=58)
	{
		current = &waveBlocks;[windex];
		ok=WriteFile(WaveFileHandle, current->lpData, 8192,  &written;, NULL);
		if(ok==0)
		{
			int err=GetLastError();
		}

		windex++;
	}
	CloseHandle(WaveFileHandle);
	playindex=SendMessage(hFileList,CB_ADDSTRING,0,(LPARAM)filedesc);
	SendMessage(hFileList,CB_SETITEMDATA,(WPARAM)playindex,(LPARAM)(DWORD)filedesc);
	EnableWindow(hSessReq, TRUE);
}

void PlayFile()
{
	int selindex=0,fplay=0,it=0;
	char *afile;
	HANDLE playFile;
	char buffer[8192];

	if(waveCurrentBlock!=0)
	{
		MessageBox(hWnd,"Audio Buffer is not currently available for file playback","Alert",MB_OK | MB_ICONINFORMATION);
		return;
	}

	selindex = SendMessage(hFileList,CB_GETCURSEL, 0, 0);
	if(selindex < 0)
	{
		MessageBox(hWnd,"No File Selected","Error", MB_OK | MB_ICONERROR);
		return;
	}
	afile = (char *)SendMessage(hFileList, CB_GETITEMDATA,(WPARAM)selindex, 0);
	playFile=CreateFile((LPCTSTR)afile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(playFile==INVALID_HANDLE_VALUE)
	{
		char msg[15];
		sprintf(msg,"File Error: %d", GetLastError());
		MessageBox(hWnd,msg,"Error", MB_OK | MB_ICONINFORMATION);
	}
	while(1)
	{
        DWORD readBytes;

        if(!ReadFile(playFile, buffer, sizeof(buffer), &readBytes;, NULL))
            break;
        if(readBytes == 0)
            break;

        if(readBytes < sizeof(buffer))
		{
            memset(buffer + readBytes, 0, sizeof(buffer) - readBytes);
        }

		BufferAudio(HWaveOut,buffer,sizeof(buffer));
	}

	fplay=waveCurrentBlock;
	waveCurrentBlock=0;

	for(it=0;it<=fplay;it++)
		PlayAudio(HWaveOut,it);

	fplay=0;
}

void SaveAll()
{
	if(waveCurrentBlock!=0)
	{
		MessageBox( hWnd,"Illegal Operation: File write operation can not be started in the middle of a streaming operation",
					"Illegal Operation",MB_OK | MB_ICONERROR);
		return;
	}

	if(recordAll==0)
	{
		SetWindowText(hSaveAll,"Save All*");
		recordAll=1;
		return;
	}

	if(recordAll==1)
	{
		SetWindowText(hSaveAll,"Save All");
		recordAll=0;
		return;
	}
}

void W32Registry()
{
	HKEY hKey;
	LONG regcheck;
	DWORD namesz,dwType;
	char retUsername[20];
	char regUsername[]="ScreenName";
	char regUserno[]="ScreenNo";
	char regpath[]="Environment\\";

	if(reg==TRUE)					//registry already read
		return;

	regcheck=RegOpenKeyEx(HKEY_CURRENT_USER, regpath , 0, KEY_ALL_ACCESS, &hKey;);
	if(regcheck!=ERROR_SUCCESS)
	{
		MessageBox(hWnd,"Error reading Registry.Please supply valid user name","Error",MB_OK|MB_ICONEXCLAMATION);
		registry(hWnd);
	}
	else
	{
		if((RegQueryValueEx(hKey,regUsername,0, &dwType;, (BYTE *)&retUsername;,&namesz;))!=ERROR_SUCCESS)
		{
			registry(hWnd);
			RegSetValueEx(hKey,(CONST CHAR*)regUsername,0,REG_SZ,(CONST BYTE*)usrName,strlen(usrName));
		}
		else
		{
			strcpy(usrName,retUsername);
			reg=TRUE;
		}
	}
}
