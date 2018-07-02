//AUTHOR ROBERT GITHINJI
//INSTITUTE OF COMPUTER SCIENCE, UNIVERSITY OF NAIROBI
//Visual C++ 6

//HEADER FILE variables.c
//Start of the very chaotic header file "variables.h"

/*****************************************************************************/

#include <string.h>
#include <iostream.h>
#include <iomanip.h>

#include <windows.h;>
#include <commctrl.h;>
#include <stdlib.h;>
#include <winbase.h;>
#include <conio.h;>
#include <stdio.h;>
#include <mmsystem.h;>
#include <winsock.h;>

#define BLOCK_SIZE  8192
#define BLOCK_COUNT 60
#define ERRMSG_PRINT	0		//mute error values
#define ERRMSG_NONE		1


//------------METHOD PROTOTYPES---------------
//-------------gui-----------------------

BOOL CALLBACK MainWndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
void InitDialog(HWND hwnd);
BOOL CALLBACK UserNameProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK RecordProg(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);


//-------------  tcp/ip ----------------------

SOCKET GetHTTP(LPCSTR lpServerName, LPCSTR lpFileName);
void postReq();
int SockRead(int Sock,char *Buff,int Len);
int SockWrite(int Sock,char *Buff,int Len);

//------------ multimedia --------------------

WAVEFORMATEX makeWaveFormat();
void PlayWave();
void PrintWaveErrorMsg(DWORD err, TCHAR * str);
long QueueWaveData(WAVEHDR * waveHeader);
void set_mute(MIXERLINE *mixerLine, DWORD val, BOOL errmsg);
DWORD WINAPI waveInProc(LPVOID arg);
void CALLBACK WaveOutProc(HWAVEOUT waveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
void registry(HWND hwnd);
DWORD WINAPI Request();
void Record();
void stopRecord();
DWORD WINAPI recvAudio();
WAVEHDR* allocateBlocks(int size, int count);
void freeBlocks(WAVEHDR* blockArray);
void writeAudio(HWAVEOUT hWaveOut, LPSTR data, int size);
void BufferAudio(HWAVEOUT hWaveOut, LPSTR data, int size);
void PlayAudio(HWAVEOUT hWaveOut,int playindex);
DWORD WINAPI playBuffer();
void GetVolume();
void flashWrite();

//----------- net chat --------------------
DWORD WINAPI chatTextRecv();
void GetChatText();

//end prototypes

//---------------VARIABLES-------------------

BOOL regreq=0, played=0, susp=0, reg=FALSE,first=0;
HANDLE recvAudioThrd,chatListenThrd,hFile,playBuf;
int sockRead=0;
char sendtext[1000];
int playbuf=16000;
static volatile int freeBlocksCount;
DWORD reqBuffer;

int buffered=0;
UINT iMin=0,iMax=7;
CRITICAL_SECTION adjustFreeBlocks;



/* RECORDING  /**/

HWAVEIN	WaveInHandle;	// Handle to the WAVE In Device
HMIXER	MixerHandle;	//Handle to Mixer for WAVE In Device
HANDLE	WaveFileHandle = INVALID_HANDLE_VALUE;	//Handle to the disk file
WAVEHDR	WaveHeader[2];	//2 WAVEHDR's for recording ie double-buffering
BOOL InRecord = FALSE;	// ? recording
BOOL started = FALSE;
unsigned char DoneAll;

//PLAYBACK
WAVEFORMATEX WaveFormat;		//WAVEFORMATEX structure for reading in the WAVE fmt chunk
HMMIO		HMmio;				//file handle for open file
MMCKINFO    MmckinfoSubchunk;	// subchunk information structure for reading WAVE file
DWORD		WaveDataPlayed;		//Count of waveform data still to play
HWAVEOUT	HWaveOut;			// Handle of opened WAVE Out device
DWORD		WaveBufSize;		//The size of each WAVEHDR's wave buffer
WAVEHDR		WaveHeader[2];		//2 WAVEHDR structures (for double-buffered playback of WAVE)
WAVEHDR *	LastHdr;			//Ptr to last WAVEHDR played

/* NET /**/
int count=0,wcount=0;
SOCKET	Socket,recvSocket,chatSocket;
int nRet;
LPSERVENT lpServEnt;
SOCKADDR_IN saServer;
char szBuffer[1024];
char request[1024],rqs[1024];
int len;
char Path[30]="/cgi-bin/LiveAudio";
char sstr[30]="Test Message";

HWND hWnd,hDiag,hChatRecv,hProgress,hRec,hChatSend,hSessReq,hStop,hbSend,hTrack,hPlay,hSaveCurr,hSaveAll,hFileList,hRecProg,hSpeaking;
char formatStr[200];
char usrName[30];
HINSTANCE savedInst;


static CRITICAL_SECTION waveCriticalSection;
static WAVEHDR*         waveBlocks;
static volatile int     waveFreeBlockCount;
static int              waveCurrentBlock;


/*********************************************************************************************/

//END OF HEADER FILE


/*****************************************************************************/
