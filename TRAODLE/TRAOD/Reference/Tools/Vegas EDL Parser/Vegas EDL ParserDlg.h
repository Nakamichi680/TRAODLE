// Vegas EDL ParserDlg.h : header file
//
//{{AFX_INCLUDES()
#include "msflexgrid.h"
//}}AFX_INCLUDES

#if !defined(AFX_VEGASEDLPARSERDLG_H__8696495D_F1ED_4CA7_AD03_C70FF35DA0CC__INCLUDED_)
#define AFX_VEGASEDLPARSERDLG_H__8696495D_F1ED_4CA7_AD03_C70FF35DA0CC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CVegasEDLParserDlg dialog

#include "EDLParser.h"

#include "gdbserver/gdbglobal.h"

class CVegasEDLParserDlg : public CDialog
{
// Construction
public:
	void PopulateTable();
	CVegasEDLParserDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CVegasEDLParserDlg)
	enum { IDD = IDD_VEGASEDLPARSER_DIALOG };
	CButton	m_Merge;
	CButton	m_Export;
	CEdit	m_Text;
	CEdit	m_StartTime;
	CEdit	m_SpeechID;
	CEdit	m_Length;
	CEdit	m_DBKey;
	CButton	m_HideFiltered;
	CEdit	m_StartOffset;
	CEdit	m_EndOffset;
	CButton	m_StopButton;
	CButton	m_TestButton;
	CStatic	m_Time;
	CMSFlexGrid	m_Grid;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVegasEDLParserDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	CEDLParser m_EDLParser;
	bool m_Test;
	int m_Index;
	
	GDBServer m_gdb;

	LARGE_INTEGER m_TimeStart;
	LARGE_INTEGER m_TicksPerSecond;
	LARGE_INTEGER m_TimeNow;
	LARGE_INTEGER m_TimeElapsed;

	char* m_SoundData;

	CString m_CutSceneName;

	int* m_IndexTable;
	int m_IndexCount;

	// Generated message map functions
	//{{AFX_MSG(CVegasEDLParserDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButton1();
	afx_msg void OnButtonTest();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnMove(int x, int y);
	afx_msg void OnCancelMode();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnButtonStop();
	afx_msg void OnButtonSaveAsCsc();
	afx_msg void OnCheckHideFiltered();
	afx_msg void OnSelChangeMsflexgrid1();
	afx_msg void OnButtonUpdate();
	afx_msg void OnRowColChangeMsflexgrid1();
	afx_msg void OnButtonMerge();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VEGASEDLPARSERDLG_H__8696495D_F1ED_4CA7_AD03_C70FF35DA0CC__INCLUDED_)
