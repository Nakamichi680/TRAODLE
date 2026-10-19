// GMX MasherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GMX Masher.h"
#include "GMX MasherDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CGMXMasherApp theApp;

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

DWORD WINAPI AG_ThreadProc(
  LPVOID lpParameter   // thread data
)
{
	CGMXMasherDlg* dlg=(CGMXMasherDlg*)lpParameter;
	if (dlg==0L)
		return 0L;
	
	dlg->EndDialog(dlg->Go());

}

DWORD gAG_ThreadID=0L;

/////////////////////////////////////////////////////////////////////////////
// CGMXMasherDlg dialog

CGMXMasherDlg::CGMXMasherDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGMXMasherDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGMXMasherDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGMXMasherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGMXMasherDlg)
	DDX_Control(pDX, IDC_EDIT_FILTER, m_Filter);
	DDX_Control(pDX, IDC_PROGRESS_BAR_MASH, m_ProgressMash);
	DDX_Control(pDX, IDC_STATIC_PROGRESS_DESC, m_ProgressDesc);
	DDX_Control(pDX, IDC_PROGRESS_BAR, m_Progress);
	DDX_Control(pDX, IDC_EDIT_GMX_CS_OUTPUT_FOLDER, m_GMXCSOutput);
	DDX_Control(pDX, IDC_EDIT_GMX_CS_INPUT_FOLDER, m_GMXCSInput);
	DDX_Control(pDX, IDC_EDIT_SCENENAME, m_SceneName);
	DDX_Control(pDX, IDC_EDIT_PROJECT_FOLDER, m_ProjectFolder);
	DDX_Control(pDX, IDC_EDIT_GMX_OUTPUT_FOLDER, m_GMXOutputFolder);
	DDX_Control(pDX, IDC_EDIT_GMX_INPUT_FOLDER, m_GMXInputFolder);
	DDX_Control(pDX, IDC_EDIT_AWD_OUTPUT_FOLDER, m_AWDOutputFolder);
	DDX_Control(pDX, IDC_CHECK_SPANISH, m_Spanish);
	DDX_Control(pDX, IDC_CHECK_KOREAN, m_Korean);
	DDX_Control(pDX, IDC_CHECK_JAPANESE, m_Japanese);
	DDX_Control(pDX, IDC_CHECK_ITALIAN, m_Italian);
	DDX_Control(pDX, IDC_CHECK_GERMAN, m_German);
	DDX_Control(pDX, IDC_CHECK_FRENCH, m_French);
	DDX_Control(pDX, IDC_CHECK_ENGLISH, m_English);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGMXMasherDlg, CDialog)
	//{{AFX_MSG_MAP(CGMXMasherDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGMXMasherDlg message handlers

