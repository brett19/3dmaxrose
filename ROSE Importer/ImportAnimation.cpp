#include "ROSEImport.h"

bool ImportAnimation(const TCHAR* filename, ImpInterface *imp){
	R3E::RoseAnimation anim;
	if(!anim.Load(filename)) return false;

	SetFrameRate(anim.mFps);
	int ticksPerFrame = GetTicksPerFrame();

	Interval range;
	range.SetStart(0);
	range.SetEnd(ticksPerFrame * anim.mFrameCount);
	imp->SetAnimRange(range);

	for(int i = 0; i < anim.mChannelCount; ++i){
		R3E::RoseAnimation::Channel* curChan = anim.mChannels[i];
		INode* curBone = cCurrentSkeleton->mNodeBones[curChan->mLink]->GetINode();

		switch(curChan->mType){
				case R3E::RoseAnimation::CTYPE_POSITION:
					{
						Control* tmCont = curBone->GetTMController();
						Control* linpos = (Control*)CreateInstance(CTRL_POSITION_CLASS_ID, Class_ID(LININTERP_POSITION_CLASS_ID,0));
						tmCont->SetPositionController(linpos);
						IKeyControl* ikeys = GetKeyControlInterface(linpos);
						if(!ikeys) continue;
						R3E::RoseAnimation::ChannelPosition* curChanPos = (R3E::RoseAnimation::ChannelPosition*)curChan;
						for(int j = 0; j < anim.mFrameCount; ++j){
							ILinPoint3Key* linPosKey = new ILinPoint3Key();
							linPosKey->val = curChanPos->mPositions[j];
							linPosKey->time = j * ticksPerFrame;
							ikeys->AppendKey(linPosKey);
						}
				}
				break;
				case R3E::RoseAnimation::CTYPE_ROTATION:
					{
						Control* tmCont = curBone->GetTMController();
						Control* linrot = (Control*)CreateInstance(CTRL_ROTATION_CLASS_ID, Class_ID(LININTERP_ROTATION_CLASS_ID,0));
						tmCont->SetRotationController(linrot);
						IKeyControl* ikeys = GetKeyControlInterface(linrot);
						if(!ikeys) continue;
						R3E::RoseAnimation::ChannelRotation* curChanRot = (R3E::RoseAnimation::ChannelRotation*)curChan;
						for(int j = 0; j < anim.mFrameCount; ++j){
							ILinRotKey* linRotKey = new ILinRotKey();
							linRotKey->val = curChanRot->mRotations[j];
							linRotKey->val.w = -curChanRot->mRotations[j].w;
							linRotKey->time = j * ticksPerFrame;
							ikeys->AppendKey(linRotKey);
						}
				}
				/*
				case R3E::RoseAnimation::CTYPE_SCALE:
					{
						Control* tmCont = curBone->GetTMController();
						Control* linscale = (Control*)CreateInstance(CTRL_SCALE_CLASS_ID, Class_ID(LININTERP_SCALE_CLASS_ID,0));
						tmCont->SetScaleController(linscale);
						IKeyControl* ikeys = GetKeyControlInterface(linscale);
						if(!ikeys) continue;
						R3E::RoseAnimation::ChannelScale* curChanScale = (R3E::RoseAnimation::ChannelScale*)curChan;
						for(int j = 0; j < anim.mFrameCount; ++j){
							ILinScaleKey* linScaleKey = new ILinScaleKey();
							linScaleKey->val.s.x = curChanScale->mScale[j];
							linScaleKey->val.s.y = curChanScale->mScale[j];
							linScaleKey->val.s.z = curChanScale->mScale[j];
							linScaleKey->time = j * ticksPerFrame;
							ikeys->AppendKey(linScaleKey);
						}
					}
				*/
				break;
				default:
					continue;
		};
	}

	return true;
}