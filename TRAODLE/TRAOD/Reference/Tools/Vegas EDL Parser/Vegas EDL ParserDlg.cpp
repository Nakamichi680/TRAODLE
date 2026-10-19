// Vegas EDL ParserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Vegas EDL Parser.h"
#include "Vegas EDL ParserDlg.h"

#include "EDLParser.h"

#include "mmsystem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// Column Index and names

#define EDL_NUM_COLUMNS 7

#define EDL_COL_INDEX 0
#define EDL_COL_START_TIME 1
#define EDL_COL_LENGTH 2
#define EDL_COL_EXPORT 3
#define EDL_COL_SPEECH_ID 4
#define EDL_COL_DB_ID 5
#define EDL_COL_TEXT 6

#define EDL_COL_INDEX_NAME "Index"
#define EDL_COL_START_TIME_NAME "StartTime"
#define EDL_COL_LENGTH_NAME "Length"
#define EDL_COL_EXPORT_NAME "Export"
#define EDL_COL_SPEECH_ID_NAME "Speech ID"
#define EDL_COL_DB_ID_NAME "DB ID"
#define EDL_COL_TEXT_NAME "Text"

#define EDL_COL_INDEX_WIDTH 500
// #define EDL_COL_START_TIME_WIDTH 100
// #define EDL_COL_LENGTH_WIDTH 100
// #define EDL_COL_EXPORT_WIDTH 100
#define EDL_COL_SPEECH_ID_WIDTH 3000
#define EDL_COL_DB_ID_WIDTH 1200
#define EDL_COL_TEXT_WIDTH 24000

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
// CVegasEDLParserDlg dialog

CVegasEDLParserDlg::CVegasEDLParserDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVegasEDLParserDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVegasEDLParserDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVegasEDLParserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVegasEDLParserDlg)
	DDX_Control(pDX, IDC_BUTTON_MERGE, m_Merge);
	DDX_Control(pDX, IDC_CHECK_EXPORT, m_Export);
	DDX_Control(pDX, IDC_EDIT_TEXT, m_Text);
	DDX_Control(pDX, IDC_EDIT_STARTTIME, m_StartTime);
	DDX_Control(pDX, IDC_EDIT_SPEECH_ID, m_SpeechID);
	DDX_Control(pDX, IDC_EDIT_LENGTH, m_Length);
	DDX_Control(pDX, IDC_EDIT_DB_KEY, m_DBKey);
	DDX_Control(pDX, IDC_CHECK_HIDE_FILTERED, m_HideFiltered);
	DDX_Control(pDX, IDC_EDIT_STARTOFFSET, m_StartOffset);
	DDX_Control(pDX, IDC_EDIT_ENDOFFSET, m_EndOffset);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_StopButton);
	DDX_Control(pDX, IDC_BUTTON_TEST, m_TestButton);
	DDX_Control(pDX, IDC_STATIC_TIME, m_Time);
	DDX_Control(pDX, IDC_MSFLEXGRID1, m_Grid);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CVegasEDLParserDlg, CDialog)
	//{{AFX_MSG_MAP(CVegasEDLParserDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON_TEST, OnButtonTest)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_MOVE()
	ON_WM_CANCELMODE()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_AS_CSC, OnButtonSaveAsCsc)
	ON_BN_CLICKED(IDC_CHECK_HIDE_FILTERED, OnCheckHideFiltered)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, OnButtonUpdate)
	ON_BN_CLICKED(IDC_BUTTON_MERGE, OnButtonMerge)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVegasEDLParserDlg message handlers

