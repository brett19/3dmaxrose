#ifndef ROSE_STL_CLASS
#define ROSE_STL_CLASS

#include <ExLIB/SafeDelete.hpp>
#include <ExLIB/String.hpp>
#include <ExLIB/StringMap.hpp>
#include <ExLIB/List.hpp>
#include <ExLIB/File.hpp>

namespace ROSE {
	class STL {
		class Entry {
			class Data {
			protected:
				Ex::String mText;
				Ex::String mComment;
				Ex::String mQuest1;
				Ex::String mQuest2;

				friend class STL;
			};

			Entry() : mData(0) {}
			~Entry(){
				SAFE_DELETE_ARRAY(mData);
			}

		protected:
			void AllocLanguages(unsigned int languages){
				mData = new Data[languages];
			}

			Data* mData;
			unsigned int mID;

			friend class STL;
		};

		enum {
			STL_TEXT = 1,
			STL_COMMENT = 1 << 1,
			STL_QUEST = 1 << 2,
		} STL_TYPES;

	public:
		STL() : mEntryList(0), mEntries(0), mLanguages(0), mType(0) {}
		~STL(){
			SAFE_DELETE_ARRAY(mEntryList);
		}

		bool Open(const char* path){
			Ex::File fh(path, "rb");
			if(!fh.IsOpen()) return false;

			Ex::String type = fh.ReadPascalString();
			if(type == Ex::String("NRST01")) mType = STL_TEXT;
			else if(type == Ex::String("ITST01")) mType = STL_TEXT | STL_COMMENT;
			else if(type == Ex::String("QEST01")) mType = STL_TEXT | STL_COMMENT | STL_QUEST;
			else{
				fh.Close();
				return false;
			}

			mEntries = fh.Read<unsigned int>();
			mEntryList = new Entry[mEntries];
			for(unsigned int i = 0; i < mEntries; ++i){
				Ex::String strId = fh.ReadPascalString();
				unsigned int id = fh.Read<unsigned int>();
				mStrIdMap[strId] = id;
				mEntryList[i].mID = id;
				//assert(i != id);
			}

			mLanguages = fh.Read<unsigned int>();
			unsigned int* lanOffsets = new unsigned int[mLanguages];
			for(unsigned int i = 0; i < mLanguages; ++i){
				lanOffsets[i] = fh.Read<unsigned int>();
			}

			unsigned int** strOffsets = new unsigned int*[mLanguages];
			for(unsigned int i = 0; i < mLanguages; ++i){
				fh.Seek(lanOffsets[i]);
				strOffsets[i] = new unsigned int[mEntries];
				for(unsigned int j = 0; j < mEntries; ++j){
					strOffsets[i][j] = fh.Read<unsigned int>();
				}
			}

			for(unsigned int i = 0; i < mEntries; ++i){
				mEntryList[i].AllocLanguages(mLanguages);
				for(unsigned int j = 0; j < mLanguages; ++j){
					fh.Seek(strOffsets[j][i]);
					if(mType & STL_TEXT){
						mEntryList[i].mData[j].mText = fh.ReadPascalString();
					}

					if(mType & STL_COMMENT){
						mEntryList[i].mData[j].mComment = fh.ReadPascalString();
					}

					if(mType & STL_QUEST){
						mEntryList[i].mData[j].mQuest1 = fh.ReadPascalString();
						mEntryList[i].mData[j].mQuest2 = fh.ReadPascalString();
					}
				}
			}

			for(unsigned int i = 0; i < mLanguages; ++i)
				delete [] strOffsets[i];

			delete [] strOffsets;
			delete [] lanOffsets;

			fh.Close();
			return true;
		}

		unsigned int LanguageCount() const {
			return mLanguages;
		}

		unsigned int EntryCount() const {
			return mEntries;
		}

		bool HasText() const {
			return (mType & STL_TEXT) == STL_TEXT;
		}

		bool HasComment() const {
			return (mType & STL_COMMENT) == STL_COMMENT;
		}

		bool HasQuest() const {
			return (mType & STL_QUEST) == STL_QUEST;
		}

		unsigned int FindByStrId(const Ex::String& strid) const {
			Ex::StringMap<unsigned int>::Iterator itr = mStrIdMap.find(strid);
			if(itr == mStrIdMap.end()) return -1;
			return FindById(*itr);
		}

		unsigned int FindById(unsigned int id) const {
			for(unsigned int i = 0; i < mEntries; ++i){
				if(mEntryList[i].mID != id) continue;
				return i;
			}

			return 0;
		}

		unsigned int ID(unsigned int entry) const {
			if(entry >= mEntries) return 0;
			return mEntryList[entry].mID;
		}

		const Ex::String& Text(unsigned int entry, unsigned int language = 1) const {
			static Ex::String fail;
			if(entry >= mEntries) return fail;
			if(language >= mLanguages) return fail;
			return mEntryList[entry].mData[language].mText;
		}

		const Ex::String& Comment(unsigned int entry, unsigned int language = 1) const {
			static Ex::String fail;
			if(entry >= mEntries) return fail;
			if(language >= mLanguages) return fail;
			return mEntryList[entry].mData[language].mComment;
		}

		const Ex::String& Quest1(unsigned int entry, unsigned int language = 1) const {
			static Ex::String fail;
			if(entry >= mEntries) return fail;
			if(language >= mLanguages) return fail;
			return mEntryList[entry].mData[language].mQuest1;
		}

		const Ex::String& Quest2(unsigned int entry, unsigned int language = 1) const {
			static Ex::String fail;
			if(entry >= mEntries) return fail;
			if(language >= mLanguages) return fail;
			return mEntryList[entry].mData[language].mQuest2;
		}

	private:
		unsigned int mType;
		unsigned int mLanguages;
		unsigned int mEntries;
		Entry* mEntryList;
		Ex::StringMap<unsigned int> mStrIdMap;
	};
};

#endif
