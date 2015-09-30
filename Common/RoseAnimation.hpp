namespace R3E {
	class RoseAnimation {
	public:
		enum {
			CTYPE_NONE = (1 << 0),
			CTYPE_POSITION = (1 << 1),
			CTYPE_ROTATION = (1 << 2),
			CTYPE_NORMAL = (1 << 3),
			CTYPE_ALPHA = (1 << 4),
			CTYPE_UV0 = (1 << 5),
			CTYPE_UV1 = (1 << 6),
			CTYPE_UV2 = (1 << 7),
			CTYPE_UV3 = (1 << 8),
			CTYPE_TEXTUREANIM = (1 << 9),
			CTYPE_SCALE = (1 << 10),
		};

		struct Channel {
			Channel(){}
			~Channel(){}

			int mLink;
			int mType;

			virtual void Init(int frames) = 0;
			virtual void Delete() = 0;
			virtual void LoadFrame(int frame, BufferedFile<>& fh) = 0;
			virtual void SaveFrame(int frame, BufferedFile<>& fh) = 0;
		};

		struct ChannelPosition : public Channel {
			ChannelPosition() : mPositions(0) {}
			~ChannelPosition(){ DELARR(mPositions); }

			Point3* mPositions;

			void Init(int frames){
				mPositions = new Point3[frames];
			}

			void LoadFrame(int frame, BufferedFile<>& fh){
				fh >> mPositions[frame];
			}

			void SaveFrame(int frame, BufferedFile<>& fh){
				fh << mPositions[frame];
			}

			void Delete(){
				delete this;
			}
		};

		struct ChannelRotation : public Channel {
			ChannelRotation() : mRotations(0) {}
			~ChannelRotation(){ DELARR(mRotations); }

			Quat* mRotations;

			void Init(int frames){
				mRotations = new Quat[frames];
			}

			void LoadFrame(int frame, BufferedFile<>& fh){
				fh >> mRotations[frame];
			}

			void SaveFrame(int frame, BufferedFile<>& fh){
				fh << mRotations[frame];
			}

			void Delete(){
				delete this;
			}
		};

		struct ChannelNormal : public Channel {
			ChannelNormal() : mNormals(0) {}
			~ChannelNormal(){ DELARR(mNormals); }

			Point3* mNormals;

			void Init(int frames){
				mNormals = new Point3[frames];
			}

			void LoadFrame(int frame, BufferedFile<>& fh){
				fh >> mNormals[frame];
			}

			void SaveFrame(int frame, BufferedFile<>& fh){
				fh << mNormals[frame];
			}

			void Delete(){
				delete this;
			}
		};

		struct ChannelAlpha : public Channel {
			ChannelAlpha() : mAlphas(0) {}
			~ChannelAlpha(){ DELARR(mAlphas); }

			float* mAlphas;

			void Init(int frames){
				mAlphas = new float[frames];
			}

			void LoadFrame(int frame, BufferedFile<>& fh){
				fh >> mAlphas[frame];
			}

			void SaveFrame(int frame, BufferedFile<>& fh){
				fh << mAlphas[frame];
			}

			void Delete(){
				delete this;
			}
		};

		struct ChannelUV0 : public Channel {
			ChannelUV0() : mUV0s(0) {}
			~ChannelUV0(){ DELARR(mUV0s); }

			Point2* mUV0s;

			void Init(int frames){
				mUV0s = new Point2[frames];
			}

			void LoadFrame(int frame, BufferedFile<>& fh){
				fh >> mUV0s[frame];
			}

			void SaveFrame(int frame, BufferedFile<>& fh){
				fh << mUV0s[frame];
			}

			void Delete(){
				delete this;
			}
		};

		struct ChannelUV1 : public Channel {
			ChannelUV1() : mUV1s(0) {}
			~ChannelUV1(){ DELARR(mUV1s); }

			Point2* mUV1s;

			void Init(int frames){
				mUV1s = new Point2[frames];
			}

			void LoadFrame(int frame, BufferedFile<>& fh){
				fh >> mUV1s[frame];
			}

			void SaveFrame(int frame, BufferedFile<>& fh){
				fh << mUV1s[frame];
			}

			void Delete(){
				delete this;
			}
		};

		struct ChannelUV2 : public Channel {
			ChannelUV2() : mUV2s(0) {}
			~ChannelUV2(){ DELARR(mUV2s); }

			Point2* mUV2s;

			void Init(int frames){
				mUV2s = new Point2[frames];
			}

			void LoadFrame(int frame, BufferedFile<>& fh){
				fh >> mUV2s[frame];
			}

			void SaveFrame(int frame, BufferedFile<>& fh){
				fh << mUV2s[frame];
			}

			void Delete(){
				delete this;
			}
		};

		struct ChannelUV3 : public Channel {
			ChannelUV3() : mUV3s(0) {}
			~ChannelUV3(){ DELARR(mUV3s); }

			Point2* mUV3s;

			void Init(int frames){
				mUV3s = new Point2[frames];
			}

			void LoadFrame(int frame, BufferedFile<>& fh){
				fh >> mUV3s[frame];
			}

			void SaveFrame(int frame, BufferedFile<>& fh){
				fh << mUV3s[frame];
			}

			void Delete(){
				delete this;
			}
		};

		struct ChannelTexture : public Channel {
			ChannelTexture() : mTextures(0) {}
			~ChannelTexture(){ DELARR(mTextures); }

			float* mTextures;

			void Init(int frames){
				mTextures = new float[frames];
			}

			void LoadFrame(int frame, BufferedFile<>& fh){
				fh >> mTextures[frame];
			}

			void SaveFrame(int frame, BufferedFile<>& fh){
				fh << mTextures[frame];
			}

			void Delete(){
				delete this;
			}
		};

		struct ChannelScale : public Channel {
			ChannelScale() : mScale(0) {}
			~ChannelScale(){ DELARR(mScale); }

			float* mScale;

			void Init(int frames){
				mScale = new float[frames];
			}

			void LoadFrame(int frame, BufferedFile<>& fh){
				fh >> mScale[frame];
			}

			void SaveFrame(int frame, BufferedFile<>& fh){
				fh << mScale[frame];
			}

			void Delete(){
				delete this;
			}
		};

	public:
		RoseAnimation() : mChannels(0) {}
		~RoseAnimation(){
			for(INT i = 0; i < mChannelCount; ++i)
				mChannels[i]->Delete();

			DELARR(mChannels);
		}

		Channel* CreateChannel(int type){
			switch(type){
				case CTYPE_POSITION:		return new ChannelPosition;
				case CTYPE_ROTATION:		return new ChannelRotation;
				case CTYPE_NORMAL:			return new ChannelNormal;
				case CTYPE_ALPHA:				return new ChannelAlpha;
				case CTYPE_UV0:					return new ChannelUV0;
				case CTYPE_UV1:					return new ChannelUV1;
				case CTYPE_UV2:					return new ChannelUV2;
				case CTYPE_UV3:					return new ChannelUV3;
				case CTYPE_TEXTUREANIM:	return new ChannelTexture;
				case CTYPE_SCALE:				return new ChannelScale;
				default: return NULL;
			};
		}

		bool Save(const char* path){
			BufferedFile<> fh;
			if(!fh.OpenWrite(path)) return false;
			char* header = "ZMO0002\x0";
			fh << FixLengthString(header, 8);
			fh << mFps;
			fh << mFrameCount;
			fh << mChannelCount;

			for(int i = 0; i < mChannelCount; ++i){
				fh << mChannels[i]->mType;
				fh << mChannels[i]->mLink;
			}

			for(int i = 0; i < mFrameCount; ++i){
				for(int j = 0; j < mChannelCount; ++j){
					mChannels[j]->SaveFrame(i, fh);
				}
			}

			fh.Close();
			return true;
		}

		bool Load(const char* path){
			BufferedFile<> fh;
			if(!fh.OpenRead(path)) return false;
			char header[8];
			fh >> FixLengthString(header, 8);
			if(strncmp(header, "ZMO0002", 7)){
				fh.Close();
				return false;
			}

			fh >> mFps;
			fh >> mFrameCount;
			fh >> mChannelCount;

			mChannels = new Channel*[mChannelCount];
			for(int i = 0; i < mChannelCount; ++i){
				int type;
				fh >> type;
				mChannels[i] = CreateChannel(type);

				if(!mChannels[i]){
					fh.Close();
					return false;
				}

				mChannels[i]->mType = type;
				fh >> mChannels[i]->mLink;
				mChannels[i]->Init(mFrameCount);
			}

			for(int i = 0; i < mFrameCount; ++i){
				for(int j = 0; j < mChannelCount; ++j){
					mChannels[j]->LoadFrame(i, fh);
				}
			}

			fh.Close();
			return true;
		}

		int mFps;
		int mFrameCount;
		int mChannelCount;

		Channel** mChannels;
	};
};