BOOL CVegasEDLParserDlg::OnInitDialog()
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
	m_SoundData=0L;
	m_gdb.Open();
	m_HideFiltered.SetCheck(1);

	m_IndexTable=0L;


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CVegasEDLParserDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CVegasEDLParserDlg::OnPaint() 
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
HCURSOR CVegasEDLParserDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CVegasEDLParserDlg::OnButton1() 
{
	// TODO: Add your control notification handler code here
	CFileDialog fd(true,"*.txt",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,"Text Files (*.txt)|*.txt||");
	if (IDCANCEL==fd.DoModal())
		return;

	CString fileName=fd.GetFileName();
	CString fileExt=fd.GetFileExt();
	CString fileTitle=fd.GetFileTitle();
	CString filePath=fd.GetPathName();

	m_CutSceneName=fileName.Left(fileName.GetLength()-4);

	CEDLParser_Error err=m_EDLParser.Parse(filePath);
	if (err!=CEDLParser_NoError)
	{
		MessageBox(CEDLParser_ErrorText[err],"EDL Parse Error",MB_OK);
		return;
	}

	PopulateTable();

	CString windowTitle="Vegas EDL Parser - ";
	windowTitle+=m_CutSceneName;
	SetWindowText(windowTitle);

}

void CVegasEDLParserDlg::PopulateTable()
{
	int numEntries=m_EDLParser.GetNumLines();

	m_Grid.Clear();
	m_Grid.SetCols(EDL_NUM_COLUMNS);
	m_Grid.SetRows(numEntries+1);
	m_Grid.SetSelectionMode(1);



	m_Grid.SetTextMatrix(0,EDL_COL_INDEX,EDL_COL_INDEX_NAME);
	m_Grid.SetColWidth(0,EDL_COL_INDEX_WIDTH);

	m_Grid.SetTextMatrix(0,EDL_COL_START_TIME,EDL_COL_START_TIME_NAME);

	m_Grid.SetTextMatrix(0,EDL_COL_LENGTH,EDL_COL_LENGTH_NAME);

	m_Grid.SetTextMatrix(0,EDL_COL_EXPORT,EDL_COL_EXPORT_NAME);

	m_Grid.SetTextMatrix(0,EDL_COL_SPEECH_ID,EDL_COL_SPEECH_ID_NAME);
	m_Grid.SetColWidth(EDL_COL_SPEECH_ID,EDL_COL_SPEECH_ID_WIDTH);

	m_Grid.SetTextMatrix(0,EDL_COL_DB_ID,EDL_COL_DB_ID_NAME);
	m_Grid.SetColWidth(EDL_COL_DB_ID,EDL_COL_DB_ID_WIDTH);
	
	m_Grid.SetTextMatrix(0,EDL_COL_TEXT,EDL_COL_TEXT_NAME);
	m_Grid.SetColWidth(EDL_COL_TEXT,EDL_COL_TEXT_WIDTH);

	char tmp[200];
	sprintf(tmp,"%d",m_EDLParser.m_StartOffset);
	m_StartOffset.SetWindowText(tmp);
	sprintf(tmp,"%u",m_EDLParser.m_EndOffset);
	m_EndOffset.SetWindowText(tmp);

	if (m_IndexTable!=0L)
	{
		delete m_IndexTable;
		m_IndexTable=0L;
	}

	if (m_IndexTable==0L)
	{
		m_IndexTable=new int[numEntries];
		int i;
		for (i=0;i<numEntries;i++)
		{
			m_IndexTable[i]=-1;
		}
	}

	m_IndexCount=0;

	int c=0;

	int i;
	for (i=0;i<numEntries;i++)
	{
		CEDLLineData* lineData=m_EDLParser.GetLineEntry(i);
		if (lineData!=0L)
		{
			bool add=false;
			if (m_HideFiltered.GetCheck()!=0)
			{
				if (lineData->m_AutoExport==true)
					add=true;
/*
				if (lineData->m_FileNameExt.CollateNoCase(".wav")==0)
				{
					if (lineData->m_FileNameFName.Find(" - ")==-1)
					{
						if (lineData->m_FileNameFName.GetLength()<=8)
						{
							CString left2=lineData->m_FileNameFName.Left(2);
							if (left2.CompareNoCase("PA")==0)
								add=true;

							if (left2.CompareNoCase("PR")==0)
								add=true;

							if (left2.CompareNoCase("IG")==0)
								add=true;

							CString left3=lineData->m_FileNameFName.Left(3);
							if (left3.CompareNoCase("ADD")==0)
								add=true;

							CString right1=lineData->m_FileNameFName.Right(1);
							right1.MakeUpper();

							char* lastChar=right1.GetBuffer(1);
							if ((lastChar[0]>='A')&&(lastChar[0]<='Z'))
							{
								lineData->m_FileNameFName=lineData->m_FileNameFName.Left(lineData->m_FileNameFName.GetLength()-1);
							}

							if ((lineData->m_StartTime+lineData->m_Length)>m_EDLParser.m_EndOffset)
								add=false;
							
							if (lineData->m_StartTime<m_EDLParser.m_StartOffset)
								add=false;
							
						}
					}
				}
*/
			}
			else
			{
				add=true;
			}

			if (add==true)
			{
				unsigned int newStartTime=lineData->m_StartTime-m_EDLParser.m_StartOffset;
				
				m_IndexTable[m_IndexCount]=i;
				m_IndexCount++;

				char t[MAX_PATH];
				sprintf(t,"%i",c+1);	m_Grid.SetTextMatrix(c+1,EDL_COL_INDEX,t);
				sprintf(t,"%i",lineData->m_StartTime); m_Grid.SetTextMatrix(c+1,EDL_COL_START_TIME,t);
				sprintf(t,"%i",lineData->m_Length); m_Grid.SetTextMatrix(c+1,EDL_COL_LENGTH,t);
				m_Grid.SetTextMatrix(c+1,EDL_COL_SPEECH_ID,lineData->m_FileNameFName);

				if (lineData->m_AutoExport==true)
					m_Grid.SetTextMatrix(c+1,EDL_COL_EXPORT,"Yes");
				else
					m_Grid.SetTextMatrix(c+1,EDL_COL_EXPORT,"No");
					

				bool found=false;

				list<GDBSentence*>::iterator kk;
				for ( kk = m_gdb.m_lSentence.begin(); kk != m_gdb.m_lSentence.end(); kk++)
				{
					struct GDBSentence *pSentanceRef = *kk;

					// Get speech level key.
					if (lineData->m_FileNameFName==pSentanceRef->m_szKey)
					{

						const char* sentance_text=m_gdb.FindTextByKey(pSentanceRef->m_nKeyText);
						if (sentance_text!=0l)
						{
							m_Grid.SetTextMatrix(c+1,EDL_COL_TEXT,sentance_text);
							sprintf(t,"0x%08X",pSentanceRef->m_nKeyText); m_Grid.SetTextMatrix(c+1,EDL_COL_DB_ID,t);
							lineData->m_DBID=pSentanceRef->m_nKeyText;
						}

						found=true;
						break;
					}
				}

				if (found==false)
				{
					CString unknown="[";
					unknown+=lineData->m_FileNameFName;
					unknown+="]";
					m_Grid.SetTextMatrix(c+1,EDL_COL_SPEECH_ID,unknown);
				}

				c++;
			}
		}
	}
}

