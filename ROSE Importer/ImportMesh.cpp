#include "ROSEImport.h"
#include <MeshNormalSpec.h> 
#include <MNNormalSpec.h> 
#include <iEditNormals.h> 
#define EDIT_NORMALS_CLASS_ID Class_ID(0x4aa52ae3, 0x35ca1cde)
#define MAX_MAPS 4

bool ImportMesh(const TCHAR* filename, ImpInterface* imp, Interface* inter){
	TriObject* object = CreateNewTriObject();
	if(!object){
		delete object;
		return false;
	}

	Mesh* mesh = &object->GetMesh();

	R3E::RoseMesh cMesh;
	if(!cMesh.Load(filename)) return false;

	mesh->setNumVerts(cMesh.mVertCount);
	//mesh->setNumTVerts(cMesh.mVertCount);

	mesh->setNumFaces(cMesh.mFaceCount);
	//mesh->setNumTVFaces(cMesh.mFaceCount);

	for(int i = 0; i < cMesh.mVertCount; i++){
		mesh->setVert(i, cMesh.mVertPos[i]);
		//mesh->setNormal(i, cMesh.mVertNorm[i]);
	}

	int map_indices[] = { 1, 2, 3, 4 };
	for(int i = 0; i < MAX_MAPS; ++i){
		if(!cMesh.mVertUV[i]) break;
		/*mesh->setMapSupport(map_indices[i], true);
		mesh->setNumMapVerts(map_indices[i], cMesh.mVertCount);
		mesh->setNumMapFaces(map_indices[i], cMesh.mFaceCount);
		for(int j = 0; j < cMesh.mVertCount; j++){
			UVVert uv(cMesh.mVertUV[i][j].x, cMesh.mVertUV[i][j].y, .0f);
			mesh->setMapVert(map_indices[i], j, uv);
		}*/

		mesh->setMapSupport(map_indices[i]);
		mesh->maps[map_indices[i]].setNumVerts(cMesh.mVertCount);
		for( int j = 0; j < cMesh.mVertCount; ++j){
			mesh->maps[map_indices[i]].tv[j].x = cMesh.mVertUV[i][j].x;
			mesh->maps[map_indices[i]].tv[j].y = cMesh.mVertUV[i][j].y;
		}
	}

	int facePos = 0;
	for(int i = 0; i < cMesh.mFaceCount; ++i){
		int first = cMesh.mFaces[facePos];++facePos;
		int second = cMesh.mFaces[facePos];++facePos;
		int third = cMesh.mFaces[facePos];++facePos;

		mesh->faces[i].setVerts(first, second, third);
		//mesh->faces[i].setEdgeVisFlags(1, 1, 1);

		/*for(int j = 0; j < MAX_MAPS; ++j){
			if(!cMesh.mVertUV[j]) break;
			TVFace* tvFace = mesh->mapFaces(map_indices[j]);
			tvFace[i].setTVerts(first, second, third);
		}*/
	}

	int mc = mesh->getNumMaps();
	for(int i = 1; i < mc; ++i){
		mesh->maps[i].setNumFaces(cMesh.mFaceCount);
		for(int j = 0; j < cMesh.mFaceCount; ++j){
			mesh->maps[i].tf[j].t[0] = mesh->faces[j].v[0];
			mesh->maps[i].tf[j].t[1] = mesh->faces[j].v[1];
			mesh->maps[i].tf[j].t[2] = mesh->faces[j].v[2];
		}
	}

	ImpNode* node = imp->CreateNode();
	if(!node){
		delete object;
		return false;
	}

	Matrix3 matIdentity;
	matIdentity.IdentityMatrix();
	node->Reference(object);
	node->SetTransform(0, matIdentity);

	INode* Node = node->GetINode();
	Object* Obj = object;
	IDerivedObject *DerObj = NULL;
	if(Obj->SuperClassID() == GEN_DERIVOB_CLASS_ID){
		DerObj = static_cast<IDerivedObject*>(Obj);	  
	}else{ 
		DerObj = CreateDerivedObject();  
		DerObj->TransferReferences(Obj);  
		DerObj->ReferenceObject(Obj);  
	}

	if(cCurrentSkeleton && cMesh.mFormat & R3E::RMF_BONEINDICES && cMesh.mFormat & R3E::RMF_BONEWEIGHTS){
		Modifier* SkinMod = (Modifier*)CreateInstance(OSM_CLASS_ID, SKIN_CLASSID);
		ISkinImportData* SkinImport = (ISkinImportData*)SkinMod->GetInterface(I_SKINIMPORTDATA);
		ModContext* Skeletoncontext = new ModContext(new Matrix3(1), NULL, NULL );
		DerObj->AddModifier(SkinMod, Skeletoncontext);

		bool* isAdded = new bool[cCurrentSkeleton->mBoneCount];
		for(int j = 0; j < cCurrentSkeleton->mBoneCount; ++j)
			isAdded[j] = false;

		for(int i = 0; i < cMesh.mVertCount; ++i){
			for(int j = 0; j < 4; ++j){
				if(*((dword*)&cMesh.mVertBones[i].mWeights[j]) == 0) continue;
				if(!isAdded[cMesh.mVertBones[i].mBones[j]]){
					SkinImport->AddBoneEx(cCurrentSkeleton->mNodeBones[cMesh.mVertBones[i].mBones[j]]->GetINode(), FALSE);
					isAdded[cMesh.mVertBones[i].mBones[j]] = true;
				}
			}
		}

		Node->EvalWorldState(0);

		for(int i = 0; i < cMesh.mVertCount; ++i){
			Tab<float> weights;
			Tab<INode*> bones;
			INode* boneNodes[4];
			for(int j = 0; j < 4; ++j){
				if(*((dword*)&cMesh.mVertBones[i].mWeights[j]) == 0) continue;
				weights.Append(1, &cMesh.mVertBones[i].mWeights[j]);
				boneNodes[j] = cCurrentSkeleton->mNodeBones[cMesh.mVertBones[i].mBones[j]]->GetINode();
				bones.Append(1, &boneNodes[j]);
			}
			SkinImport->AddWeights(Node, i, bones, weights);
		}

		delete [] isAdded;
	}

	/*Modifier* EditNormalsMod = (Modifier*)CreateInstance(OSM_CLASS_ID, EDIT_NORMALS_CLASS_ID);
	ModContext* EditNormalsModContext = new ModContext(new Matrix3(1), NULL, NULL );
	DerObj->AddModifier(EditNormalsMod, EditNormalsModContext, DerObj->NumModifiers());
	//IEditNormalsMod* pIMod = (IEditNormalsMod*)EditNormalsMod->GetInterface(EDIT_NORMALS_MOD_INTERFACE);

	mesh->SpecifyNormals();
	MeshNormalSpec* nmMesh = mesh->GetSpecifiedNormals();
	nmMesh->SetParent(mesh);
	nmMesh->SetNumFaces(cMesh.mFaceCount);
	nmMesh->SetNumNormals(cMesh.mVertCount);

	facePos = 0;
	for(int i = 0; i < cMesh.mFaceCount; ++i){
		int first = cMesh.mFaces[facePos];++facePos;
		int second = cMesh.mFaces[facePos];++facePos;
		int third = cMesh.mFaces[facePos];++facePos;
		nmMesh->SetNormal(i, 0, cMesh.mVertNorm[first]);
		nmMesh->SetNormal(i, 1, cMesh.mVertNorm[second]);
		nmMesh->SetNormal(i, 2, cMesh.mVertNorm[third]);
	}

	mesh->buildBoundingBox();
	mesh->InvalidateEdgeList();
	mesh->InvalidateTopologyCache();
	mesh->InvalidateGeomCache();*/

	mesh->InvalidateTopologyCache( );
	mesh->InvalidateGeomCache( );
	mesh->InvalidateEdgeList( );
	mesh->buildBoundingBox( );
	mesh->buildNormals( );

	if(cMesh.mFormat & R3E::RMF_NORMALS){
		mesh->SpecifyNormals();
		MeshNormalSpec* nmMesh = mesh->GetSpecifiedNormals();
		nmMesh->SetParent(mesh);
		nmMesh->CheckNormals();
		nmMesh->AverageNormals(true, 0.1f, false, NULL);
	}

	{
		char* texture = _strdup(filename);
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
			BitmapTex* btex = NewDefaultBitmapTex();
			btex->SetMapName(texture);

			StdMat* mtl = NewDefaultStdMat();
			mtl->SetSubTexmap(ID_DI, btex);
			mtl->SetActiveTexmap(btex);

			Node->SetMtl(mtl);

			inter->ActivateTexture(btex, mtl);

			delete [] texture;
		}
	}

	imp->AddNodeToScene(node);
	node->SetName(GetFileNameFromPath(filename));

	return true;
}
