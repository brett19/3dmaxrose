namespace R3E {
	struct FixLengthString {
		FixLengthString(char* string, int length) : mString(string), mLength(length){}

		char* mString;
		int mLength;
	};

	struct TerminatedString {
		TerminatedString(char** string, char terminator, int maxLength = 255) : mString(string), mTerminator(terminator), mMaxLength(maxLength){}

		char** mString;
		char mTerminator;
		int mMaxLength;
	};

	template <class tVarLength> struct VarLengthString {
		VarLengthString(char** string) : mString(string) {}

		tVarLength mLength;
		char** mString;
	};

	struct HiBitString {
		HiBitString(char** string) : mString(string), mLength(0) {}

		int mLength;
		char** mString;
	};

	static const char* BufferedFileErrorText[] =  {
		"File is not open!",
		"Read past End of File!",
		"Buffer is too small!",
	};

#define ReadWholeFile(FILEPATH, MAHBUFFOR) { \
	FILE* fh; \
	fopen_s(&fh, FILEPATH, "rb"); \
	MAHBUFFOR = NULL; \
	if(fh){ \
	fseek(fh, 0, SEEK_END); \
	int MAHSIZE = ftell(fh); \
	fseek(fh, 0, SEEK_SET); \
	MAHBUFFOR = new char[MAHSIZE+1]; \
	fread(MAHBUFFOR, 1, MAHSIZE, fh); \
	MAHBUFFOR[MAHSIZE] = 0; \
	fclose(fh); \
	} \
		}

	template <int tBufferSize = 1024>
	class BufferedFile {
	public:
		BufferedFile() : mFh(NULL){}
		~BufferedFile(){
			if(mFh)
				Close();
		}

		bool __forceinline OpenRead(const char* path){
			mRead = true;
			return Open(path, "rb");
		}

		bool __forceinline OpenWrite(const char* path){
			mRead = false;
			return Open(path, "wb");
		}

		bool Open(const char* path, char* method){
			fopen_s(&mFh, path, method);
			if(mFh == NULL) return false;

			mCurrentReadPosition = 0;
			mCurrentBufferSize = 0;
			mCurrentBufferPosition = 0;

			if(mRead)
				CheckBytesAvailable(-1);

			return true;
		}

		void Close(){
			fclose(mFh);
			mFh = 0;
		}

		template <class tType> BufferedFile<tBufferSize>& operator>>(tType& value){
			if(!CheckBytesAvailable(sizeof(tType))) return *this;
			memcpy_s(&value, sizeof(tType), mBuffer + mCurrentBufferPosition, sizeof(tType));
			mCurrentBufferPosition += sizeof(tType);
			return *this;
		}

		template <> BufferedFile<tBufferSize>& operator>>(FixLengthString& value){
			if(!CheckBytesAvailable(value.mLength)) return *this;
			memcpy_s(value.mString, value.mLength, mBuffer + mCurrentBufferPosition, value.mLength);
			mCurrentBufferPosition += value.mLength;
			return *this;
		}

		template <> BufferedFile<tBufferSize>& operator>>(TerminatedString& value){
			if(!CheckBytesAvailable(value.mMaxLength)) return *this;
			char* pch = (char*)memchr((void*)(mBuffer + mCurrentBufferPosition), value.mTerminator, value.mMaxLength);
			int stringLength = (int)( (pch - (mBuffer + mCurrentBufferPosition)) + 1 );
			(*value.mString) = new char[stringLength];
			memcpy_s((*value.mString), value.mMaxLength, mBuffer + mCurrentBufferPosition, stringLength);
			mCurrentBufferPosition += stringLength;
			return *this;
		}

		template <> BufferedFile<tBufferSize>& operator>>(Point2& value){
			if(!CheckBytesAvailable(sizeof(value.x) * 2)) return *this;
			*this >> value.x >> value.y;
			return *this;
		}

		template <> BufferedFile<tBufferSize>& operator>>(Point3& value){
			if(!CheckBytesAvailable(sizeof(value.x) * 3)) return *this;
			*this >> value.x >> value.y >> value.z;
			return *this;
		}

		template <> BufferedFile<tBufferSize>& operator>>(Quat& value){
			if(!CheckBytesAvailable(sizeof(value.x) * 4)) return *this;
			*this >> value.w >> value.x >> value.y >> value.z;
			return *this;
		}

		template <> BufferedFile<tBufferSize>& operator>>(Box3& value){
			if(!CheckBytesAvailable(sizeof(value.pmin.x) * 6)) return *this;
			*this >> value.pmin >> value.pmax;
			return *this;
		}

		template <> BufferedFile<tBufferSize>& operator>>(HiBitString& value){
			byte tmpByteLen = 0x00;
			*this >> tmpByteLen;
			if(tmpByteLen & 0x80){
				value.mLength = tmpByteLen & 0x7F;
				*this >> tmpByteLen;
				value.mLength |= tmpByteLen << 7;
			}else{
				value.mLength = tmpByteLen;
			}

			*value.mString = 0;
			if(!CheckBytesAvailable(sizeof(value.mLength))) return *this;
			if(value.mLength == 0) return *this;
			*value.mString = new char[value.mLength + 1];
			memcpy_s((*value.mString), value.mLength, mBuffer + mCurrentBufferPosition, value.mLength);
			(*value.mString)[value.mLength] = 0;
			mCurrentBufferPosition += value.mLength;
			return *this;
		}

		template <> BufferedFile<tBufferSize>& operator>>(VarLengthString<byte>& value){
			*this >> value.mLength;
			*value.mString = 0;
			if(!CheckBytesAvailable(sizeof(value.mLength))) return *this;
			if(value.mLength == 0) return *this;
			*value.mString = new char[value.mLength + 1];
			memcpy_s((*value.mString), value.mLength, mBuffer + mCurrentBufferPosition, value.mLength);
			(*value.mString)[value.mLength] = 0;
			mCurrentBufferPosition += value.mLength;
			return *this;
		}

		template <> BufferedFile<tBufferSize>& operator>>(VarLengthString<word>& value){
			*this >> value.mLength;
			*value.mString = 0;
			if(!CheckBytesAvailable(sizeof(value.mLength))) return *this;
			if(value.mLength == 0) return *this;
			*value.mString = new char[value.mLength + 1];
			memcpy_s((*value.mString), value.mLength, mBuffer + mCurrentBufferPosition, value.mLength);
			(*value.mString)[value.mLength] = 0;
			mCurrentBufferPosition += value.mLength;
			return *this;
		}

		template <> BufferedFile<tBufferSize>& operator>>(VarLengthString<dword>& value){
			*this >> value.mLength;
			*value.mString = 0;
			if(!CheckBytesAvailable(sizeof(value.mLength))) return *this;
			if(value.mLength == 0) return *this;
			*value.mString = new char[value.mLength + 1];
			memcpy_s((*value.mString), value.mLength, mBuffer + mCurrentBufferPosition, value.mLength);
			(*value.mString)[value.mLength] = 0;
			mCurrentBufferPosition += value.mLength;
			return *this;
		}

		template <class tType> BufferedFile<tBufferSize>& operator<<(tType& value){
			fwrite(&value, sizeof(tType), 1, mFh);
			return *this;
		}

		template <> BufferedFile<tBufferSize>& operator<<(FixLengthString& value){
			int curLen = (int)strlen(value.mString);
			if(curLen >= value.mLength){
				fwrite(value.mString, 1, value.mLength, mFh);
			}else{
				fwrite(value.mString, 1, curLen, mFh);
				byte tempVal = 0;
				for(int i = curLen; i < value.mLength; ++i)
					fwrite(&tempVal, 1, 1, mFh);
			}
			return *this;
		}

		template <> BufferedFile<tBufferSize>& operator<<(TerminatedString& value){
			char* pch = *value.mString;
			while(*pch != value.mTerminator){
				fwrite(pch, 1, 1, mFh);
				++pch;
			}
			fwrite(&value.mTerminator, 1, 1, mFh);
			return *this;
		}

		template <> BufferedFile<tBufferSize>& operator<<(Point2& value){
			*this << value.x << value.y;
			return *this;
		}

		template <> BufferedFile<tBufferSize>& operator<<(Point3& value){
			*this << value.x << value.y << value.z;
			return *this;
		}

		template <> BufferedFile<tBufferSize>& operator<<(Quat& value){
			*this << value.w << value.x << value.y << value.z;
			return *this;
		}

		template <> BufferedFile<tBufferSize>& operator<<(Box3& value){
			*this << value.pmin << value.pmax;
			return *this;
		}

		template <> BufferedFile<tBufferSize>& operator<<(HiBitString& value){
			value.mLength = strlen(value.mString);
			if(value.mLength > 0x7F){
				byte btLen = (value.mLength & 0x7F) | 0x80;
				fh << btLen;
				btLen = (value.mLength >> 7);
				fh << btLen;
			}else{
				byte btLen = value.mLength;
				fh << btLen;
			}

			fwrite(*value.mString, 1, value.mLength, mFh);

			return *this;
		}

		template <> BufferedFile<tBufferSize>& operator<<(VarLengthString<byte>& value){
			*this << value.mLength;
			fwrite(*value.mString, 1, value.mLength, mFh);
			return *this;
		}

		template <> BufferedFile<tBufferSize>& operator<<(VarLengthString<word>& value){
			*this << value.mLength;
			fwrite(*value.mString, 1, value.mLength, mFh);
			return *this;
		}

		template <> BufferedFile<tBufferSize>& operator<<(VarLengthString<dword>& value){
			*this << value.mLength;
			fwrite(*value.mString, 1, value.mLength, mFh);
			return *this;
		}

		static const enum Errors {
			FILE_NOT_OPEN,
			FILE_READ_PAST_END,
			FILE_BUFFER_TOO_SMALL,
		};

		void Skip(int bytes){
			mCurrentReadPosition = (mCurrentReadPosition - mCurrentBufferSize) + mCurrentBufferPosition + bytes;
			CheckBytesAvailable(-1);
		}

		void Seek(int position){
			mCurrentReadPosition = position;
			CheckBytesAvailable(-1);
		}

		int Position(){
			if(mRead)
				return (mCurrentReadPosition - mCurrentBufferSize) + mCurrentBufferPosition;
			else
				return ftell(fh);
		}

	private:
		bool CheckBytesAvailable(int amount){
			if(amount > mCurrentBufferSize)
				throw FILE_BUFFER_TOO_SMALL;

			if(amount == -1 || mCurrentBufferPosition + amount > mCurrentBufferSize){
				if(amount != -1)
					mCurrentReadPosition = (mCurrentReadPosition - tBufferSize) + mCurrentBufferPosition;
				fseek(mFh, mCurrentReadPosition, SEEK_SET);
				mCurrentBufferSize = (int)fread(mBuffer, 1, tBufferSize, mFh);
				mCurrentReadPosition += mCurrentBufferSize;
				mCurrentBufferPosition = 0;
				if(amount > mCurrentBufferSize)
					throw FILE_READ_PAST_END;
			}

			return true;
		}

	private:
		FILE* mFh;
		char mBuffer[tBufferSize];

		int mCurrentReadPosition;
		int mCurrentBufferSize;
		int mCurrentBufferPosition;

		bool mRead;
	};

#undef FileError
};
