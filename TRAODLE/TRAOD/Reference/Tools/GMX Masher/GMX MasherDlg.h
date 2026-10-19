// GMX MasherDlg.h : header file
//

#if !defined(AFX_GMXMASHERDLG_H__E8D8130A_33CF_4384_A415_0824C84FE7FC__INCLUDED_)
#define AFX_GMXMASHERDLG_H__E8D8130A_33CF_4384_A415_0824C84FE7FC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "gdbserver/gdbglobal.h"

typedef	struct tagWAD_DIRENTRY
{
		unsigned int HashID;

		int			 Offset;
		int			 Length;

} WAD_DIRENTRY;

typedef	struct tagWAD_DIRECTORY
{
		float	Ver;
		short	nFiles;
		short	nResident;

		WAD_DIRENTRY Files[170];
} WAD_DIRECTORY;




/////////////////////////////////////////////////////////////////////////////
// CGMXMasherDlg dialog

class CGMXMasherDlg : public CDialog
{
// Construction
public:
	int Go();
	int MashLevel(CString level);
	unsigned int GetHashValue(char *s);
	CGMXMasherDlg(CWnd* pParent = NULL);	// standard constructor
	GDBServer m_gdb;

// Dialog Data
	//{{AFX_DATA(CGMXMasherDlg)
	enum { IDD = IDD_GMXMASHER_DIALOG };
	CEdit	m_Filter;
	CProgressCtrl	m_ProgressMash;
	CStatic	m_ProgressDesc;
	CProgressCtrl	m_Progress;
	CEdit	m_GMXCSOutput;
	CEdit	m_GMXCSInput;
	CEdit	m_SceneName;
	CEdit	m_ProjectFolder;
	CEdit	m_GMXOutputFolder;
	CEdit	m_GMXInputFolder;
	CEdit	m_AWDOutputFolder;
	CButton	m_Spanish;
	CButton	m_Korean;
	CButton	m_Japanese;
	CButton	m_Italian;
	CButton	m_German;
	CButton	m_French;
	CButton	m_English;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGMXMasherDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CGMXMasherDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GMXMASHERDLG_H__E8D8130A_33CF_4384_A415_0824C84FE7FC__INCLUDED_)
