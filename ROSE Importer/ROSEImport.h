/**********************************************************************
 *<
	FILE: 3dsMax ZnZin Importer.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#ifndef __CZImporter__H
#define __CZImporter__H

//#define USE_FBX_HAX
#ifdef USE_FBX_HAX
#pragma comment(lib, "ZMxConverter.lib")
#define ZDLLEXPORT __declspec(dllimport) _cdecl
extern "C" {
	void ZDLLEXPORT Reset();

	bool ZDLLEXPORT ImportFBX(char* path);
	bool ZDLLEXPORT ExportZMX();

	bool ZDLLEXPORT SetSkeleton(char* path);
	bool ZDLLEXPORT AddMesh(char* path, char* texture = 0);
	bool ZDLLEXPORT AddAnimation(char* path);
	bool ZDLLEXPORT ImportZMX();
	bool ZDLLEXPORT ExportFBX(char* path);
}
#endif

#include <Max.h>
#include <istdplug.h>
#include <iparamb2.h>
#include <iparamm2.h>
#include <iskin.h>
#include <modstack.h>
#include <STDMAT.H>

#include <direct.h>
#include <commdlg.h>
#include <commctrl.h>
#include <vector>
#include "resource.h"

typedef unsigned __int8 byte;
typedef unsigned __int16 word;
typedef unsigned __int32 dword;
typedef unsigned __int64 qword;

#define DEL(x) {if(x) delete x; x = 0;}
#define DELARR(x) {if(x) delete [] x; x = 0;}
#define DELVEC(x) {for(unsigned int i = 0; i < x.size(); ++i){ if(x[i]){ delete x[i]; } } x.clear();}

#define MB_PROGRESS() {char buffer[32]; sprintf_s(buffer, 32, "Line %i", __LINE__); MessageBoxA(NULL, buffer, __FILE__, 0); }
#define DBMSG(x) {MessageBoxA(NULL, x, "Debug", MB_ICONINFORMATION);}

#include "..\Common\BufferedFile.hpp"
#include "..\Common\RoseMesh.hpp"
#include "..\Common\RoseSkeleton.hpp"
#include "..\Common\RoseAnimation.hpp"

extern bool ImportMesh(const TCHAR* filename, ImpInterface* imp, Interface* inter);
extern R3E::RoseSkeleton* ImportSkeleton(const TCHAR* filename, ImpInterface *imp);
extern bool ImportAnimation(const TCHAR* filename, ImpInterface *imp);
extern R3E::RoseSkeleton* cCurrentSkeleton;
extern const TCHAR* GetFileNameFromPath(const TCHAR* fullpath);

extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;

enum {
	ZZ_FORMAT_ZMS,
	ZZ_FORMAT_ZMD,
	ZZ_FORMAT_ZMO,
	ZZ_FORMAT_COUNT,
};

#define CZImporter_CLASS_ID	Class_ID(0xbe3761e1, 0x644d30ee)

class CZImporter : public SceneImport {
public:
	static HWND hParams;

	CZImporter();
	~CZImporter();

	int ExtCount();
	const TCHAR* Ext(int n);
	const TCHAR* LongDesc();
	const TCHAR* ShortDesc();
	const TCHAR* AuthorName();
	const TCHAR* CopyrightMessage();
	const TCHAR* OtherMessage1();
	const TCHAR* OtherMessage2();
	unsigned int Version();
	void ShowAbout(HWND hWnd);
	int DoImport(const TCHAR *name,ImpInterface *i,Interface *gi, BOOL suppressPrompts=FALSE);	// Import file
};

class CZImporterClassDesc : public ClassDesc2 {
public:
	int IsPublic(){ return TRUE; }
	void* Create(BOOL loading = FALSE){ return new CZImporter(); }
	const TCHAR* ClassName(){ return GetString(IDS_CLASS_NAME); }
	SClass_ID SuperClassID(){ return SCENE_IMPORT_CLASS_ID; }
	Class_ID ClassID(){ return CZImporter_CLASS_ID; }
	const TCHAR* Category(){ return GetString(IDS_CATEGORY); }

	const TCHAR* InternalName(){ return _T("CZImporter"); }
	HINSTANCE HInstance(){ return hInstance; }	
};

#endif
