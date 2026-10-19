// Vegas EDL Parser.h : main header file for the VEGAS EDL PARSER application
//

#if !defined(AFX_VEGASEDLPARSER_H__FAE5F04D_8DD0_4F33_9A56_257BFC3A0AAB__INCLUDED_)
#define AFX_VEGASEDLPARSER_H__FAE5F04D_8DD0_4F33_9A56_257BFC3A0AAB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CVegasEDLParserApp:
// See Vegas EDL Parser.cpp for the implementation of this class
//

class CVegasEDLParserApp : public CWinApp
{
public:
	CVegasEDLParserApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVegasEDLParserApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CVegasEDLParserApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VEGASEDLPARSER_H__FAE5F04D_8DD0_4F33_9A56_257BFC3A0AAB__INCLUDED_)
