
#ifndef	__COLLISIONSPEC_H
#define	__COLLISIONSPEC_H

#include <iostream>
#include <list>
#include <windows.h>

#ifdef _WIN32
using namespace std;
#endif

struct FieldEntry
{
	char	name[512];						// Can't rely on either CString or MString being present
};

struct FieldHeader
{
	unsigned int Hash;
	list<FieldEntry>::iterator it;
	list<FieldEntry> FieldEntries;
};

class	CCollisionSpec
{
		public:

		private:

		bool	bScriptLoaded;

		list<FieldHeader> FieldList;

		bool	FindToken  (char *&,char *&,int &,int);
		bool	FindMember (char *&,char *&,int &,int);

		char	StringWrk[1024];

		public:

		 CCollisionSpec();
		~CCollisionSpec();

		void	FieldReset		   (char *);
		bool	FieldGet		   (char *,char *&);
		bool	ReadCollisionScript();

		bool			FillCollisionSpec	 (HWND,int *);
		void			SetCollisionDlgValue (HWND,int *,unsigned int);
		unsigned int	GetCollisionDlgValue (HWND ,int *);
		char		   *GetCollisionString   (unsigned int,int);
		bool			CreateIncludeFile	 (const char *);
};

#define	CF_GETSURFACETYPE	0xff
#define	CF_GETMAPPINGTYPE	0x300
#define	CF_BITATTRSTART		10

#define	CF_MAPPINGSHIFT		8

#define	CMC_GETSURFACETYPE(v) ( v & CF_GETSURFACETYPE)
#define	CMC_GETMAPPINGTYPE(v) ((v & CF_GETMAPPINGTYPE) >> CF_MAPPINGSHIFT)

#define	CMC_MAKESURFACETYPE(v) (v & CF_GETSURFACETYPE)
#define	CMC_MAKEMAPPINGTYPE(v) ((v << CF_MAPPINGSHIFT) & CF_GETMAPPINGTYPE)

extern	CCollisionSpec	CollideSpec;

#endif	//__COLLISIONSPEC_H