BOOL CGMXMasherDlg::OnInitDialog()
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
	
	CGMXMasherAppCommandLineInfo* cli=&theApp.m_cli;

	m_English.SetCheck(cli->m_bEnglish);
	m_French.SetCheck(cli->m_bFrench);
	m_German.SetCheck(cli->m_bGerman);
	m_Italian.SetCheck(cli->m_bItalian);
	m_Spanish.SetCheck(cli->m_bSpanish);
	m_Japanese.SetCheck(cli->m_bJapanese);
	m_Korean.SetCheck(cli->m_bKorean);

	m_AWDOutputFolder.SetWindowText(cli->m_OutputFolder);
	m_GMXInputFolder.SetWindowText(cli->m_GMXInputDir);
	m_GMXOutputFolder.SetWindowText(cli->m_GMXOutputDir);
	m_ProjectFolder.SetWindowText(cli->m_ProjectFolder);
	m_SceneName.SetWindowText(cli->m_SceneName);
	m_GMXCSInput.SetWindowText(cli->m_GMXCSInputDir);
	m_GMXCSOutput.SetWindowText(cli->m_GMXCSOutputDir);
	m_Filter.SetWindowText(cli->m_Filter);

	m_Progress.SetRange(0,100);
	m_ProgressMash.SetRange(0,8);

	if (CreateThread(NULL,NULL,AG_ThreadProc,this,NULL,&gAG_ThreadID)==NULL)
	{
		MessageBox("Failed to Create Worker Thread");
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CGMXMasherDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CGMXMasherDlg::OnPaint() 
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
HCURSOR CGMXMasherDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

int CGMXMasherDlg::Go()
{
	CGMXMasherAppCommandLineInfo* cli=&theApp.m_cli;
	
	CString lastLevel="";

	if (cli->m_bAllLevels==false)
	{
		m_Progress.SetPos(100);
		return MashLevel(cli->m_SceneName);
	}
	else
	{

		m_gdb.Open();
		int countLevels=0;
		
		list<GDBLevel*>::iterator j;
		for ( j = m_gdb.m_lLevel.begin(); j != m_gdb.m_lLevel.end(); j++)
		{
			struct GDBLevel *pLevelRef = *j;
			// Only import the levels that have a level export name assigned to them - and do not start with "CS" or "IG" as these are cutscenes!
			if ((strlen(pLevelRef->m_szExportName)>0)&&(pLevelRef->m_szExportName[0]!='*'))
			{
				CString levelExportName=pLevelRef->m_szExportName;

				if (levelExportName.Compare(lastLevel)!=0)
				{
					if (pLevelRef->m_nZone!=-1)
					{
						if (levelExportName.Left(cli->m_Filter.GetLength()).CompareNoCase(cli->m_Filter)==0)
						{
							lastLevel=levelExportName;
							countLevels++;
						}
					}
				}
			}
		}

		lastLevel="";

		int errCount=0;

		int levelNumber=0;

		// Do database stuff here
		for ( j = m_gdb.m_lLevel.begin(); j != m_gdb.m_lLevel.end(); j++)
		{
			struct GDBLevel *pLevelRef = *j;
			// Only import the levels that have a level export name assigned to them - and do not start with "CS" or "IG" as these are cutscenes!
			if ((strlen(pLevelRef->m_szExportName)>0)&&(pLevelRef->m_szExportName[0]!='*'))
			{
				CString levelExportName=pLevelRef->m_szExportName;

				if (levelExportName.Compare(lastLevel)!=0)
				{
					if (pLevelRef->m_nZone!=-1)
					{
						if (levelExportName.Left(cli->m_Filter.GetLength()).CompareNoCase(cli->m_Filter)==0)
						{
							lastLevel=levelExportName;
							
							m_SceneName.SetWindowText(levelExportName);
							levelNumber++;

							m_Progress.SetPos((100/countLevels)*levelNumber);

							int mashErr=MashLevel(levelExportName);
							if (mashErr!=0L)
							{
								errCount++;
							}
						}
					}
				}
			}
		}

		m_gdb.Release();

		if (errCount!=0)
		{
			MessageBox("Error(s) have occurred while mashing");
			return 1;
		}
	}

	return 0;
}

int CGMXMasherDlg::MashLevel(CString level)
{
	bool cutscene=false;
	if ((level.Left(2)=="CS")||(level.Left(2)=="IG"))
		cutscene=true;

	CString AWDcommandLine="z:\\audio\\tools\\awdmasher.exe ";

	CGMXMasherAppCommandLineInfo* cli=&theApp.m_cli;

	if (cli->m_bEnglish==true)
		AWDcommandLine+=" -english";

	if (cli->m_bFrench==true)
		AWDcommandLine+=" -french";

	if (cli->m_bGerman==true)
		AWDcommandLine+=" -german";

	if (cli->m_bItalian==true)
		AWDcommandLine+=" -italian";

	if (cli->m_bSpanish==true)
		AWDcommandLine+=" -spanish";

	if (cli->m_bJapanese==true)
		AWDcommandLine+=" -japanese";

	if (cli->m_bKorean==true)
		AWDcommandLine+=" -korean";

	AWDcommandLine+=" proj:";
	AWDcommandLine+=cli->m_ProjectFolder;

	AWDcommandLine+=" scene:";
	AWDcommandLine+=level;

	AWDcommandLine+=" exp:";
	AWDcommandLine+=cli->m_OutputFolder;

	STARTUPINFO ag_sui;
	memset(&ag_sui,0,sizeof(ag_sui));
	ag_sui.cb=sizeof(ag_sui);
	PROCESS_INFORMATION ag_pi;
	memset(&ag_pi,0,sizeof(ag_pi));

	m_ProgressMash.SetPos(1);
	m_ProgressDesc.SetWindowText(CString("["+level+"]"+" Mashing AWD")); 

	bool result=CreateProcess(0,AWDcommandLine.GetBuffer(1),NULL,NULL,TRUE,0,NULL,"z:\\audio\\tools",&ag_sui,&ag_pi);
	if (result==false)
	{
		MessageBox("Failed to create awdmasher");
		return 1;
	}
	else
	{
		WaitForSingleObject(ag_pi.hProcess,INFINITE);
		DWORD exitCode=0L;
		GetExitCodeProcess(ag_pi.hProcess,&exitCode);

		switch (exitCode)
		{
		case 0: // Normal Termination
			// MessageBox("AWD Masher Completed Successfully");
			break;
		case 1: // Abnormal Termination
			MessageBox("AWD Masher FAILED");
			break;
		case 2: // Partial Error Termination
			MessageBox("AWD Masher Partial Failure");
			break;
		}
	}

	m_ProgressMash.SetPos(2);
	m_ProgressDesc.SetWindowText(CString("["+level+"]"+" Opening Mashed AWD")); 

	CString awdPath=cli->m_OutputFolder+"\\"+level+".AWD";
	FILE* awdfile=fopen(awdPath,"rb");
	if (awdfile==0L)
	{
		MessageBox("Unable to open AWD file");
		return 1;
	}

	fseek(awdfile,0,SEEK_END);
	fpos_t filelength;
	fgetpos(awdfile,&filelength);
	fseek(awdfile,0,SEEK_SET);


	CString gmxPath="";
	
	if (cutscene==true)
		gmxPath=cli->m_GMXCSInputDir+"\\"+level+".GMX";
	else
		gmxPath=cli->m_GMXInputDir+"\\"+level+".GMX";
		

	CString gmxOutput="";
	
	if (cutscene==true)
		gmxOutput=cli->m_GMXCSOutputDir+"\\"+level+".GMX";
	else
		gmxOutput=cli->m_GMXOutputDir+"\\"+level+".GMX";
		
	// Check if file source and dest are the same
	if (gmxOutput.CompareNoCase(gmxPath)==0)
	{
		MessageBox("Source and Destination GMX paths are the same");
		fclose(awdfile);
		return 1;
	}
	
	m_ProgressMash.SetPos(3);
	m_ProgressDesc.SetWindowText(CString("["+level+"]"+" Copying Input GMX to Output GMX")); 

	// Copy source file to destination file
	if (CopyFile(gmxPath,gmxOutput,false)==0)
	{
		MessageBox("Unable to copy Input GMX to Output GMX");
		fclose(awdfile);
		return 1;
	}

	
	// Get Header From copied file into memory
	FILE* f=fopen(gmxOutput,"rb");
	if (f==0L)
	{
		CString errmes="Error opening GMX file ";
		errmes+=gmxPath;
		MessageBox(errmes);
		fclose(awdfile);
		return 1;
	}

	WAD_DIRECTORY waddir;
	fread(&waddir,0x800,1,f);
	fclose(f);


	// Process Header and Add Entry
	WAD_DIRENTRY* newEntry=&waddir.Files[waddir.nFiles];
	WAD_DIRENTRY* prevEntry=&waddir.Files[waddir.nFiles-1];

	CString sceneName=cli->m_SceneName+".AWD";
	sceneName.MakeUpper();

	newEntry->HashID=GetHashValue(sceneName.GetBuffer(1));

	newEntry->Length=filelength;
	if ((filelength%2048)!=0)
		newEntry->Length+=(2048-(filelength%2048));

	newEntry->Offset=prevEntry->Offset+prevEntry->Length;
	if ((newEntry->Offset%2048)!=0)
	{
		MessageBox("Not on a 2048 byte boundary");
		newEntry->Offset+=(2048-(newEntry->Offset%2048));
	}

	waddir.nFiles++;


	// Open Output GMX For appending (binary)
	m_ProgressMash.SetPos(4);
	m_ProgressDesc.SetWindowText(CString("["+level+"]"+" Opening Output GMX")); 

	FILE* gmxout=fopen(gmxOutput,"r+b");
	if (gmxout==0L)
	{
		MessageBox("Unable to open output GMX file");
		fclose(awdfile);
		return 1;
	}

	m_ProgressMash.SetPos(5);
	m_ProgressDesc.SetWindowText(CString("["+level+"]"+" Writing new GMX header")); 

	fwrite(&waddir,2048,1,gmxout);
	fseek(gmxout,2048+newEntry->Offset,SEEK_SET);
	
	m_ProgressMash.SetPos(6);
	m_ProgressDesc.SetWindowText(CString("["+level+"]"+" Reading AWD data")); 

	char* temp=new char[newEntry->Length];
	fread(temp,filelength,1,awdfile);

	m_ProgressMash.SetPos(7);
	m_ProgressDesc.SetWindowText(CString("["+level+"]"+" Appending AWD data to output GMX")); 

	fwrite(temp,newEntry->Length,1,gmxout);

	m_ProgressMash.SetPos(8);
	m_ProgressDesc.SetWindowText(CString("["+level+"]"+" GMX Mash Completed")); 

	delete temp;
	fclose(awdfile);
	fclose(gmxout);

	return 0;
}


//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

/*
	Function	:	STRING_GetHashValue
	Useage		:	Return Hash Value Of String
*/

unsigned int CGMXMasherDlg::GetHashValue(char *s)
{
	int h,l,step;
	l = strlen(s);
	h = l;
	step = (l>>6)+1;
	for(;l>0;l-=step)
		h = h^((h<<5)+(h>>2)+(unsigned char)*(s++));
  	return h;
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

