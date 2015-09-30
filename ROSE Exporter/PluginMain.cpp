#include "ROSEExport.h"

static ZnZinExporterClassDesc ZnZinExporterDesc;
ClassDesc2* GetZnZinExporterDesc(){
	return &ZnZinExporterDesc;
}

ZnZinExporter::ZnZinExporter(){
}

ZnZinExporter::~ZnZinExporter(){

}

int ZnZinExporter::ExtCount(){
	return ZZ_FORMAT_COUNT;
}

const TCHAR *ZnZinExporter::Ext(int n){		
	switch(n){
		case ZZ_FORMAT_ZMS:
			return _T("ZMS");
			break;
		case ZZ_FORMAT_ZMD:
			return _T("ZMD");
			break;
		case ZZ_FORMAT_ZMO:
			return _T("ZMO");
			break;
	};

	return _T("");
}


const TCHAR *ZnZinExporter::LongDesc(){
	return _T("ZnZin Engine Files");
}

const TCHAR *ZnZinExporter::ShortDesc(){
	return _T("ZnZin");
}

const TCHAR *ZnZinExporter::AuthorName(){
	return _T("ExJam");
}

const TCHAR *ZnZinExporter::CopyrightMessage(){
	return _T("Copyright ExJam :)");
}

const TCHAR *ZnZinExporter::OtherMessage1(){
	return _T("");
}

const TCHAR *ZnZinExporter::OtherMessage2(){
	return _T("");
}

unsigned int ZnZinExporter::Version(){
	return 100;
}

void ZnZinExporter::ShowAbout(HWND hWnd){
}

BOOL ZnZinExporter::SupportsOptions(int ext, DWORD options){
	return TRUE;
}
