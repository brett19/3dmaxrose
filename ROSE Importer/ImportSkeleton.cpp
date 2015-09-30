#include "ROSEImport.h"

R3E::RoseSkeleton* ImportSkeleton(const TCHAR* filename, ImpInterface *imp){
	R3E::RoseSkeleton* cSkeleton = new R3E::RoseSkeleton();
	if(!cSkeleton->Load(filename)) return false;
	cSkeleton->mNodeBones.SetCount(cSkeleton->mBones.size());
	for(int i = 0; i < cSkeleton->mBoneCount + cSkeleton->mDummyCount; ++i){
		ImpNode* nBoneNode = imp->CreateNode();
		INode* iNode = nBoneNode->GetINode();
		iNode->SetBoneNodeOnOff(TRUE, 0);
		iNode->SetRenderable(FALSE);
		iNode->ShowBone(1);
		
		Object* obj;
		if(i < cSkeleton->mBoneCount)
			obj = (Object*)CreateInstance(GEOMOBJECT_CLASS_ID, BONE_OBJ_CLASSID);
		else
			obj = (Object*)CreateInstance(HELPER_CLASS_ID, Class_ID(DUMMY_CLASS_ID, 0));

		nBoneNode->Reference(obj);

		if(cSkeleton->mBones[i]->mParent != i)
			cSkeleton->mNodeBones[cSkeleton->mBones[i]->mParent]->GetINode()->AttachChild(iNode);

		Matrix3 nodeTM;
		cSkeleton->mBones[i]->mRotation.MakeMatrix(nodeTM);
		nodeTM.SetTrans(cSkeleton->mBones[i]->mPosition);
		
		nBoneNode->SetTransform(0, nodeTM);
		nBoneNode->SetName(cSkeleton->mBones[i]->mName);

		imp->AddNodeToScene(nBoneNode);

		cSkeleton->mNodeBones[i] = nBoneNode;
	}

	return cSkeleton;
}
