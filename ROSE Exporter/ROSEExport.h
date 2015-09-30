/**********************************************************************
 *<
	FILE: ZnZinExporter.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#ifndef __ZnZinExporter__H
#define __ZnZinExporter__H

#include <Max.h>
#include <istdplug.h>
#include <iparamb2.h>
#include <iparamm2.h>
#include <iskin.h>
#include <modstack.h>
#include <STDMAT.H>
#include <vector>
#include <list>
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

enum {
	ZZ_FORMAT_ZMS,
	ZZ_FORMAT_ZMD,
	ZZ_FORMAT_ZMO,
	ZZ_FORMAT_COUNT,
};

struct BoneInfo {
	BoneInfo(INode* node, TCHAR* name) : mNode(node), mName(name) {}

	INode* mNode;
	TCHAR* mName;
};

extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;
extern void ExportMesh(INode* node, Interface* gi);
extern void ExportSkeleton(Interface* gi);
extern void ExportAnimation(Interface* gi);
extern int GetBoneIndex(INode* boneNode);
extern void MakeBoneList(Interface* gi);
extern std::vector<BoneInfo> gBoneList;
extern std::list<std::string> gDummyList;

#define ZnZinExporter_CLASS_ID	Class_ID(0x6ea77f4d, 0xa7b89266)

class ZnZinExporter : public SceneExport {
public:
	static HWND hParams;

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

	BOOL SupportsOptions(int ext, DWORD options);
	int DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);

	ZnZinExporter();
	~ZnZinExporter();	
};

class ZnZinExporterClassDesc : public ClassDesc2 {
public:
	int IsPublic(){ return TRUE; }
	void* Create(BOOL loading = FALSE){ return new ZnZinExporter(); }
	const TCHAR* ClassName(){ return GetString(IDS_CLASS_NAME); }
	SClass_ID SuperClassID(){ return SCENE_EXPORT_CLASS_ID; }
	Class_ID ClassID(){ return ZnZinExporter_CLASS_ID; }
	const TCHAR* Category(){ return GetString(IDS_CATEGORY); }

	const TCHAR* InternalName(){ return _T("ZnZinExporter"); }
	HINSTANCE HInstance(){ return hInstance; }	
};

#endif