void CVegasEDLParserDlg::OnButtonTest() 
{
	// TODO: Add your control notification handler code here
	if (m_EDLParser.GetNumLines()==0L)
	{
		MessageBox("Unable To Test: No Entries Available","Oh no!");
		return;
	}

	m_TestButton.EnableWindow(FALSE);
	m_StopButton.EnableWindow(TRUE);

	m_Test=true;
	m_Index=0;


	CString cutscenewav="Z:\\Audio\\Projects\\Tombraider\\Samples\\Music\\CutScenes\\English\\";
	cutscenewav+=m_CutSceneName;
	cutscenewav+=".wav";


	PlaySound(NULL,NULL,NULL);

	if (m_SoundData!=0L)
	{
		delete m_SoundData;
		m_SoundData=0L;
	}
	
	fpos_t fileLength=0L;
	FILE* f=fopen(cutscenewav,"r");
	if (f!=0l)
	{
		fseek(f,0,SEEK_END);
		fgetpos(f,&fileLength);
		fclose(f);
		m_SoundData=new char[fileLength];
	}

	if (m_SoundData!=0L)
	{
		HANDLE fHandle=0L;
		if (fHandle=CreateFile(cutscenewav,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL))
		{
			DWORD dataRead=0L;
			ReadFile(fHandle,m_SoundData,fileLength,&dataRead,NULL);
			CloseHandle(fHandle);

			PlaySound(m_SoundData,NULL,SND_MEMORY | SND_ASYNC);
		}
	}

	QueryPerformanceFrequency(&m_TicksPerSecond);
	QueryPerformanceCounter(&m_TimeStart);

	SetTimer(100,100,0L);

}

void CVegasEDLParserDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnTimer(nIDEvent);

	if (nIDEvent==100)
	{

		QueryPerformanceCounter(&m_TimeNow);

		float ticksPerSec=(float)m_TicksPerSecond.QuadPart;
		float timeStart=(float)m_TimeStart.QuadPart;
		float timeNow=(float)m_TimeNow.QuadPart;
		float timeElapsed=(timeNow-timeStart)/ticksPerSec;

		char t[200];
		sprintf(t,"%0.03f",timeElapsed);
		m_Time.SetWindowText(t);
		m_Time.RedrawWindow(NULL,NULL,RDW_UPDATENOW | RDW_INTERNALPAINT);
		
		int numLines=m_EDLParser.GetNumLines();

		int i;
		for (i=m_Index;i<numLines;i++)
		{
			CEDLLineData* lineData=m_EDLParser.GetLineEntry(i);


			if (lineData!=0l)
			{
				CString timeContent=m_Grid.GetTextMatrix(i+1,EDL_COL_START_TIME);
				unsigned int gridTime=strtoul(timeContent,0L,10)-m_EDLParser.m_StartOffset;

//				if ((((float)lineData->m_StartTime)/1000.0f)<timeElapsed)
				if ((((float)gridTime)/1000.0f)<timeElapsed)
				{
					m_Grid.SetRow(i+1);
					m_Grid.SetCol(0);
					m_Grid.SetColSel(EDL_NUM_COLUMNS-1);
					m_Index=i+1;

					long cellLeft=m_Grid.GetCellLeft();
					long cellTop=m_Grid.GetCellTop();

					CDC* dc=this->GetDC();
					double scaleX=(dc->GetDeviceCaps(LOGPIXELSX)*(1.0/1440)); 
					double scaleY=(dc->GetDeviceCaps(LOGPIXELSY)*(1.0/1440)); 

					RECT grd; 
					m_Grid.GetWindowRect(&grd); //m_Grid is the flexgrid ctrl 

					RECT loc; 

					loc.left=grd.left+m_Grid.GetCellLeft()*scaleX; 
					loc.top=grd.top+m_Grid.GetCellTop()*scaleY; 
					loc.right=loc.left+m_Grid.GetCellWidth()*scaleX; 
					loc.bottom=loc.top+m_Grid.GetCellHeight()*scaleY; 
					ScreenToClient(&loc); //OOPS! forgot to add this! 

					int topRow=i-5;
					if (topRow<1)
						topRow=1;

					m_Grid.SetTopRow(topRow);
					
				}
				else
				{
					break;
				}
			}
		}

		if (m_Index==numLines)
		{
			m_TestButton.EnableWindow(TRUE);
			m_StopButton.EnableWindow(FALSE);
			KillTimer(100);
			m_Test=false;
		}
	}
}

void CVegasEDLParserDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	
	m_gdb.Release();

}

