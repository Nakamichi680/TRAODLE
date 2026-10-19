// ScriptProducerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ScriptProducer.h"
#include "ScriptProducerDlg.h"
#include "gmx.h"
#include    "../SmallCompiler/amx.h"

#define SCRIPT_FILE_FORMAT_VERSION 0x102

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern void strMFE(char *szNew, const char *szOld, const char *ext);

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScriptProducerDlg dialog

CScriptProducerDlg::CScriptProducerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScriptProducerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScriptProducerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	*m_szFile = 0;
}

void CScriptProducerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScriptProducerDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CScriptProducerDlg, CDialog)
	//{{AFX_MSG_MAP(CScriptProducerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK2, OnBrowse)
	ON_BN_CLICKED(IDC_BUTTON3, OnReload)
	ON_BN_CLICKED(IDC_BUTTON2, OnRepack)
	ON_BN_CLICKED(IDC_BUTTON4, OnCompile)
	ON_BN_CLICKED(IDC_BUTTON1, OnBrowseInput)
	ON_BN_CLICKED(IDC_BUTTON5, OnEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


const char *file_name(const char *szFilename)
{
	const char *s = szFilename;
	while (*s)
		s++;

	while ((s>szFilename)&&(*s!='/')&&(*s!='\\')&&(*s!=':'))
		s--;

	return (*s=='/')||(*s=='\\')||(*s==':') ? s+1 : s;
}

const char *file_ext(const char *szFilename)
{
	const char *s = szFilename;
	while (*s)
		s++;

	while ((s>szFilename)&&(*s!='.'))
		s--;
	return (*s == '.') ? s+1 : s;
}

int file_cmpExtension(const char *filename, const char *ext)
{
	const char *s = file_ext(filename);
	if (s)
	{
		return stricmp(s, ext);
	}
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
// CScriptProducerDlg message handlers

BOOL CScriptProducerDlg::OnInitDialog()
{
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
	
	// TODO: Add extra initialization here

	CListCtrl *list = (CListCtrl*)GetDlgItem(IDC_LIST1);
	list->InsertColumn(0, "Hash", LVCFMT_LEFT, 60);
	list->InsertColumn(1, "Length", LVCFMT_LEFT, 80);
	list->InsertColumn(2, "Filename", LVCFMT_LEFT, 210);
	list->InsertColumn(3, "Status", LVCFMT_LEFT, 80);

	CListCtrl *list2 = (CListCtrl*)GetDlgItem(IDC_LIST2);
	list2->InsertColumn(0, "Hash", LVCFMT_LEFT, 64);
	list2->InsertColumn(1, "Link", LVCFMT_LEFT, 64);

	if (*m_szFile)
	{
		CEdit		*pEdit = (CEdit *)GetDlgItem (IDC_EDIT1);
		pEdit->SetWindowText(m_szFile);	
		Dearchive();
	}	
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CScriptProducerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CScriptProducerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CScriptProducerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


int BrowseFile(char *filename, const char *title, int maxChar, const char *filter, int iFilter)
{
	char initDisk[_MAX_DRIVE];
	char initDir[_MAX_DIR];
	char initFile[_MAX_FNAME];
	char initExt[_MAX_EXT];

	_splitpath(filename, initDisk, initDir, initFile, initExt);
	
	OPENFILENAME ofn; 
	memset(&ofn, 0,	sizeof(OPENFILENAME));

	ofn.lStructSize 	= sizeof(OPENFILENAME);
	ofn.hwndOwner		= 0;
	ofn.lpstrFilter 	= filter;
	ofn.nFilterIndex	= iFilter;
	ofn.lpstrFile		= initFile;
	ofn.nMaxFile		= maxChar;
	
	ofn.lpstrInitialDir = initDir; 
	ofn.lpstrDefExt 	= initExt + 1;
	ofn.lpstrTitle 		= title;
	
	ofn.Flags			= OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;  
	
	int ret = (GetOpenFileName(&ofn)==TRUE);

	if (ret)
		strncpy(filename, ofn.lpstrFile, maxChar);

	return ret;
}

void CScriptProducerDlg::OnBrowse() 
{
	// TODO: Add your control notification handler code here
	
	if (BrowseFile(m_szFile, "Open source file", _MAX_PATH, "Exported Script Database (*.SCX)\0*.SCX\0Wad file (*.GMX)\0*.GMX\0All files (*.*)\0*.*\0\0", 0))
	{
		CEdit		*pEdit = (CEdit *)GetDlgItem (IDC_EDIT1);
		pEdit->SetWindowText(m_szFile);
		Dearchive();
	}
}

scriptRelEntry::scriptRelEntry()
{
	Hash = 0;
	data = 0;
	len = 0;
	*filename = 0;
}

int scriptRelEntry::Compile()
{	
	if (*filename)
	{
		void *olddata = data;
		int ret = ::Compile("output.tmp", filename, &data, &len);
		if (!ret)
		{
			delete [] olddata;
		}
		else
			data = olddata;
		return ret;
	}
	return 0;
}

static void falign(int Boundary, FILE * fp)
{
	int CurLen = ftell(fp);
	int	len = CurLen;

	len +=   Boundary-1;
	len &= ~(Boundary-1);

	if (len-CurLen>0)
	{
		unsigned long tmp[]={
			0xdeadbeef,
			0xdeadbeef,
			0xdeadbeef,
			0xdeadbeef,
			0xdeadbeef,
			0xdeadbeef,
			0xdeadbeef,
			0xdeadbeef,
			0xdeadbeef,
			0xdeadbeef,
			0xdeadbeef,
			0xdeadbeef,
			0xdeadbeef,
			0xdeadbeef,
			0xdeadbeef
		};
		fwrite(tmp, 1, len-CurLen, fp);
	}
}

int scriptRelEntry::Archive(FILE *fp)
{
	fwrite(&Hash, 1, sizeof(DWORD), fp);
	fwrite(&len, 1, sizeof(DWORD), fp);
	falign(16, fp);
	fwrite(data, len, sizeof(char), fp);
	falign(4, fp);
	return len;
}

void scriptRelEntry::Desarchive(DWORD _Hash, void *_data, DWORD _len, const char *_szFilename)
{
	Hash =  _Hash;
	len = _len;
	data = new char[len];
	memcpy(data, _data, len);
	// Verify header

	AMX_HEADER *hdr = (AMX_HEADER *)data;
	ASSERT( !(((unsigned short)hdr->magic!=AMX_MAGIC || hdr->defsize!=sizeof(AMX_FUNCSTUB))));
	ASSERT( !(hdr->amx_version<MIN_FILE_VERSION));
	ASSERT( !(hdr->stp<=0));
	ASSERT( !((hdr->flags & AMX_FLAG_CHAR16)!=0));
	ASSERT(len!=0);

	strcpy(filename, _szFilename);
}

void scriptSymEntry::Archive(FILE *fp)
{
	fwrite(&Hash, 1, sizeof(DWORD), fp);
	fwrite(&Link, 1, sizeof(DWORD), fp);
}

void scriptSymEntry::Dearchive(DWORD a, DWORD b)
{
	Hash = a;
	Link = b;
}

void CScriptProducerDlg::Archive()
{	
	// Read file
	FILE *fp = fopen(m_szFile, "wb");
	if (!fp)
	{
		return;
	}

	fwrite( &m_nVersion, sizeof(DWORD), 1 , fp);
	fwrite( &m_nRelScript, sizeof(DWORD), 1, fp);

	int i;
	for	(i=0; i<m_nRelScript; i++)
	{
		m_pRelScript[i].Archive(fp);		
	}

	fwrite( &m_nSymScript, sizeof(DWORD), 1, fp );

	for	(i=0; i<m_nSymScript; i++)
	{
		m_pSymScript[i].Archive(fp);
	}

	fclose(fp);

}

long file_size(FILE * fp)
{
	long curpos, length;
    curpos = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    length = ftell(fp);
    fseek(fp, curpos, SEEK_SET);
	return length;
}


void CScriptProducerDlg::Dearchive()
{

	FILE *fp = 0;
	int len = 0;
	while(1)
	{		
		if (!file_cmpExtension(m_szFile, "GMX"))
		{
			WadFileReader pWad(m_szFile);
			char tmp[MAX_PATH];
			strMFE(tmp, file_name(m_szFile), "SCX");
			WAD_DIRENTRY *pF = pWad.GetEntry(tmp);
			if (pF)
			{
				fp = fopen(m_szFile, "rb");	
				fseek(fp, pF->Offset, SEEK_SET);
				len = pF->Length;
				strMFE(m_szFile, tmp, "SCX");
				break;
			}
		}
		else
		{
			 fp = fopen(m_szFile, "rb");
			 if (fp)
			 {
				len = file_size(fp);
				break;
			 }
			 else
			 {
				 char tmp[MAX_PATH];
				 strMFE(tmp, m_szFile, "GMX");
				 strcpy(m_szFile, tmp);
			 }
		}
	}
	// Read file
	
	if (!fp)
	{
		return;
	}
	
	if (!len)
	{
		return ;
	}

	void *pData = new char[ len ];
	fread(pData, len, 1, fp);
	fclose(fp);	

	CListCtrl *list = (CListCtrl*)GetDlgItem(IDC_LIST1);
	CListCtrl *list2 = (CListCtrl*)GetDlgItem(IDC_LIST2);

	// Parse script
	char	*pWrk = (char*)pData;
	int		 i;

	m_nVersion   = *(((int *)pWrk)+0);
	ASSERT(m_nVersion == SCRIPT_FILE_FORMAT_VERSION);
	m_nRelScript = *(((int *)pWrk)+1);

	pWrk += 8;

	m_pRelScript = new scriptRelEntry [ m_nRelScript ];
	char szBrowseFilename[_MAX_PATH];
	strMFE(szBrowseFilename, m_szFile, "bsc");
	LoadBrowseFile(szBrowseFilename);
	list->DeleteAllItems();
	list2->DeleteAllItems();

	for	(i=0; i<m_nRelScript; i++)
	{
		DWORD	hash = *(((int *)pWrk)+0);		// Get hash id
		int len;
		pWrk += 4;
		len = *(((int *)pWrk)+0);				// Get program length
		pWrk += 4;

		// align 16byte								// Align script code on 16byte
		{
			int mis = ((char*)pWrk-(char*)pData)&15;
			if (mis)
			{
				pWrk+=16-mis;
			}
		}

		m_pRelScript[i].Desarchive(hash, pWrk, len, m_pRelScript[i].filename);
		pWrk += len;	
		
		// align 4byte								// Post
		{
			int mis = len&3;
			if (mis)
			{
				pWrk+=4-mis;
			}
		}

		LVITEM lv;
		memset(&lv, 0, sizeof(lv));
		char tmp[32];
		sprintf(tmp, "%x", hash);
		lv.mask = LVIF_TEXT;
		lv.iItem = i; 
		lv.iSubItem = 0; 
		lv.state = 0; 
		lv.pszText = tmp; 
		lv.cchTextMax = strlen(tmp)+1;
		list->InsertItem(&lv);

		sprintf(tmp, "%d", len);
		lv.cchTextMax = strlen(tmp)+1;
		lv.iSubItem = 1;
		list->SetItem(&lv);

		lv.pszText = *m_pRelScript[i].filename ? m_pRelScript[i].filename : "??";
		lv.cchTextMax = strlen(lv.pszText)+1;
		lv.iSubItem = 2;
		list->SetItem(&lv);

		sprintf(tmp, "<Unknown>");
		lv.pszText = tmp;
		lv.cchTextMax = strlen(tmp)+1;
		lv.iSubItem = 3;
		list->SetItem(&lv);
	}

	m_nSymScript  = *(((int *)pWrk)); pWrk+=4;
	m_pSymScript = new scriptSymEntry [ m_nSymScript ];
	
	for	(i=0; i<m_nSymScript; i++)
	{
		m_pSymScript[i].Dearchive(*(DWORD*)(pWrk), *(DWORD*)(pWrk+4));
		pWrk+=8;

		LVITEM lv;
		memset(&lv, 0, sizeof(lv));
		char tmp[32];
		sprintf(tmp, "%x", m_pSymScript[i].Hash);
		lv.mask = LVIF_TEXT;
		lv.iItem = i; 
		lv.iSubItem = 0; 
		lv.state = 0; 
		lv.pszText = tmp; 
		lv.cchTextMax = strlen(tmp)+1;
		list2->InsertItem(&lv);

		sprintf(tmp, "%x", m_pSymScript[i].Link);
		lv.cchTextMax = strlen(tmp)+1;
		lv.iSubItem = 1;
		list2->SetItem(&lv);		
	}
	return;
}

void CScriptProducerDlg::OnReload() 
{
	// TODO: Add your control notification handler code here
	Dearchive();	
}


void CScriptProducerDlg::OnRepack() 
{
	// TODO: Add your control notification handler code here
	Archive();

	char szBrowseFilename[_MAX_PATH];
	strMFE(szBrowseFilename, m_szFile, "bsc");
	SaveBrowseFile(szBrowseFilename);
}

void CScriptProducerDlg::OnCompile() 
{
	// TODO: Add your control notification handler code here
	CListCtrl *pmyListCtrl = (CListCtrl*)GetDlgItem(IDC_LIST1);

	UINT i, uSelectedCount = pmyListCtrl->GetSelectedCount();
	int  nItem = -1;

	// Update all of the selected items.
	if (uSelectedCount > 0)
	{
	   for (i=0;i < uSelectedCount;i++)
	   {
			nItem = pmyListCtrl->GetNextItem(nItem, LVNI_SELECTED);
			int ret = m_pRelScript[nItem].Compile();

			LVITEM lv;
			memset(&lv, 0, sizeof(lv));
			char tmp[32];
			
			lv.mask = LVIF_TEXT;
			lv.iItem = nItem; 
			lv.iSubItem = 0; 
			lv.state = 0; 
			lv.pszText = tmp; 
			lv.cchTextMax = strlen(tmp)+1;

			sprintf(tmp, "%d", m_pRelScript[nItem].len);
			lv.cchTextMax = strlen(tmp)+1;
			lv.iSubItem = 1;
			pmyListCtrl->SetItem(&lv);

			sprintf(tmp, ret ? "Failed" : "Updated");
			lv.cchTextMax = strlen(tmp)+1;
			lv.iSubItem = 3;
			pmyListCtrl->SetItem(&lv);
	   }
	}
	Archive();

	
}

void CScriptProducerDlg::OnBrowseInput() 
{
	// TODO: Add your control notification handler code here
	CListCtrl *pmyListCtrl = (CListCtrl*)GetDlgItem(IDC_LIST1);

	UINT i, uSelectedCount = pmyListCtrl->GetSelectedCount();
	int  nItem = -1;

	// Update all of the selected items.
	if (uSelectedCount > 0)
	{
	   for (i=0;i < uSelectedCount;i++)
	   {
			nItem = pmyListCtrl->GetNextItem(nItem, LVNI_SELECTED);
			ASSERT(nItem != -1);		  
			if (BrowseFile(m_pRelScript[nItem].filename, "Open original script", _MAX_PATH, "SmallC Script (*.SMA)\0*.SMA\0All files (*.*)\0*.*\0\0", 0))
			{
				LVITEM lv;
				memset(&lv, 0, sizeof(lv));
				lv.mask = LVIF_TEXT;
				lv.iItem = nItem; 
				lv.iSubItem = 0; 
				lv.state = 0; 			
				lv.pszText = m_pRelScript[nItem].filename;		
				lv.cchTextMax = strlen(lv.pszText)+1;
				lv.iSubItem = 2;
				pmyListCtrl->SetItem(&lv);
				pmyListCtrl->Update(nItem); 				
			}
	   }
	}
	
}
void strcpy_clean(char *d, const char *s)
{
	while (*s)
	{
		if (*s!='"')
		{
			*d = *s;
			d++;
		}
		s++;
	}
	*d = 0;
}

void CScriptProducerDlg::SetArchive(LPSTR szFilename)
{
	if (szFilename && *szFilename)
	{
		strcpy_clean(m_szFile, szFilename);		
	}
}


void CScriptProducerDlg::LoadBrowseFile(LPSTR szFilename)
{
	FILE *fp = fopen(szFilename,"rb");
	if (fp)
	{
		int n;
		fread(&n, 1, sizeof(int), fp);
		n = min(n, m_nRelScript);
		for	(int i=0; i<n; i++)
		{
			fread(m_pRelScript[i].filename, 1, _MAX_PATH, fp);
		}
		fclose(fp);
	}	
	else
	{
		for (int i=0;i<m_nRelScript;i++)
		{
			*m_pRelScript[i].filename = 0;
		}
	}
}

void CScriptProducerDlg::SaveBrowseFile(LPSTR szFilename)
{
	FILE *fp = fopen(szFilename,"wb");
	if (fp)
	{
		fwrite(&m_nRelScript, 1, sizeof(int), fp);
		for	(int i=0; i<m_nRelScript; i++)
		{
			fwrite(m_pRelScript[i].filename, 1, _MAX_PATH, fp);
		}
		fclose(fp);
	}
}

extern int GetEditorProgram(char *dst);

void CScriptProducerDlg::OnEdit() 
{
	char editor[MAX_PATH];
	GetEditorProgram(editor);

	// TODO: Add your control notification handler code here
	CListCtrl *pmyListCtrl = (CListCtrl*)GetDlgItem(IDC_LIST1);

	UINT i, uSelectedCount = pmyListCtrl->GetSelectedCount();
	int  nItem = -1;

	// Update all of the selected items.
	if (uSelectedCount > 0)
	{
	   for (i=0;i < uSelectedCount;i++)
	   {
			nItem = pmyListCtrl->GetNextItem(nItem, LVNI_SELECTED);
			ASSERT(nItem != -1);		  
			ShellExecute(*this, "open", editor, m_pRelScript[nItem].filename, NULL, SW_NORMAL);
	   }
	}	
}
