namespace R3E {
	struct BoneWeightData {
		float mWeights[4];
		int mBones[4];
	};

	enum RoseMeshFormat {
		//RMF_INVALID = 1,
		RMF_POSITIONS = 2,
		RMF_NORMALS = 4,
		RMF_COLORS = 8,
		RMF_BONEINDICES = 16,
		RMF_BONEWEIGHTS = 32,
		RMF_TANGENTS = 64,
		RMF_UVMAP1 = 128,
		RMF_UVMAP2 = 256,
		RMF_UVMAP3 = 512,
		RMF_UVMAP4 = 1024,
	};

	class RoseMesh {
	public:
		RoseMesh(){
			mVertBones = 0;
			mFaces = 0;
			mVertUV[0] = 0;
			mVertUV[1] = 0;
			mVertUV[2] = 0;
			mVertUV[3] = 0;
			mVertPos = 0;
			mVertNorm = 0;
		}

		~RoseMesh(){
			DELARR(mVertBones);
			DELARR(mFaces);
			DELARR(mVertUV[0]);
			DELARR(mVertUV[1]);
			DELARR(mVertUV[2]);
			DELARR(mVertUV[3]);
			DELARR(mVertPos);
			DELARR(mVertNorm);
		}

		bool Load(const char* path){
			BufferedFile<> fh;
			if(!fh.OpenRead(path)) return false;
			int version;
			char VersionHeader[8];
			fh >> FixLengthString(VersionHeader, 8);
			if(strcmp(VersionHeader, "ZMS0008")){
				version = 8;
			}else if(strcmp(VersionHeader, "ZMS0007")){
				version = 7;
			}else if(strcmp(VersionHeader, "ZMS0006")){
				version = 6;
			}else if(strcmp(VersionHeader, "ZMS0005")){
				version = 5;
			}else{
				fh.Close();
				return false;
			}

			if(version >= 7) LoadMesh8(fh);
			else LoadMesh6(fh);

			fh.Close();

			return true;
		}

		bool Save(const char* path){
			BufferedFile<> fh;
			if(!fh.OpenWrite(path)) return false;
			char* ver = "ZMS0008";
			fh << TerminatedString(&ver, 0, 255);
			fh << mFormat;

			mBoundingBox.pmin *= 0.01f;
			mBoundingBox.pmax *= 0.01f;
			fh << mBoundingBox;

			byte boneTable[64];
			std::vector<short> boneLookup;
			if(mFormat & RMF_BONEINDICES && mFormat & RMF_BONEWEIGHTS){
				memset(boneTable, 0xFF, 64);
				for(int i = 0; i < mVertCount; ++i){
					for(int j = 0; j < 4; ++j){
						if(mVertBones[i].mWeights[j] > 0.0f){
							if(boneTable[mVertBones[i].mBones[j]] != 0xFF) continue;
							boneTable[mVertBones[i].mBones[j]] = (byte)boneLookup.size();
							boneLookup.push_back(mVertBones[i].mBones[j]);
						}
					}
				}

				short boneCount = (short)boneLookup.size();
				fh << boneCount;
				for(int i = 0; i < boneCount; ++i){
					short boneId = boneLookup[i];
					fh << boneId;
				}
			}else{
				short boneCount = 0;
				fh << boneCount;
			}

			fh << mVertCount;
			if(mFormat & RMF_POSITIONS){
				for(int i = 0; i < mVertCount; ++i){
					mVertPos[i] *= 0.01f;
					fh << mVertPos[i];
				}
			}

			if(mFormat & RMF_NORMALS){
				for(int i = 0; i < mVertCount; ++i)
					fh << mVertNorm[i];
			}

			if(mFormat & RMF_BONEINDICES && mFormat & RMF_BONEWEIGHTS){
				for(int i = 0; i < mVertCount; ++i){
					fh << mVertBones[i].mWeights[0];
					fh << mVertBones[i].mWeights[1];
					fh << mVertBones[i].mWeights[2];
					fh << mVertBones[i].mWeights[3];
					for(int j = 0; j < 4; ++j){
						short tmpBone;
						if(mVertBones[i].mWeights[j] > 0.0f) tmpBone = boneTable[mVertBones[i].mBones[j]];
						else tmpBone = 0;
						fh << tmpBone;
					}
				}
			}

			if(mFormat & RMF_UVMAP1){
				for(int i = 0; i < mVertCount; ++i){
					mVertUV[0][i].y = 1.0f - mVertUV[0][i].y;
					fh << mVertUV[0][i];
				}
			}

			if(mFormat & RMF_UVMAP2){
				for(int i = 0; i < mVertCount; ++i){
					mVertUV[1][i].y = 1.0f - mVertUV[1][i].y;
					fh << mVertUV[1][i];
				}
			}

			if(mFormat & RMF_UVMAP3){
				for(int i = 0; i < mVertCount; ++i){
					mVertUV[2][i].y = 1.0f - mVertUV[2][i].y;
					fh << mVertUV[2][i];
				}
			}

			if(mFormat & RMF_UVMAP4){
				for(int i = 0; i < mVertCount; ++i){
					mVertUV[3][i].y = 1.0f - mVertUV[3][i].y;
					fh << mVertUV[3][i];
				}
			}

			fh << mFaceCount;
			int faceSize = mFaceCount * 3;
			for(int i = 0; i < faceSize; ++i)
				fh << mFaces[i];

			short stripCount = 0;
			fh << stripCount;

			short matType = 0;
			fh << matType;

			short somethingElseThatIForgot = 0;
			fh << somethingElseThatIForgot;

			fh.Close();
			return true;
		}