void CVegasEDLParserDlg::OnMove(int x, int y) 
{
	CDialog::OnMove(x, y);
	
	// TODO: Add your message handler code here
	
}

void CVegasEDLParserDlg::OnCancelMode() 
{
	CDialog::OnCancelMode();
	
	// TODO: Add your message handler code here
	
}

void CVegasEDLParserDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
	CRect r;
	GetClientRect(&r);

	char t[200];
	sprintf(t,"Left:%i Right:%i Top:%i Bottom:%i Width:%i Height:%i\n",r.left,r.right,r.top,r.bottom,r.Width(),r.Height());
	OutputDebugString(t);

	if (m_Grid.m_hWnd!=0L)
	{
		m_Grid.SetWindowPos(NULL,0,0,r.Width()-24,r.Height()-190,SWP_NOZORDER | SWP_NOMOVE);
	}
}

void CVegasEDLParserDlg::OnButtonStop() 
{
	// TODO: Add your control notification handler code here
	PlaySound(NULL,NULL,NULL);
	m_TestButton.EnableWindow(TRUE);
	m_StopButton.EnableWindow(FALSE);
	KillTimer(100);
	m_Test=false;
}

void CVegasEDLParserDlg::OnButtonSaveAsCsc() 
{
	// TODO: Add your control notification handler code here
	CFileDialog fd(false,"*.css",m_CutSceneName,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,"CutScene Subtitle File (*.css)|*.css||");
	if (IDCANCEL==fd.DoModal())
		return;

	CString fileName=fd.GetFileName();
	CString fileExt=fd.GetFileExt();
	CString fileTitle=fd.GetFileTitle();
	CString filePath=fd.GetPathName();

	FILE* f=fopen(filePath,"wb");
	if (f==0L)
		MessageBox("Unable to save .css file\n","ERROR");

	int numLines=m_EDLParser.GetNumLines();
	int outputLineCount=0;
	int i;
	for (i=0;i<numLines;i++)
	{
		CEDLLineData* lineData=m_EDLParser.GetLineEntry(i);
		if (lineData!=0L)
		{
			if (lineData->m_AutoExport==true)
				outputLineCount++;
		}
	}


	fwrite("CSCF",4,1,f); // Header
	fwrite(&outputLineCount,sizeof(numLines),1,f); // Num Lines

	unsigned int fileFlags1=0;
	fwrite(&fileFlags1,sizeof(fileFlags1),1,f);

	unsigned int fileFlags2=0;
	fwrite(&fileFlags2,sizeof(fileFlags2),1,f);

	for (i=0;i<numLines;i++)
	{
		CEDLLineData* lineData=m_EDLParser.GetLineEntry(i);
		if (lineData!=0L)
		{
			if (lineData->m_AutoExport==true)
			{
				// Write Frame Start Number
				float startTime=(float)lineData->m_StartTime;
				unsigned int frameOffset=(unsigned int)(((float)m_EDLParser.m_StartOffset/1000.0f)*30.0f);
				unsigned int frameNo=(unsigned int)((startTime/1000.0f)*30.0f);
				frameNo-=frameOffset;
				fwrite(&frameNo,sizeof(frameNo),1,f);

				// Write Frame Length
				float length=(float)lineData->m_Length;
				unsigned int numFrames=(unsigned int)((length/1000.0f)*30.0f);
				fwrite(&numFrames,sizeof(numFrames),1,f);

				// Write DB ID
				unsigned int dbID=lineData->m_DBID;
				fwrite(&dbID,sizeof(dbID),1,f);

				// Write Line Flags
				unsigned int Flags=lineData->m_Flags;
				fwrite(&Flags,sizeof(Flags),1,f);
			}
		}
	}

	fclose(f);
}

void CVegasEDLParserDlg::OnCheckHideFiltered() 
{
	// TODO: Add your control notification handler code here
	m_Index=0;
	PopulateTable();
}

