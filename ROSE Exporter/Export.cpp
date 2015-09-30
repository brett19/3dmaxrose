#include "ROSEExport.h"
Interface* gi;
std::vector<char*> gExportList;

INT_PTR CALLBACK ZnZinExporterOptionsDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static ZnZinExporter *imp = NULL;
	static int LastMsgBox = IDC_SCENE_LIST;

	switch(message) {
		case WM_INITDIALOG:
		{
			bool hasSkel = false;
			imp = (ZnZinExporter*)lParam;
			CenterWindow(hWnd, GetParent(hWnd));

			INode* rootNode = gi->GetRootNode();
			for(int i = 0; i < rootNode->NumberOfChildren(); ++i){
				INode* node = rootNode->GetChildNode(i);
				ObjectState os = node->EvalWorldState(0); 
				if(!os.obj) continue;
				if(os.obj->SuperClassID() != GEOMOBJECT_CLASS_ID) continue;
				if(os.obj->ClassID() == BONE_OBJ_CLASSID){
					SendDlgItemMessage(hWnd, IDC_SCENE_LIST, LB_ADDSTRING, 0, (LPARAM)"Skeleton");
					hasSkel = true;
				}else{
					if(os.obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
						SendDlgItemMessage(hWnd, IDC_SCENE_LIST, LB_ADDSTRING, 0, (LPARAM)node->GetName());
				}
			}
			if(hasSkel)
				SendDlgItemMessage(hWnd, IDC_SCENE_LIST, LB_ADDSTRING, 0, (LPARAM)"Animation");
			return TRUE;
		}

		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case IDC_SCENE_LIST:
					LastMsgBox = IDC_SCENE_LIST;
					break;

				case IDC_EXPORT_LIST:
					LastMsgBox = IDC_EXPORT_LIST;
					break;

				case IDC_BUTTON_ADD_FILE:
				{
					int selID = SendDlgItemMessage(hWnd, LastMsgBox, LB_GETCURSEL, 0, 0);
					char buffer[128];
					int selString = SendDlgItemMessage(hWnd, LastMsgBox, LB_GETTEXT, (WPARAM)selID, (LPARAM)buffer);
					if(selString == LB_ERR) break;
					SendDlgItemMessage(hWnd, (LastMsgBox == IDC_EXPORT_LIST)?IDC_SCENE_LIST:IDC_EXPORT_LIST, LB_ADDSTRING, 0, (LPARAM)buffer);
					SendDlgItemMessage(hWnd, LastMsgBox, LB_DELETESTRING, (WPARAM)selID, (LPARAM)0);
				}
				break;

				case IDC_BUTTON_EXPORT:
				{
					int exportCount = SendDlgItemMessage(hWnd, IDC_EXPORT_LIST, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
					for(int i = 0; i < exportCount; ++i){
						char buffer[128];
						int selString = SendDlgItemMessage(hWnd, IDC_EXPORT_LIST, LB_GETTEXT, (WPARAM)i, (LPARAM)buffer);
						if(selString == LB_ERR) continue;
						gExportList.push_back(_strdup(buffer));
					}
					EndDialog(hWnd, 1);
				}
				break;

				case IDC_BUTTON_CANCEL:
					EndDialog(hWnd, 0);
					break;

				default:
					return FALSE;
			};
			return TRUE;

		case WM_CLOSE:
			EndDialog(hWnd, 0);
			return TRUE;
	};

	return 0;
}

INode* FindNode(char* name, Interface* gi){
	bool findBone = false;
	if(!_strcmpi(name, "Animation")) return (INode*)1;
	if(!_strcmpi(name, "Skeleton")) return (INode*)2;

	INode* rootNode = gi->GetRootNode();
	for(int i = 0; i < rootNode->NumberOfChildren(); ++i){
		INode* node = rootNode->GetChildNode(i);
		ObjectState os = node->EvalWorldState(0); 
		if(!os.obj) continue;
		if(os.obj->SuperClassID() != GEOMOBJECT_CLASS_ID) continue;
		if(os.obj->ClassID() == BONE_OBJ_CLASSID) continue;
		if(os.obj->ClassID() == Class_ID(DUMMY_CLASS_ID, 0)) continue;
		if(!os.obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) continue;
		if(!_strcmpi(node->GetName(), name)) return node;
	}

	return NULL;
}

DWORD WINAPI fn (LPVOID arg){
	return 0;
}

void ExportType(int etype){
	for(std::vector<char*>::const_iterator itr = gExportList.begin(); itr != gExportList.end(); ++itr){
		INode* node = FindNode(*itr, gi);
		if(node == 0) continue;
		if(node == (INode*)1){
			if(etype == ZZ_FORMAT_ZMO) ExportAnimation(gi);
			else continue;
		}else if(node == (INode*)2){
			if(etype == ZZ_FORMAT_ZMD) ExportSkeleton(gi);
			else continue;
		}else if(etype == ZZ_FORMAT_ZMS){
			ExportMesh(node, gi);
		}
	}
}

int	ZnZinExporter::DoExport(const TCHAR* name, ExpInterface* ei, Interface* i, BOOL suppressPrompts, DWORD options){
	gi = i;
	if(!DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ZZ_EXPORT_OPTIONS), GetActiveWindow(), ZnZinExporterOptionsDlgProc, (LPARAM)this) || gExportList.size() == 0)
		return IMPEXP_CANCEL;

	gi->ProgressStart("Exporting...", TRUE, fn, NULL);
	gi->ProgressUpdate(50);

	MakeBoneList(gi);

	ExportType(ZZ_FORMAT_ZMS);
	ExportType(ZZ_FORMAT_ZMD);
	ExportType(ZZ_FORMAT_ZMO);

	for(std::vector<char*>::const_iterator itr = gExportList.begin(); itr != gExportList.end(); ++itr) delete [] *itr;
	gExportList.clear();
	gBoneList.clear();

	gi->ProgressEnd();

	return IMPEXP_SUCCESS;
}
