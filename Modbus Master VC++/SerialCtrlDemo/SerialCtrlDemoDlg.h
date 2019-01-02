// SerialCtrlDemoDlg.h : header file
//

#pragma once
#include "SerialCtrl.h"
#include "afxwin.h"
#include "ModbusMaster.h"

class CSerialCtrlDemoDlg : public CDialog
{
// Construction
public:
	CSerialCtrlDemoDlg(CWnd* pParent = NULL);	// standard constructor
	~CSerialCtrlDemoDlg(); // Destructor

	ModbusMaster MM;	

// Dialog Data
	enum { IDD = IDD_SERIALCTRLDEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
		

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	BOOL TestHandler();
	bool StopTimer(UINT TimerVal);
	UINT StartTimer(UINT TimerDuration);
	void OnTimer(UINT_PTR TimerVal);
	CButton m_btnOpen, m_ButtonStart, m_ButtonStop;
	CEdit m_editWrite, m_EditRead, m_EditStatus;
	int LoadExcelFilenames(char *ptrPrimary, char *ptrSecondary);
	int FindComPorts(CString *ptrComPortNames);
	BOOL storeStartupInfo(CString ptrFilename);

	CStatic m_staticInfo;
	BOOL bPortOpened;	
protected:
	virtual void OnOK();
public:	
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnCbnSelchangeComboSn();
	afx_msg void OnBnClickedButtonWr();
};