BEGIN_EVENTSINK_MAP(CVegasEDLParserDlg, CDialog)
    //{{AFX_EVENTSINK_MAP(CVegasEDLParserDlg)
	ON_EVENT(CVegasEDLParserDlg, IDC_MSFLEXGRID1, 69 /* SelChange */, OnSelChangeMsflexgrid1, VTS_NONE)
	ON_EVENT(CVegasEDLParserDlg, IDC_MSFLEXGRID1, 70 /* RowColChange */, OnRowColChangeMsflexgrid1, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CVegasEDLParserDlg::OnSelChangeMsflexgrid1() 
{
	// TODO: Add your control notification handler code here

	// Get Row Number
	long row=m_Grid.GetRow();
	long rowsel=m_Grid.GetRowSel();
	CString temp;

	// Do enable disable window on multiple selection

	m_DBKey.EnableWindow(row==rowsel);
	m_SpeechID.EnableWindow(row==rowsel);
	m_Length.EnableWindow(row==rowsel);
	m_StartTime.EnableWindow(row==rowsel);
	m_Text.EnableWindow(row==rowsel);
	m_Export.EnableWindow(row==rowsel);
	m_Merge.EnableWindow(row!=rowsel);


	m_DBKey.SetWindowText(m_Grid.GetTextMatrix(row,EDL_COL_DB_ID));
	m_SpeechID.SetWindowText(m_Grid.GetTextMatrix(row,EDL_COL_SPEECH_ID));
	m_Length.SetWindowText(m_Grid.GetTextMatrix(row,EDL_COL_LENGTH));
	m_StartTime.SetWindowText(m_Grid.GetTextMatrix(row,EDL_COL_START_TIME));
	m_Text.SetWindowText(m_Grid.GetTextMatrix(row,EDL_COL_TEXT));
	m_Export.SetCheck(!m_Grid.GetTextMatrix(row,EDL_COL_EXPORT).CompareNoCase("Yes"));
}

void CVegasEDLParserDlg::OnButtonUpdate() 
{
	// TODO: Add your control notification handler code here
	long row=m_Grid.GetRow();

	CString speechID;
	m_SpeechID.GetWindowText(speechID);

	// Get index entry
	int index=m_IndexTable[row-1];

	CEDLLineData* lineData=m_EDLParser.GetLineEntry(index);
	if (lineData!=0L)
	{
		lineData->m_FileNameFName=speechID;
		lineData->m_AutoExport=m_Export.GetCheck();
		PopulateTable();
		OnSelChangeMsflexgrid1();
	}

	
}

void CVegasEDLParserDlg::OnRowColChangeMsflexgrid1() 
{
	// TODO: Add your control notification handler code here
	
}

void CVegasEDLParserDlg::OnButtonMerge() 
{
	// TODO: Add your control notification handler code here
	long row=m_Grid.GetRow();
	long rowsel=m_Grid.GetRowSel();

	// Get index entry
	int index1=m_IndexTable[row-1];

	CEDLLineData* lineData1=m_EDLParser.GetLineEntry(index1);
	if (lineData1==0L)
		return;

	// Get index entry
	int index2=m_IndexTable[rowsel-1];

	CEDLLineData* lineData2=m_EDLParser.GetLineEntry(index2);
	if (lineData2==0L)
		return;

	if ((rowsel-row)>=2)
	{
		int i;
		for (i=row+1;i<rowsel;i++)
		{
			CEDLLineData* lineDataTween=m_EDLParser.GetLineEntry(m_IndexTable[i-1]);
			if (lineDataTween!=0L)
			{
				lineDataTween->m_AutoExport=false;
			}
		}
	}

	lineData2->m_AutoExport=false;
	lineData1->m_Length=(lineData2->m_StartTime-lineData1->m_StartTime)+lineData2->m_Length;
	
	PopulateTable();
	OnSelChangeMsflexgrid1();
}