	private:
		bool LoadMesh6(BufferedFile<>& fh){
			int tmpInt;
			fh >> mFormat;
			fh >> mBoundingBox;

			int boneCount;
			fh >> boneCount;
			int* boneLookup = new int[boneCount];
			for(int i = 0; i < boneCount; ++i){
				fh.Skip(4);
				fh >> boneLookup[i];
			}

			fh >> tmpInt;
			mVertCount = tmpInt;
			if(mFormat & RMF_POSITIONS){
				mVertPos = new Point3[mVertCount];
				for(int i = 0; i < mVertCount; ++i){
					fh.Skip(4);
					fh >> mVertPos[i].z;
					fh >> mVertPos[i].y;
					fh >> mVertPos[i].x;
					mVertPos[i] *= 100.0f;
				}
			}

			if(mFormat & RMF_NORMALS){
				mVertNorm = new Point3[mVertCount];
				for(int i = 0; i < mVertCount; ++i){
					fh.Skip(4);
					fh >> mVertNorm[i];
				}
			}

			if(mFormat & RMF_COLORS)
				fh.Skip(mVertCount * ((sizeof(float) * 4) + sizeof(int)));

			if(mFormat & RMF_BONEINDICES && mFormat & RMF_BONEWEIGHTS){
				mVertBones = new BoneWeightData[mVertCount];

				for(int i = 0; i < mVertCount; ++i){
					fh.Skip(4);
					fh >> mVertBones[i].mWeights[0];
					fh >> mVertBones[i].mWeights[1];
					fh >> mVertBones[i].mWeights[2];
					fh >> mVertBones[i].mWeights[3];

					int tmpBone;
					fh >> tmpBone; mVertBones[i].mBones[0] = boneLookup[tmpBone];
					fh >> tmpBone; mVertBones[i].mBones[1] = boneLookup[tmpBone];
					fh >> tmpBone; mVertBones[i].mBones[2] = boneLookup[tmpBone];
					fh >> tmpBone; mVertBones[i].mBones[3] = boneLookup[tmpBone];
				}
			}

			if(mFormat & RMF_TANGENTS)
				fh.Skip(mVertCount * ((sizeof(float) * 3) + sizeof(int)));

			if(mFormat & RMF_UVMAP1){
				mVertUV[0] = new Point2[mVertCount];
				for(int i = 0; i < mVertCount; ++i){
					fh.Skip(4);
					fh >> mVertUV[0][i];
					mVertUV[0][i].y = 1.0f - mVertUV[0][i].y;
				}
			}

			if(mFormat & RMF_UVMAP2){
				mVertUV[1] = new Point2[mVertCount];
				for(int i = 0; i < mVertCount; ++i){
					fh.Skip(4);
					fh >> mVertUV[1][i];
					mVertUV[1][i].y = 1.0f - mVertUV[1][i].y;
				}
			}

			if(mFormat & RMF_UVMAP3){
				mVertUV[2] = new Point2[mVertCount];
				for(int i = 0; i < mVertCount; ++i){
					fh.Skip(4);
					fh >> mVertUV[2][i];
					mVertUV[2][i].y = 1.0f - mVertUV[2][i].y;
				}
			}

			if(mFormat & RMF_UVMAP4){
				mVertUV[3] = new Point2[mVertCount];
				for(int i = 0; i < mVertCount; ++i){
					fh.Skip(4);
					fh >> mVertUV[3][i];
					mVertUV[3][i].y = 1.0f - mVertUV[3][i].y;
				}
			}

			fh >> tmpInt;
			mFaceCount = tmpInt;
			int faceSize = mFaceCount * 3;
			mFaces = new short[faceSize];
			for(int i = 0; i < faceSize; ++i){
				fh >> tmpInt;
				mFaces[i] = tmpInt;
			}

			DELARR(boneLookup);
			return true;
		}

