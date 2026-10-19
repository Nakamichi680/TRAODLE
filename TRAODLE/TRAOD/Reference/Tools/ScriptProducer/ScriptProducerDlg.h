// ScriptProducerDlg.h : header file
//

#if !defined(AFX_SCRIPTPRODUCERDLG_H__A3DE3F79_7FFB_49EA_8E44_56B559E6ECCD__INCLUDED_)
#define AFX_SCRIPTPRODUCERDLG_H__A3DE3F79_7FFB_49EA_8E44_56B559E6ECCD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class scriptRelEntry 
{
	public:
		scriptRelEntry();

		DWORD		Hash;
		void	*	data;
		DWORD		len;
		char		filename[_MAX_PATH];
	
		int			Archive(FILE *fp);
		int			Compile();
		void		Desarchive(DWORD Hash, void *data, DWORD len, const char *szFilename);
};

class scriptSymEntry 
{
	public:
		DWORD		Hash;
		DWORD		Link;
		void		Archive(FILE *fp);
		void		Dearchive(DWORD a, DWORD b);
};

/////////////////////////////////////////////////////////////////////////////
// CScriptProducerDlg dialog

class CScriptProducerDlg : public CDialog
{
// Construction
public:
	CScriptProducerDlg(CWnd* pParent = NULL);	// standard constructor
	void SetArchive(LPSTR filename);

// Dialog Data
	//{{AFX_DATA(CScriptProducerDlg)
	enum { IDD = IDD_SCRIPTPRODUCER_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScriptProducerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON	m_hIcon;
	TCHAR	m_szFile[_MAX_PATH];
	DWORD	m_nVersion;
	DWORD	m_nRelScript;
	DWORD	m_nSymScript;

	scriptRelEntry	*m_pRelScript;
	scriptSymEntry	*m_pSymScript;

	void	Archive();
	void	Dearchive();
	void	LoadBrowseFile(LPSTR filename);
	void	SaveBrowseFile(LPSTR filename);

	// Generated message map functions
	//{{AFX_MSG(CScriptProducerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBrowse();
	afx_msg void OnReload();
	afx_msg void OnRepack();
	afx_msg void OnCompile();
	afx_msg void OnBrowseInput();
	afx_msg void OnEdit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCRIPTPRODUCERDLG_H__A3DE3F79_7FFB_49EA_8E44_56B559E6ECCD__INCLUDED_)
