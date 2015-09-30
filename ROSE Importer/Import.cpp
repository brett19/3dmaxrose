#include "ROSEImport.h"

struct FileToImport {
	FileToImport(int type, const TCHAR* file) : mType(type) { mFile = _wcsdup(file); }
	FileToImport() : mType(-1), mFile(NULL) {}
	~FileToImport(){ if(mFile) delete [] mFile; }

	int mType;
	TCHAR* mFile;
};

std::vector<FileToImport*> ImportList;
HTREEITEM hZmsRootNode;
HTREEITEM hZmdRootNode;
HTREEITEM hZmoRootNode;

R3E::RoseSkeleton* cCurrentSkeleton;

bool gResetScene;

static CZImporterClassDesc CZImporterDesc;
ClassDesc2* GetCZImporterDesc() { return &CZImporterDesc; }

HTREEITEM AddTreeItem(FileToImport* impFile, const TCHAR* text, HTREEITEM parent, HTREEITEM insertAfter){
	TV_INSERTSTRUCT nTreeItem;
	nTreeItem.item.pszText = const_cast<LPWSTR>(text);
	nTreeItem.hParent = parent;
	nTreeItem.item.mask = TVIF_TEXT;
	return (HTREEITEM)SendDlgItemMessage(GetActiveWindow(), IDC_TREE_FILES, TVM_INSERTITEM, 0, (LPARAM)&nTreeItem);
}

int GetFileType(const TCHAR* filename){
	size_t len = wcslen(filename);
	const TCHAR* type = filename + len - 3;
	if(!_wcsicmp(type, L"ZMS")) return ZZ_FORMAT_ZMS;
	if(!_wcsicmp(type, L"ZMD")) return ZZ_FORMAT_ZMD;
	if(!_wcsicmp(type, L"ZMO")) return ZZ_FORMAT_ZMO;
	return -1;
}

const TCHAR* GetFileNameFromPath(const TCHAR* fullpath){
	const TCHAR* ptr = wcsrchr(fullpath, '\\');
	return ptr + 1;
}

void AddToTreeView(FileToImport* impFile){
	switch(impFile->mType){
		case ZZ_FORMAT_ZMS:
			AddTreeItem(impFile, GetFileNameFromPath(impFile->mFile), hZmsRootNode, TVI_LAST);
			break;

		case ZZ_FORMAT_ZMD:
			AddTreeItem(impFile, GetFileNameFromPath(impFile->mFile), hZmdRootNode, TVI_LAST);
			break;

		case ZZ_FORMAT_ZMO:
			AddTreeItem(impFile, GetFileNameFromPath(impFile->mFile), hZmoRootNode, TVI_LAST);
			break;
	};
}

bool AddFileToList(const TCHAR* filename, bool addToTv = true){
	int type = GetFileType(filename);
	if(type == -1) return false;
	if(type != ZZ_FORMAT_ZMS){
		for(std::vector<FileToImport*>::const_iterator itr = ImportList.begin(); itr != ImportList.end(); ++itr)
			if((*itr)->mType == type) return false;
	}

	FileToImport* impFile = new FileToImport(type, filename);
	ImportList.push_back(impFile);
	if(addToTv)
		AddToTreeView(impFile);

	return true;
}

HTREEITEM gSelectedItem = 0;

