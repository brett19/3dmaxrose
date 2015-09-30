#include "ROSEExport.h"
#include <MeshNormalSpec.h> 
#include <MNNormalSpec.h> 
#include <iEditNormals.h> 
#define EDIT_NORMALS_CLASS_ID Class_ID(0x4aa52ae3, 0x35ca1cde)
#define MAX_MAPS 4

struct MyVert {
	int pos_idx;
	int norm_idx;
	int uv_idx[4];

	Point3 pos;
	Point3 norm;
	Point2 uv[4];

	R3E::BoneWeightData bones;
};

struct MyFace {
	int v[3];
};

int PushVert( MyVert* verts, int& numverts, const MyVert& newvert ) {
	for( int i = 0; i < numverts; i++ ) {
		if( verts[i].pos_idx == newvert.pos_idx &&
			verts[i].norm_idx == newvert.norm_idx &&
			verts[i].uv_idx[0] == newvert.uv_idx[0] &&
			verts[i].uv_idx[1] == newvert.uv_idx[1] &&
			verts[i].uv_idx[2] == newvert.uv_idx[2] &&
			verts[i].uv_idx[3] == newvert.uv_idx[3] ) {
				return i;
		}
	}
	verts[numverts] = newvert;
	return numverts++;
};

void ExportMesh(INode* node, Interface* gi){
	Object* obj = node->EvalWorldState(0).obj;
	if(!obj){
		DBMSG("obj == NULL");
		return;
	}

	TriObject* tri = (TriObject*)obj->ConvertToType(0, Class_ID(TRIOBJ_CLASS_ID, 0));
	if(!tri){
		DBMSG("tri == NULL");
		return;
	}

	Mesh* triMesh = &tri->GetMesh();
	if(!triMesh){
		DBMSG("triMesh == NULL");
		return;
	}




	bool bonesused = false;
	bool mapused[4] = { false, false, false, false }; for( int i = 0; i < triMesh->numMaps - 1; i++ ) { mapused[i] = triMesh->maps[i+1].IsUsed(); }
	int numverts = 0;
	int numfaces = triMesh->getNumFaces( );

	char txt[300];
	sprintf( txt, "Maps Used: %d %d %d %d", mapused[0]?1:0, mapused[1]?1:0, mapused[2]?1:0, mapused[3]?1:0 );
	DBMSG( txt );


	sprintf( txt, "Faces: %d", numfaces );
	DBMSG( txt );

	MyFace* faces = new MyFace[ numfaces ];
	MyVert* verts = new MyVert[ numfaces * 3 ];


	DBMSG( "Begin of Export" );


	Object* Obj = node->GetObjectRef();
	if(!Obj){
		DBMSG("Obj == 0");
		return;
	}

	IDerivedObject *DerObj = NULL;
	if(Obj->SuperClassID() == GEN_DERIVOB_CLASS_ID){
		DerObj = static_cast<IDerivedObject*>(Obj);	  
	}else{ 
		DerObj = CreateDerivedObject();  
		DerObj->TransferReferences(tri);  
		DerObj->ReferenceObject(tri);  
	}

	DBMSG( "Got Object" );

	ISkin* TehSkin = 0;
	ISkinContextData* SkinData = 0;

	int modCount = DerObj->NumModifiers();
	for(int i = 0; i < modCount; ++i){
		Modifier* CurMod = DerObj->GetModifier(i);
		if(CurMod->ClassID() == SKIN_CLASSID){
			TehSkin = (ISkin*)CurMod->GetInterface(I_SKIN);
			SkinData = TehSkin->GetContextInterface(node);
		}
	}

	if( TehSkin != 0 && SkinData != 0 ) bonesused = true;

	sprintf( txt, "Prepared Skin Data - Bones?: %d", bonesused?1:0 );
	DBMSG( txt );

	MeshNormalSpec* nmMesh = triMesh->GetSpecifiedNormals();
	if(!nmMesh){
		triMesh->SpecifyNormals();
		nmMesh = triMesh->GetSpecifiedNormals();
		nmMesh->BuildNormals( );
	}

	DBMSG( "Completed Normals" );

	for( int i = 0; i < numfaces; i++ ) {
		for( int j = 0; j < 3; j++ ) {
			MyVert v;

			memset(&v.bones, 0, sizeof(R3E::BoneWeightData));

			v.pos_idx = triMesh->faces[i].v[j];
			v.pos = triMesh->verts[ v.pos_idx ];

			v.norm_idx = nmMesh->GetNormalIndex( i, j );
			v.norm = nmMesh->Normal( v.norm_idx );

			for( int k = 0; k < 4; k++ ) {
				if( !mapused[k] ) {
					v.uv_idx[k] = 0;
					continue;
				}

				v.uv_idx[k] = triMesh->maps[k+1].tf[i].t[j];
				v.uv[k].x = triMesh->maps[k+1].tv[ v.uv_idx[k] ].x;
				v.uv[k].y = triMesh->maps[k+1].tv[ v.uv_idx[k] ].y;
			}

			if( bonesused ) {
				for(int k = 0; k < SkinData->GetNumAssignedBones(v.pos_idx); ++k){
					int boneIdx = GetBoneIndex(TehSkin->GetBone(SkinData->GetAssignedBone(v.pos_idx, k)));
					if(boneIdx != -1){
						v.bones.mBones[k] = boneIdx;
						v.bones.mWeights[k] = SkinData->GetBoneWeight(v.pos_idx, k);
					}
				}
			}

			faces[i].v[j] = PushVert( verts, numverts, v );
		}
	}


	DBMSG( "Preparing Export!" );
	
	R3E::RoseMesh mesh;
	mesh.mFormat = R3E::RMF_POSITIONS | R3E::RMF_NORMALS;
	mesh.mVertCount = numverts;
	mesh.mFaceCount = numfaces;
	mesh.mBoundingBox = triMesh->getBoundingBox();

	if(!mesh.mVertCount){
		DBMSG("mesh.mVertCount == 0");
		return;
	}

	if(!mesh.mFaceCount){
		DBMSG("mesh.mFaceCount == 0");
		return;
	}

	mesh.mVertPos = new Point3[mesh.mVertCount];
	mesh.mVertNorm = new Point3[mesh.mVertCount];
	for(int i = 0; i < mesh.mVertCount; i++){
		mesh.mVertPos[i] = verts[i].pos;
		mesh.mVertNorm[i] = verts[i].norm;
	}

	int facePos = 0;
	mesh.mFaces = new short[mesh.mFaceCount * 3];
	for(int i = 0; i < mesh.mFaceCount; ++i){
		mesh.mFaces[facePos] = faces[i].v[0]; ++facePos;
		mesh.mFaces[facePos] = faces[i].v[1]; ++facePos;
		mesh.mFaces[facePos] = faces[i].v[2]; ++facePos;
	}


	for( int i = 0; i < 4; i++ ) {
		if( !mapused[i] ) continue;

		mesh.mFormat |= R3E::RMF_UVMAP1 << i;
		mesh.mVertUV[i] = new Point2[ mesh.mVertCount ];
		for( int j = 0; j < mesh.mVertCount; j++ ) {
			mesh.mVertUV[i][j] = verts[j].uv[i];
		}
	}

	if( bonesused ) {
		mesh.mFormat |= R3E::RMF_BONEINDICES | R3E::RMF_BONEWEIGHTS;
		mesh.mVertBones = new R3E::BoneWeightData[ mesh.mVertCount ];
		
		for( int i = 0; i < mesh.mVertCount; i++ ) {
			mesh.mVertBones[i] = verts[i].bones;
		}
	}

	delete[] faces;
	delete[] verts;

	DBMSG( "Completed Export" );

	TCHAR* meshName = node->GetName();
	mesh.Save(meshName);

	DBMSG( "Completed Saving" );
}