		bool LoadMesh8(BufferedFile<>& fh){
			fh >> mFormat;
			fh >> mBoundingBox;

			short boneCount;
			fh >> boneCount;
			short* boneLookup = new short[boneCount];
			for(int i = 0; i < boneCount; ++i)
				fh >> boneLookup[i];

			fh >> mVertCount;
			if(mFormat & RMF_POSITIONS){
				mVertPos = new Point3[mVertCount];
				for(int i = 0; i < mVertCount; ++i){
					fh >> mVertPos[i];
					mVertPos[i] *= 100.0f;
				}
			}

			if(mFormat & RMF_NORMALS){
				mVertNorm = new Point3[mVertCount];
				for(int i = 0; i < mVertCount; ++i)
					fh >> mVertNorm[i];
			}

			if(mFormat & RMF_COLORS)
				fh.Skip(mVertCount * sizeof(float) * 4);

			if(mFormat & RMF_BONEINDICES && mFormat & RMF_BONEWEIGHTS){
				mVertBones = new BoneWeightData[mVertCount];

				for(int i = 0; i < mVertCount; ++i){
					fh >> mVertBones[i].mWeights[0];
					fh >> mVertBones[i].mWeights[1];
					fh >> mVertBones[i].mWeights[2];
					fh >> mVertBones[i].mWeights[3];
					short tmpBone;
					fh >> tmpBone; mVertBones[i].mBones[0] = boneLookup[tmpBone];
					fh >> tmpBone; mVertBones[i].mBones[1] = boneLookup[tmpBone];
					fh >> tmpBone; mVertBones[i].mBones[2] = boneLookup[tmpBone];
					fh >> tmpBone; mVertBones[i].mBones[3] = boneLookup[tmpBone];
				}
			}

			if(mFormat & RMF_TANGENTS){
				fh.Skip(mVertCount * sizeof(float) * 3);
			}

			if(mFormat & RMF_UVMAP1){
				mVertUV[0] = new Point2[mVertCount];
				for(int i = 0; i < mVertCount; ++i){
					fh >> mVertUV[0][i];
					mVertUV[0][i].y = 1.0f - mVertUV[0][i].y;
				}
			}

			if(mFormat & RMF_UVMAP2){
				mVertUV[1] = new Point2[mVertCount];
				for(int i = 0; i < mVertCount; ++i){
					fh >> mVertUV[1][i];
					mVertUV[1][i].y = 1.0f - mVertUV[1][i].y;
				}
			}

			if(mFormat & RMF_UVMAP3){
				mVertUV[2] = new Point2[mVertCount];
				for(int i = 0; i < mVertCount; ++i){
					fh >> mVertUV[2][i];
					mVertUV[2][i].y = 1.0f - mVertUV[2][i].y;
				}
			}

			if(mFormat & RMF_UVMAP4){
				mVertUV[3] = new Point2[mVertCount];
				for(int i = 0; i < mVertCount; ++i){
					fh >> mVertUV[3][i];
					mVertUV[3][i].y = 1.0f - mVertUV[3][i].y;
				}
			}

			fh >> mFaceCount;
			int faceSize = mFaceCount * 3;
			mFaces = new short[faceSize];
			for(int i = 0; i < faceSize; ++i)
				fh >> mFaces[i];

			DELARR(boneLookup);
			return true;
		}

	public:
		int mFormat;

		Box3 mBoundingBox;
		short mVertCount;
		BoneWeightData* mVertBones;

		Point3* mVertPos;
		Point3* mVertNorm;

		Point2* mVertUV[4];

		short mFaceCount;
		short* mFaces;
	};
};