INT_PTR CALLBACK CZImporterOptionsDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	static CZImporter *imp = NULL;

	switch(message) {
		case WM_INITDIALOG:
			imp = (CZImporter*)lParam;
			CenterWindow(hWnd, GetParent(hWnd));
			hZmsRootNode = AddTreeItem(NULL, L"Mesh", NULL, TVI_ROOT);
			hZmdRootNode = AddTreeItem(NULL, L"Skeleton", NULL, TVI_ROOT);
			hZmoRootNode = AddTreeItem(NULL, L"Animation", NULL, TVI_ROOT);
			cCurrentSkeleton = 0;
			if(ImportList.size() > 0)
				AddToTreeView(ImportList[0]);

			CheckRadioButton(hWnd, IDC_RADIO_SCENE_CREATE_NEW, IDC_RADIO_SCENE_ADD_CURRENT, IDC_RADIO_SCENE_CREATE_NEW);
			return TRUE;

		case WM_NOTIFY:
			if(wParam == IDC_TREE_FILES){
				LPNMTREEVIEW nmMsg = (LPNMTREEVIEW)lParam;
				if(nmMsg->hdr.code == TVN_SELCHANGED)
					gSelectedItem = nmMsg->itemNew.hItem;
			}
			return TRUE;

		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case IDC_BUTTON_ADD_FILE:
					{
						OPENFILENAME ofn;
						TCHAR szFileName[MAX_PATH] = L"";
						ZeroMemory(&ofn, sizeof(ofn));

						ofn.lStructSize = sizeof(ofn);
						ofn.hwndOwner = hWnd;
						ofn.lpstrFilter = L"ZnZin (*.ZMS;*.ZMD;*.ZMO)\0*.ZMS;*.ZMD;*.ZMO\0";
						ofn.lpstrFile = szFileName;
						ofn.nMaxFile = MAX_PATH;
						ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT;

						if(!GetOpenFileName(&ofn)) return TRUE;
						if(ofn.nFileOffset < lstrlen(szFileName)){
							if(!AddFileToList(szFileName))
								MessageBoxA(hWnd, "Only one Skeleton or Animation allowed", "Error", MB_ICONERROR);
						}else{
							TCHAR FilePath[MAX_PATH] = L"";
							TCHAR Message[MAX_PATH] = L"";
							wcscpy(FilePath, szFileName);
							FilePath[ofn.nFileOffset] = 0;
							while(szFileName[ofn.nFileOffset] != 0){
								wcscpy(Message, FilePath);
								wcscat(Message, L"\\" );
								wcscat(Message, szFileName+ofn.nFileOffset);
								if(!AddFileToList(Message))
									MessageBoxA(hWnd, "Only one Skeleton or Animation allowed", "Error", MB_ICONERROR);
								ofn.nFileOffset += (WORD)(wcslen(szFileName + ofn.nFileOffset) + 1);
							}
						}
					}
					return TRUE;

				case IDC_BUTTON_REMOVE_FILE:
					{
						if(!gSelectedItem) return TRUE;

						TV_ITEM tvi;
						TCHAR Text[255] = L"";
						memset(&tvi, 0, sizeof(tvi));

						tvi.mask = TVIF_TEXT;
						tvi.pszText = Text;
						tvi.cchTextMax = 256;
						tvi.hItem = gSelectedItem;

						if(!SendDlgItemMessage(hWnd, IDC_TREE_FILES, TVM_GETITEM, TVGN_CARET, (LPARAM)&tvi)) return TRUE;
						FileToImport* impFile;

						for(std::vector<FileToImport*>::const_iterator itr = ImportList.begin(); itr != ImportList.end(); ++itr){
							if(wcscmp(GetFileNameFromPath((*itr)->mFile), Text)) continue;
							impFile = (*itr);
							break;
						}

						if(!impFile) return TRUE;

						SendDlgItemMessage(hWnd, IDC_TREE_FILES, TVM_DELETEITEM, TVGN_CARET,(LPARAM)tvi.hItem);

						for(std::vector<FileToImport*>::const_iterator itr = ImportList.begin(); itr != ImportList.end(); ++itr){
							if(*itr != impFile) continue;
							ImportList.erase(itr);
							delete impFile;
							break;
						}
					}
					return TRUE;

				case IDC_BUTTON_CANCEL:
					EndDialog(hWnd, 0);
					return TRUE;

				case IDC_BUTTON_IMPORT:
					gResetScene = (IsDlgButtonChecked(hWnd, IDC_RADIO_SCENE_CREATE_NEW) == BST_CHECKED);
					EndDialog(hWnd, 1);
					return TRUE;
			};
			return TRUE;

		case WM_CLOSE:
			EndDialog(hWnd, 0);
			return TRUE;
	}
	return FALSE;
}


void ImportFromList(ImpInterface *i, Interface* inter, int cImportType){
	for(std::vector<FileToImport*>::const_iterator itr = ImportList.begin(); itr != ImportList.end(); ++itr){
		if((*itr)->mType != cImportType) continue;
		switch((*itr)->mType){
			case ZZ_FORMAT_ZMS:
				ImportMesh((*itr)->mFile, i, inter);
				break;
			case ZZ_FORMAT_ZMD:
				cCurrentSkeleton = ImportSkeleton((*itr)->mFile, i);
				return;
			case ZZ_FORMAT_ZMO:
				ImportAnimation((*itr)->mFile, i);
				return;
		};
	}
}

DWORD WINAPI fn (LPVOID arg){
	return(0);
}

int CZImporter::DoImport(const TCHAR* filename, ImpInterface *i, Interface *gi, BOOL suppressPrompts){
	DELVEC(ImportList);

	AddFileToList(filename, false);
	if(!DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ZZ_IMPORT_OPTIONS), GetActiveWindow(), CZImporterOptionsDlgProc, (LPARAM)this))
		return IMPEXP_CANCEL;

	gi->ProgressStart(L"Importing...", TRUE, fn, NULL);
	gi->ProgressUpdate(50);

	if(gResetScene){
		gi->FileReset(TRUE);
		i->NewScene();
	}

#ifdef USE_FBX_HAX
	Reset();
	for(std::vector<FileToImport*>::const_iterator itr = ImportList.begin(); itr != ImportList.end(); ++itr){
		switch((*itr)->mType){
			case ZZ_FORMAT_ZMS:
				{
					char* texture = _strdup((*itr)->mFile);
					char* pch = texture + strlen(texture) - 3;
					pch[0] = 'd';
					pch[1] = 'd';
					pch[2] = 's';
					FILE* fh;
					fopen_s(&fh, texture, "rb");
					if(!fh){
						delete [] texture;
						texture = NULL;
					}else{
						fclose(fh);
					}
					AddMesh((*itr)->mFile, texture);
					if(texture) delete [] texture;
				}
				break;
			case ZZ_FORMAT_ZMD:
				{
					SetSkeleton((*itr)->mFile);
				}
				break;
			case ZZ_FORMAT_ZMO:
				{
					AddAnimation((*itr)->mFile);
				}
				break;
		};
	}
	ImportZMX();
	ExportFBX("Rose_Import.fbx");
	gi->ImportFromFile("Rose_Import.fbx");
#else

	int cImportType = ZZ_FORMAT_ZMS;
	ImportFromList(i, gi, ZZ_FORMAT_ZMD);
	ImportFromList(i, gi, ZZ_FORMAT_ZMS);
	ImportFromList(i, gi, ZZ_FORMAT_ZMO);
#endif

	i->RedrawViews();
	gi->ProgressEnd();

	return IMPEXP_SUCCESS;
}
