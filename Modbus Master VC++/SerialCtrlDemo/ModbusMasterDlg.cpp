/*  SerialCtrlDemoDlg.cpp : implementation file
 * 
 *	Baudrate bug fix: set to default 19200 in SerialCtrl::OpenPort()
 *	void CSerialCtrlDemoDlg::OnBnClickedButtonWr() add '\r'
 *
 *	12-21-18: Woburn: Removed all Run thread code and CSerialIO class.
 *	12-21-18: Sending and Receiving works great.
 *	12-23-18: Ann Arbor: WAM Modbus missing DLL, so I removed it entirely.
 *	12-24-18: Ann Arbor: 
 *	12-28-18  Woburn: Up to date version with tested Modbus communication
 *				Two way Modbus communication working.
 *  12-29-18: Warwick: 
 *  01-01-19: Warwick: Got Modbus read/write working in ModbusMaster object.
 *	01-02-18: Warwick: Modified OpenPort() to accept baudrate input arg.
 *				
 */

#include "stdafx.h"
#include "SerialCtrlDemo.h"
#include "SerialCtrlDemoDlg.h"
#include "SerialCtrl.h"
#include "ModbusMaster.h"
#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

int FindComPorts(CString *ptrComPortNames);

CSerialCtrlDemoDlg *ptrDialog;
CString arrSerialPorts[255];

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define	IDT_TIMER_0	WM_USER + 200
#define TIMER_INTERVAL 100  // Main process loop timer interval in milliseconds
 // Actually, it's more like milliseconds x 2
 // So an interval value of 100 corresponds to about 200 milliseconds,
 // or two tenths of a second.

// SerialCtrl ComTest;
int seconds = 0;
bool RunFlag = false;
CString strModbusRx = "";

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

CSerialCtrlDemoDlg::CSerialCtrlDemoDlg(CWnd* pParent /*=NULL*/): CDialog(CSerialCtrlDemoDlg::IDD, pParent), bPortOpened(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CSerialCtrlDemoDlg::~CSerialCtrlDemoDlg()
{
	// ComTest.ClosePort();
	MM.ClosePort();
}


void CSerialCtrlDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_OPEN, m_btnOpen);
	DDX_Control(pDX, IDC_BUTTON_START, m_ButtonStart);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_ButtonStop);
	DDX_Control(pDX, IDC_EDIT_WRITE, m_editWrite);
	DDX_Control(pDX, IDC_EDIT_READ, m_EditRead);
	DDX_Control(pDX, IDC_EDIT_STATUS, m_EditStatus);
	DDX_Control(pDX, IDC_STATIC_INFO, m_staticInfo);
}

BEGIN_MESSAGE_MAP(CSerialCtrlDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_START, &CSerialCtrlDemoDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CSerialCtrlDemoDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CSerialCtrlDemoDlg::OnBnClickedButtonOpen)
	// ON_CBN_SELCHANGE(IDC_COMBO_SN, &CSerialCtrlDemoDlg::OnCbnSelchangeComboSn)
	ON_BN_CLICKED(IDC_BUTTON_WR, &CSerialCtrlDemoDlg::OnBnClickedButtonWr)
END_MESSAGE_MAP()



void CSerialCtrlDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSerialCtrlDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSerialCtrlDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CSerialCtrlDemoDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	//__super::OnOK();
}



UINT CSerialCtrlDemoDlg::StartTimer(UINT TimerDuration)
{
	UINT TimerVal;
	TimerVal = (UINT)SetTimer(IDT_TIMER_0, TimerDuration, NULL);
	if (TimerVal == 0)
		MessageBox("Unable to obtain timer", "SYSTEM ERROR", MB_OK | MB_SYSTEMMODAL);
	return TimerVal;
}// end StartTimer

bool CSerialCtrlDemoDlg::StopTimer(UINT TimerVal)
{
	if (!KillTimer(TimerVal)) return false;
	else return true;
} // end StopTimer

void CSerialCtrlDemoDlg::OnBnClickedButtonStart()
{
	// TODO: Add your control notification handler code here
	StartTimer(TIMER_INTERVAL);
	RunFlag = true;
}

void CSerialCtrlDemoDlg::OnBnClickedButtonStop()
{
	// TODO: Add your control notification handler code here
	RunFlag = false;
	StopTimer(TIMER_INTERVAL);
}

void CSerialCtrlDemoDlg::OnBnClickedButtonOpen()
{

	if (!MM.GetPortStatus())
	// if (!ComTest.GetPortStatus())
	{
		// if (ComTest.OpenPort(configSerial, "COM6"))
		if (MM.OpenPort("COM6", 19200))
		{
			m_EditStatus.SetWindowText("COM 6 Opened OK!");
			m_btnOpen.SetWindowTextA("CLOSE");
		}
		else
		{
			m_EditStatus.SetWindowText("Error: could not open COM 6");
			m_btnOpen.SetWindowTextA("OPEN");
		}
	}
	else
	{
		// ComTest.ClosePort();
		MM.ClosePort();
		m_btnOpen.SetWindowTextA("OPEN");
		m_EditStatus.SetWindowText("COM 6 Closed");
	}
}


void CSerialCtrlDemoDlg::OnTimer(UINT_PTR TimerVal)
{
	static int tenthSeconds = 0, seconds = 0;
	CString strTimer;
	if (!KillTimer(TimerVal))
	{
		;
	}
	tenthSeconds++;
	if ((tenthSeconds % 10) == 0)
	{
		seconds++;
		strTimer.Format(_T("Time: %d seconds"), seconds);
	}
	TestHandler();
	if (RunFlag) StartTimer(TIMER_INTERVAL);
}





