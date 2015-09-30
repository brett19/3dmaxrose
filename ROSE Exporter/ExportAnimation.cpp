#include "ROSEExport.h"

int gStartTime;

TimeValue GetTimeByFrame(int frame){
	return gStartTime + (frame * GetTicksPerFrame());
}

void ExportAnimation(Interface* gi){
	gStartTime = gi->GetAnimRange().Start();

	R3E::RoseAnimation motion;
	motion.mFps = GetFrameRate();
	motion.mFrameCount = (gi->GetAnimRange().End() - gStartTime) / GetTicksPerFrame();

	motion.mChannelCount = (int)gBoneList.size() - (int)gDummyList.size();
	motion.mChannels = new R3E::RoseAnimation::Channel*[motion.mChannelCount];

	{
		motion.mChannels[0] = new R3E::RoseAnimation::ChannelPosition();
		motion.mChannels[0]->mLink = 0;
		motion.mChannels[0]->mType = R3E::RoseAnimation::CTYPE_POSITION;

		R3E::RoseAnimation::ChannelPosition* posChan = (R3E::RoseAnimation::ChannelPosition*)motion.mChannels[0];
		posChan->mPositions = new Point3[motion.mFrameCount];

		INode* curBone = gBoneList[motion.mChannels[0]->mLink].mNode;
		Control* c = curBone->GetTMController()->GetPositionController();
		Interval ivalid = FOREVER;
		for(int j = 0; j < motion.mFrameCount; ++j){
			TimeValue t = GetTimeByFrame(j);
			ObjectState os = curBone->EvalWorldState(t);
			Matrix3 tmp = curBone->GetParentTM(t);
			posChan->mPositions[j] = curBone->GetNodeTM(t).GetTrans();
		}
	}

	for(int i = 1; i < motion.mChannelCount; ++i){
		motion.mChannels[i] = new R3E::RoseAnimation::ChannelRotation();
		motion.mChannels[i]->mLink = i - 1;
		motion.mChannels[i]->mType = R3E::RoseAnimation::CTYPE_ROTATION;

		R3E::RoseAnimation::ChannelRotation* rotChan = (R3E::RoseAnimation::ChannelRotation*)motion.mChannels[i];
		rotChan->mRotations = new Quat[motion.mFrameCount];

		INode* curBone = gBoneList[motion.mChannels[i]->mLink].mNode;

		Control* c = curBone->GetTMController()->GetRotationController();
		Interval ivalid = FOREVER;
		for(int j = 0; j < motion.mFrameCount; ++j){
			TimeValue t = GetTimeByFrame(j);
			ObjectState os = curBone->EvalWorldState(t);

			if(i != 1){
				Matrix3 parentWTM, nodeWTM;
				nodeWTM = curBone->GetNodeTM(t);
				parentWTM = curBone->GetParentNode()->GetNodeTM(t);
				nodeWTM.NoScale();
				parentWTM.NoScale();
				rotChan->mRotations[j] = nodeWTM * Inverse(parentWTM);
			}else{
				rotChan->mRotations[j] = curBone->GetNodeTM(t);
			}

			rotChan->mRotations[j].w = -rotChan->mRotations[j].w;
		}
	}

	motion.Save("Animation.zmo");
}
