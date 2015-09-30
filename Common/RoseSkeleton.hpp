namespace R3E {
	namespace RoseAttachPoints {
		enum RoseAttachPoints {
			AP_RIGHT_HAND,
			AP_LEFT_HAND,
			AP_SHIELD,
			AP_BACK,
			AP_MOUTH,
			AP_EYES,
			AP_FOREHEAD,
			AP_HEAD,
			AP_NORMAL_BONE,
		};

#define ISNORMALBONE(x) ((x >> 8) != 0)
#define SETNORMALBONE(x) (R3E::RoseAttachPoints::AP_NORMAL_BONE | (x << 8))
#define GETNORMALBONE(x) (x >> 8)
	}

	struct RoseBone {
		RoseBone() : mName(NULL) {}
		~RoseBone(){
			DELARR(mName);
		}

		bool mIsDummy;
		int mParent;
		char* mName;
		Point3 mPosition;
		Quat mRotation;
	};

	class RoseSkeleton {
	public:
		RoseSkeleton() : mBones(0){}
		~RoseSkeleton(){
			DELVEC(mBones);
		}

		void CalculatePositionMatrix(int index){
			if(index > 0){
				Matrix3 nodeTM;
				mBones[mBones[index]->mParent]->mRotation.MakeMatrix(nodeTM);
				mBones[index]->mPosition = nodeTM.PointTransform(mBones[index]->mPosition);
				mBones[index]->mPosition += mBones[mBones[index]->mParent]->mPosition;
				mBones[index]->mRotation *= mBones[mBones[index]->mParent]->mRotation;
				mBones[index]->mRotation.Normalize();
			}
			ProcessChildren(index);
		}

		void ProcessChildren(int parent){
			for(int i = 0; i < mBoneCount + mDummyCount; ++i){
				if(i == parent) continue;
				if(mBones[i]->mParent != parent) continue;
				CalculatePositionMatrix(i);
			}
		}

		bool Save(const char* path){
			BufferedFile<> fh;
			if(!fh.OpenWrite(path)) return false;

			fh << FixLengthString("ZMD0003", 7);
			fh << mBoneCount;
			for(int i = 0; i < mBoneCount; ++i){
				RoseBone* cBone = mBones[i];
				fh << cBone->mParent;
				fh << TerminatedString(&cBone->mName, 0);
				fh << cBone->mPosition;
				cBone->mRotation.w = -cBone->mRotation.w;
				fh << cBone->mRotation;
			}


			fh << mDummyCount;
			for(int i = mBoneCount; i < mBoneCount + mDummyCount; ++i){
				RoseBone* cBone = mBones[i];
				fh << TerminatedString(&cBone->mName, 0);
				fh << cBone->mParent;
				fh << cBone->mPosition;
				cBone->mRotation.w = -cBone->mRotation.w;
				fh << cBone->mRotation;
			}

			fh.Close();
			return true;
		}

		bool Load(const char* path){
			BufferedFile<> fh;
			if(!fh.OpenRead(path)) return false;

			char VersionHeader[7];
			fh >> FixLengthString(VersionHeader, 7);
			int version;
			if(!strncmp(VersionHeader, "ZMD0002", 7)){
				version = 2;
			}else if(!strncmp(VersionHeader, "ZMD0003", 7)){
				version = 3;
			}else{
				fh.Close();
				return false;
			}

			fh >> mBoneCount;
			for(int i = 0; i < mBoneCount; ++i){
				RoseBone* nBone = new RoseBone();
				nBone->mIsDummy = false;
				fh >> nBone->mParent;
				fh >> TerminatedString(&nBone->mName, 0, 32);
				fh >> nBone->mPosition;
				fh >> nBone->mRotation;
				nBone->mRotation.w = -nBone->mRotation.w;
				mBones.push_back(nBone);
			}

			fh >> mDummyCount;
			for(int i = mBoneCount; i < mBoneCount + mDummyCount; ++i){
				RoseBone* nBone = new RoseBone();
				nBone->mIsDummy = false;
				fh >> TerminatedString(&nBone->mName, 0, 32);
				fh >> nBone->mParent;
				fh >> nBone->mPosition;
				if(version == 3)
					fh >> nBone->mRotation;
				nBone->mRotation.w = -nBone->mRotation.w;
				mBones.push_back(nBone);
			}

			fh.Close();

			CalculatePositionMatrix(0);

			return true;
		}

		int mBoneCount;
		int mDummyCount;
		std::vector<RoseBone*> mBones;

		Tab<ImpNode*> mNodeBones;
	};
};
