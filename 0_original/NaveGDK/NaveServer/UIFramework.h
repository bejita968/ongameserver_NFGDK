/** 
 *  @file  		UIFramework.h
 *  @brief 		Server의 화면인터페이스를 구현하는 UIFramework 객체
 *  @remarks 
 *  @author  	강동명(edith2580@gmail.com)
 *  @date  		2009-05-09
 */
#pragma once

#include "UICmdEdit.h"
#include "UICmdMsgView.h"
#include <vector>

namespace NaveServer {

	/// 새로운 커맨드를 추가합니다.
	/// ADD_COMMAND(_T("help"), CCmdHelp, L"화면에 커멘드리스트를 보여줍니다.");
	#define ADD_COMMAND(cmd, object, msg) AddCommand(cmd, new object, msg)

	/** 
	 *  @class        UICommand
	 *  @brief        어플리케이션에서 커맨드를 작성할때 사용할 커멘드 객체
	 *  @remarks      
	 *                
	 *  @par          
	 *  @author  Edith
	 *  @date    2009-05-09
	 */
	class UICommand
	{
	public:
		virtual BOOL DoProcess(WCHAR* lpParam) = 0;    
	};

	/** 
	 *  @class        UICommandFactory
	 *  @brief        커맨드를 관리하는 Factory 객체 Server의 Server의 UI객체가 상속받는다.
	 *  @remarks      
	 *                
	 *  @par          
	 *  @author  Edith
	 *  @date    2009-05-09
	 */
	class UICommandFactory
	{
	public:
	    BOOL AddCommand(const WCHAR* szCommand, UICommand* lpCommand, const WCHAR* szMessage)
		{
			if(0 == szCommand, 0 == lpCommand)
				return FALSE;    

			m_CMDVector.push_back(COMMAND(szCommand, lpCommand, szMessage));
			return TRUE;
		}

		VOID ReleaseCommand()
		{
			int iSize = m_CMDVector.size();
			for(int i = 0; i < iSize; ++i)
			{
				delete m_CMDVector[i].m_lpCMD;
			}

			m_CMDVector.clear();
		}

		virtual VOID	InitializeCommand() {};

		virtual VOID	StartCommand() {}; 							// UIConsol형식 Command 입력
		virtual VOID	EndCommand() {};

		VOID ShowCommand()
		{
			LOG_IMPORTANT( (L"------------------- Commands -------------------") );
			int iSize = m_CMDVector.size();
			for(int i = 0; i < iSize; ++i)
			{
				LOG_IMPORTANT((L"%s : %s", m_CMDVector[i].m_szCMD, m_CMDVector[i].m_szMSG));
			}
			LOG_IMPORTANT( (L"----------------- Commands End -----------------") );
		}

		VOID DoCommand(WCHAR* Command)
		{
			WCHAR Buff[DEF_BUFSIZE];
			ZeroMemory(Buff, DEF_BUFSIZE);
			wcscpy(Buff, Command);

			WCHAR* cmd;
			cmd = wcstok(Command, L" ");
			if(cmd)
			{
				Nave::uint32 uiHash = Nave::Hash(cmd);
				WCHAR* NextCmd = &Buff[wcslen(cmd)+1];

				int iSize = m_CMDVector.size();
				for(int i = 0; i < iSize; ++i)
				{
					if(m_CMDVector[i].m_uiHash != uiHash)
						continue;

					m_CMDVector[i].m_lpCMD->DoProcess(NextCmd);
					return;
				}
			}
		}

	public:
		UICommandFactory()
		{
		}
		~UICommandFactory()
		{
			ReleaseCommand();
		}

	private:
		struct COMMAND
		{
			Nave::uint32	m_uiHash;
			WCHAR			m_szCMD[32];
			WCHAR			m_szMSG[128];
			UICommand*		m_lpCMD;

			COMMAND(const WCHAR* szCommand, UICommand* lpCMD, const WCHAR* szMessage)
			{
				m_uiHash = Nave::Hash(szCommand);

				wcscpy(m_szCMD, szCommand);

				if(szMessage)
					wcscpy(m_szMSG, szMessage);
				else
					wcscpy(m_szMSG, L"No Help Message");

				m_lpCMD = lpCMD;
			}
		};

		typedef std::vector<COMMAND>	CMDVector;
		CMDVector						m_CMDVector;
	};

	/** 
	 *  @class        UIConsol
	 *  @brief		  콘솔 UI
	 *  @remarks      
	 *                
	 *  @par          
	 *  @author  Edith
	 *  @date    2009-05-09
	 */
	class UIConsol : public UICommandFactory
	{
		BOOL			m_bCommandExit;
	public:
		// 사용자 오브젝트를 초기화 합니다. (윈도우설정이 종료됀 후에 호출)
		virtual VOID	InitObject() {};
		// 사용자 오브젝트를 삭제합니다. (메인루프가 끊난후(EndProcess호출후) 호출)
		virtual VOID	ReleaseObject() {};

		BOOL			Init();

		virtual VOID	InitializeCommand() {};
		virtual VOID	StartCommand();							// UIConsol형식 Command 입력
		virtual VOID	EndCommand();

	public:
		UIConsol(VOID);
		~UIConsol(VOID);
	};

	/** 
	 *  @class        UIWindow
	 *  @brief        윈도우 UI
	 *  @remarks      
	 *                
	 *  @par          
	 *  @author  Edith
	 *  @date    2009-05-09
	 */
	class UIWindow : public UICommandFactory
	{
	public:
		HWND			GetWnd() { return m_hMainWnd; }
		BOOL			IsExit() { return m_bCommandExit; }
		// 사용자 오브젝트를 초기화 합니다. (윈도우설정이 종료됀 후에 호출)
		virtual VOID	InitObject() {};
		// 사용자 오브젝트를 삭제합니다. (메인루프가 끊난후(EndProcess호출후) 호출)
		virtual VOID	ReleaseObject() {};

		virtual VOID	ShowWindow(int nCmdShow);

		// Windows MsgProc
		virtual LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

		BOOL			Init(HINSTANCE hInstance, int nCmdShow, int iWidth, int iHeight, WCHAR* AppName, WCHAR* Icon);

		virtual VOID	InitializeCommand() {};
		virtual VOID	StartCommand();							// UIConsol형식 Command 입력
		virtual VOID	EndCommand();

	public:
		UIWindow(VOID);
		~UIWindow(VOID);

	private:
		HWND			m_hMainWnd;
		BOOL			m_bCommandExit;
		HFONT			m_hFont;

		VOID			ResizeWindows( VOID );

		// WM_EDIT_RETURN
		virtual VOID	OnEditReturn(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		// WM_COMMAND
		virtual VOID	OnCommand(HWND hWnd, INT nID, INT iEvent, LPARAM lParam) { return; }
	};

}