#include "ROSEImport.h"

CZImporter::CZImporter(){
}

CZImporter::~CZImporter(){
}

int CZImporter::ExtCount(){
	return ZZ_FORMAT_COUNT;
}

const TCHAR *CZImporter::Ext(int n){		
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

const TCHAR *CZImporter::LongDesc(){
	return _T("ZnZin Engine Files");
}
	
const TCHAR *CZImporter::ShortDesc(){
	return _T("ZnZin");
}

const TCHAR *CZImporter::AuthorName(){
	return _T("ExJam");
}

const TCHAR *CZImporter::CopyrightMessage(){
	return _T("Copyright ExJam :)");
}

const TCHAR *CZImporter::OtherMessage1(){
	return _T("");
}

const TCHAR *CZImporter::OtherMessage2(){
	return _T("");
}

unsigned int CZImporter::Version(){
	return 100;
}

void CZImporter::ShowAbout(HWND hWnd){			
}

