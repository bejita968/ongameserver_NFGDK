/** 
 *  @file		NaveNet.h
 *  @brief		Network Base Ŭ����
 *  @remarks	
 *  @author		������(edith2580@gmail.com)
 *  @date		2009-04-02
 */

#pragma once

/// ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
#define WIN32_LEAN_AND_MEAN		

// Windows ��� �����Դϴ�.
#include <windows.h>
#include <time.h>												// timer 
#include <assert.h>												// assert
#include <process.h>											// Thread 
#include <stdio.h>												// standard I/O
#include <stdlib.h>

// sock
#include <winsock2.h>											// win32 socket 
#pragma comment(lib,"ws2_32.lib")								

#include <Mswsock.h>											// extension socket library 
#pragma comment(lib,"mswsock.lib")

#include "mmsystem.h"											// ��Ƽ�̵�� Ÿ�̸Ӹ� ����
#pragma comment(lib,"winmm.lib")

#include "NFPacket.h"
#include <Nave/NFSync.h>

namespace NaveNet { 

	#define MAX_PACKET				512					/// �ִ� Packet ���� ����

	#define WM_CONNECT				WM_APP + 0x1001		/// CONNECT �޽���
	#define WM_RECV_MSG				WM_APP + 0x1002		/// RECV �޽���
	#define WM_SOCK_CLOSE			WM_APP + 0x1003		/// CLOSE �޽���

	/** 
	*  @class        NFPacketQueue
	*  @brief        Packet Queue ó���� Ŭ����
	*  @remarks      
	*                
	*  @par          
	*  @author  Edith
	*  @date    2009-04-04
	*/
	class NFPacketQueue
	{
		/// Queue ���� ����� INDEX ����ü
		struct INDEX
		{
			/// ���� �ε��� ��ġ
			INDEX*		pNext;
			/// ��Ŷ 
			NFPacket	Packet;

			INDEX()
			{
				pNext = NULL;
				Packet.Init();
			}
		};

	public:
		/// NFPacketQueue ������
		NFPacketQueue()
		{
			pList = NULL;
			pHead = NULL;
			pTail = NULL;
			nQueueCnt = 0;
			nMaxQueueCnt = 0;
		}

		/// NFPacketQueue �Ҹ���
		~NFPacketQueue()
		{
			Release();
		}

		/// Queue�� �ʱ�ȭ �մϴ�.
		VOID Release()
		{
			Nave::NFSyncLock Sync(&nfSync);

			if(pList)
			{
				delete [] pList;
				pList = NULL;
			}
		}

		/// ��Ŷ ť�� �����մϴ�.
		BOOL Create(int nMAXCnt = MAX_PACKET)
		{
			Nave::NFSyncLock Sync(&nfSync);

			if(nMAXCnt <= 0) return FALSE;

			if(pList)
			{
				delete [] pList;
				pList = NULL;
			}

			nMaxQueueCnt = nMAXCnt;

			if((pList = new INDEX[nMaxQueueCnt]) == NULL)
			{
				nMaxQueueCnt = 0;
				return FALSE;
			}

			for(int i = nMaxQueueCnt - 1; i >= 0 ; i--)
			{
				if( (i+1) == nMaxQueueCnt)
				{
					pList[i].pNext = &pList[0];
					continue;
				}
				pList[i].pNext = &pList[i+1];		
			}
			pHead = pTail = &pList[0];
			return TRUE;
		}

		/// ť�� ������ ���ɴϴ�.
		int GetQueCnt()
		{
			Nave::NFSyncLock Sync(&nfSync);
			return nQueueCnt;
		}

		/// ť�� �����մϴ�.
		void Reset()
		{
			Nave::NFSyncLock Sync(&nfSync);
			pHead = pTail = &pList[0];
			nQueueCnt = 0;
		}

		/// ��Ŷ�� �߰��մϴ�.
		BOOL Add(NFPacket& Packet)
		{
			Nave::NFSyncLock Sync(&nfSync);

			if(!pList) return FALSE;
			if(nQueueCnt == nMaxQueueCnt)
			{
				nQueueCnt = 0;
				pHead = pTail = &pList[0];
				return FALSE;
			}
			//if(szData[0] == NULL) return FALSE;
			if(Packet.m_Header.Size <= 0) return FALSE;
			if(Packet.m_Header.Size >= DEF_MAXPACKETSIZE) return FALSE;

			// Head�� Size�� Header ������ ������ �������.
			int PacketSize = Packet.m_Header.Size-sizeof(Packet.m_Header);

			// ���缭 ����.. 
			CopyMemory(&pTail->Packet.m_Header, &Packet.m_Header, sizeof(Packet.m_Header));
			CopyMemory(pTail->Packet.m_Packet,Packet.m_Packet,PacketSize);
			pTail->Packet.m_Packet[PacketSize] = 0;


			pTail = pTail->pNext;

			InterlockedIncrement((LONG*)&nQueueCnt);

			return TRUE;
		}

		/**
		* @brief	��Ŷ ������ ���ɴϴ�.
		* @param Packet ��Ŷ ������
		* @return	��Ŷ�� ������
		*/
		int GetPnt(NFPacket** Packet)
		{
			Nave::NFSyncLock Sync(&nfSync);

			if(!pList) return -1;
			if(nQueueCnt == 0) return -1;

			int nLen = -1;
			*Packet = &pHead->Packet;
			nLen = pHead->Packet.m_Header.Size;

			return nLen;
		}

		/// ù���� ��Ŷ�� �����մϴ�.
		void Del()
		{
			Nave::NFSyncLock Sync(&nfSync);

			if(!pList) return;
			if(nQueueCnt == 0) return;

			int nLen = -1;

			pHead->Packet.Init();
			pHead = pHead->pNext;

			InterlockedDecrement((LONG*)&nQueueCnt);
		}

		/**
		* @brief	��Ŷ ������ ���ɴϴ�.
		* @param Packet ��Ŷ ������
		* @return	��Ŷ ������
		*/
		int Get(NFPacket& Packet)
		{
			Nave::NFSyncLock Sync(&nfSync);

			if(!pList) return -1;
			if(nQueueCnt == 0) return -1;

			int nLen = -1;
			int PacketSize = pHead->Packet.m_Header.Size-sizeof(pHead->Packet.m_Header);

			CopyMemory(&Packet.m_Header, &pHead->Packet.m_Header, sizeof(Packet.m_Header));
			CopyMemory(Packet.m_Packet, pHead->Packet.m_Packet, PacketSize);
			Packet.m_Packet[PacketSize] = 0;

			nLen = Packet.m_Header.Size;

			pHead->Packet.Init();
			pHead = pHead->pNext;

			InterlockedDecrement((LONG*)&nQueueCnt);

			return nLen;
		}

	private:
		/// ��Ŷ�� ����Ʈ
		INDEX*			pList;
		/// ��Ŷ ����Ʈ�� ����
		INDEX*			pHead;
		/// ��Ŷ ����Ʈ�� ��
		INDEX*			pTail;

		/// ���� ���Ǵ� ť�� ����
		int				nQueueCnt;
		/// �ִ� ť�� ����
		int				nMaxQueueCnt;
		/// ����ȭ ����
		Nave::NFSync	nfSync;
	};

	/// Socket�� Port�����´�.
	int SockRemotePort( SOCKET Sock );
	/// Socket�� IP�����´�.
	int SockRemoteIP( SOCKET Sock, unsigned char ip[4] );
	/// Domain�̸����� ip�� ���ɴϴ�.
	BOOL GetHostIPByDomain(IN_ADDR &Addr, const char *szDomain);

}