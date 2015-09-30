#include "ROSEExport.h"

#include <string>
#include <list>
#include <map>

std::vector<BoneInfo> gBoneList;

std::list<std::string> gDummyList;
std::map<std::string, INode*> gDummyMap;

void AddBoneAndChildren(INode* boneNode){
	std::list<std::string> childList;
	std::map<std::string, INode*> childMap;

	{
		ObjectState os = boneNode->EvalWorldState(0);
		if(os.obj->ClassID() == BONE_OBJ_CLASSID){
			gBoneList.push_back(BoneInfo(boneNode, boneNode->GetName()));
		}else{
			if(os.obj->SuperClassID() == HELPER_CLASS_ID || os.obj->ClassID() == Class_ID(DUMMY_CLASS_ID, 0)){
				gDummyMap[boneNode->GetName()] = boneNode;
				gDummyList.push_back(boneNode->GetName());
			}
		}
	}

	for(int i = 0; i < boneNode->NumberOfChildren(); ++i){
		INode* node = boneNode->GetChildNode(i);
		ObjectState os = node->EvalWorldState(0);
		if(!os.obj) continue;
		if(os.obj->SuperClassID() != HELPER_CLASS_ID && os.obj->ClassID() != Class_ID(DUMMY_CLASS_ID, 0) && os.obj->ClassID() != BONE_OBJ_CLASSID) continue;
		childMap[boneNode->GetChildNode(i)->GetName()] = boneNode->GetChildNode(i);
		childList.push_back(boneNode->GetChildNode(i)->GetName());
	}

	childList.sort();

	for(std::list<std::string>::const_iterator itr = childList.begin(); itr != childList.end(); ++itr){
		AddBoneAndChildren(childMap[*itr]);
	}
}

void MakeBoneList(Interface* gi){
	gBoneList.clear();
	gDummyList.clear();
	gDummyMap.clear();

	INode* boneNode = NULL;
	INode* rootNode = gi->GetRootNode();
	for(int i = 0; i < rootNode->NumberOfChildren(); ++i){
		INode* node = rootNode->GetChildNode(i);
		ObjectState os = node->EvalWorldState(0); 
		if(!os.obj) continue;
		if(os.obj->SuperClassID() != GEOMOBJECT_CLASS_ID) continue;
		if(os.obj->ClassID() == BONE_OBJ_CLASSID){
			boneNode = node;
			break;
		}
	}
	if(!boneNode) return;

	AddBoneAndChildren(boneNode);
	gDummyList.sort();
	for(std::list<std::string>::const_iterator itr = gDummyList.begin(); itr != gDummyList.end(); ++itr){
		gBoneList.push_back(BoneInfo(gDummyMap[*itr], gDummyMap[*itr]->GetName()));
	}
}

int GetBoneIndex(INode* node){
	for(int i = 0; i < gBoneList.size(); ++i){
		if(gBoneList[i].mNode == node) return i;
	}
	return -1;
}

void ExportSkeleton(Interface* gi){	
	if(gBoneList.size() == 0) return;
	R3E::RoseSkeleton skeleton;
	skeleton.mBoneCount = 0;
	skeleton.mDummyCount = 0;

	for(int i = 0; i < gBoneList.size(); ++i){
		INode* curBone = gBoneList[i].mNode;
		ObjectState os = curBone->EvalWorldState(0);

		R3E::RoseBone* cBone = new R3E::RoseBone();
		cBone->mIsDummy = (os.obj->ClassID() != BONE_OBJ_CLASSID);
		cBone->mName = _strdup(curBone->GetName());
		cBone->mParent = (i == 0)?0:GetBoneIndex(curBone->GetParentNode());

		if(!cBone->mIsDummy) ++skeleton.mBoneCount;
		else ++skeleton.mDummyCount;

		Matrix3 nodeWTM = curBone->GetNodeTM(0);
		nodeWTM.NoScale();

		if(i == 0){
			cBone->mPosition = nodeWTM.GetTrans();
			cBone->mRotation = nodeWTM;
		}else{
			Matrix3 localTM = nodeWTM * Inverse(curBone->GetParentNode()->GetNodeTM(0));
			cBone->mPosition = localTM.GetTrans();
			cBone->mRotation = localTM;
		}

		skeleton.mBones.push_back(cBone);
	}

	skeleton.Save("Skeleton.ZMD");
}