// CSerialCtrlDemoDlg message handlers
BOOL CSerialCtrlDemoDlg::OnInitDialog()
{
	ptrDialog = this;	

	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon


	// GetOpenFileNameA((LPOPENFILENAMEA) "TestFile.txt");
	/*
	char filename[MAX_PATH];

	OPENFILENAME ofn;
	ZeroMemory(&filename, sizeof(filename));
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
	// ofn.lpstrFilter = "Text Files\0*.txt\0Any File\0*.*\0";
	ofn.lpstrFilter = "Comma delimited (CSV)\0*.csv\0Any File\0*.*\0";
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = "Select a File, yo!";
	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

	int numPorts = FindComPorts(arrSerialPorts);

	if (GetSaveFileNameA(&ofn))
	{
		std::cout << "You are saving the file \"" << filename << "\"\n";
		CString CSfilename = filename;
		CSfilename = CSfilename + ".csv";
	}

	CString StartupFilename = "Startup.txt";
	if (GetFileAttributes(StartupFilename) == INVALID_FILE_ATTRIBUTES);
	*/
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL CSerialCtrlDemoDlg::TestHandler()
{	
	unsigned long InPacketLength = 0;

	static int timerCounter = 0;
	timerCounter++;

	/*
	if (ComTest.GetPortStatus())
	{
		ComTest.Read(strInPacket, MAX_MODBUS_LENGTH, InPacketLength);
		if (InPacketLength > 0)
		{
			CString CSpacket(strInPacket);
			strModbusRx = strModbusRx + strInPacket;
			m_EditRead.SetWindowText(strModbusRx);
		}
	}
	*/
	if (timerCounter > 5)
	{
		timerCounter = 0;
		seconds++;
		CString strTime;
		strTime.Format("Time: %d seconds", seconds);
		m_EditStatus.SetWindowText(strTime);
	}
	return true;
}





int CSerialCtrlDemoDlg::FindComPorts(CString *ptrComPortNames)
{

	TCHAR lpTargetPath[5000]; // buffer to store the path of the COMPORTS
	DWORD test;
	int portNameIndex = 0;

	for (portNameIndex = 0; portNameIndex < 255; portNameIndex++)
		ptrComPortNames[portNameIndex].Format("");

	portNameIndex = 0;
	for (int i = 0; i < 255; i++) // checking ports from COM0 to COM255
	{
		ptrComPortNames[portNameIndex].Format("COM%d", i);

		test = QueryDosDevice(ptrComPortNames[portNameIndex], (LPSTR)lpTargetPath, 5000);
		if (test != 0) portNameIndex++;

		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			lpTargetPath[10000]; // in case the buffer got filled, increase size of the buffer.
			continue;
		}
	}
	return portNameIndex;
}


#define MAXFILENAME 16

int CSerialCtrlDemoDlg::LoadExcelFilenames(char *ptrPrimary, char *ptrSecondary)
{
	std::string strLine;
	CString INIoutFileName;
	char *ptrLine = NULL, ch;
	char filename[MAXFILENAME];
	int numFilenames = 0, i = 0;

	if (INIoutFileName == "") return 0;  // Normally this should never happen!

	ptrSecondary[0] = '\0';

	std::ifstream inFile(INIoutFileName);

	if (inFile)
	{
		numFilenames = 0;
		while (std::getline(inFile, strLine) && numFilenames < 2)
		{
			ptrLine = _strdup(strLine.c_str());
			if (ptrLine != NULL)
			{
				for (i = 0; i < MAXFILENAME; i++)
				{
					ch = ptrLine[i];
					if (ch == '\0' || ch == '\r' || ch == '\n')
					{
						filename[i] = '\0';
						break;
					}
					filename[i] = ch;
					if (ptrLine[i] == '.' && ptrLine[i + 1] == 'x' && ptrLine[i + 2] == 'l' && ptrLine[i + 3] == 's')
					{
						filename[i + 1] = 'x';
						filename[i + 2] = 'l';
						filename[i + 3] = 's';
						filename[i + 4] = '\0';
						numFilenames++;
						if (numFilenames == 1) strcpy_s(ptrPrimary, MAXFILENAME, filename);
						else if (numFilenames == 2) strcpy_s(ptrSecondary, MAXFILENAME, filename);
						break;
					}
				}
				std::free(ptrLine);
			}
		}
		inFile.close();
	}
	else return(0);
	return numFilenames;
}

void CSerialCtrlDemoDlg::OnBnClickedButtonWr()
{
	static int TrialCounter = 0;		
	CString strResult;
	static UINT16 OutData[10] = { 11111,2222,3333,4444,5577,6688,9777,888,999,1212 };
	UINT16 InData[10];

	// TODO: Add your control notification handler code here	
	if (MM.GetPortStatus())
	{
		CString strErrors;
		int Result = MM.ReadWriteModbusDevice(0x07, 100, 10, OutData, 100, 10, InData, &strErrors);
		if (Result == SYSTEM_ERROR)
			m_EditStatus.SetWindowText("Modbus COM error");
		else
		{
			strResult.Format("#%d RX: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d", TrialCounter++, InData[0], InData[1], InData[2], InData[3], InData[4], InData[5], InData[6], InData[7], InData[8], InData[9]);
			m_EditStatus.SetWindowText(strResult);
		}
	}
	else m_EditStatus.SetWindowText("Port not open");
	for (int i = 0; i < 10; i++) OutData[i] = OutData[i] + 1;
}


/*
BOOL CSerialCtrlDemoDlg::storeStartupInfo(CString ptrStartupFilename, CString *ptrComPortNames)
{
	std::ofstream myfile;

	if (ptrStartupFilename == "") return FALSE;

	myfile.open(ptrStartupFilename);
	// myfile << ptrPrimaryFilename << "\r\n";

	myfile.close();
	return TRUE;
}
*/