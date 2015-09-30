#ifndef ROSE_STB_CLASS
#define ROSE_STB_CLASS

#include <ExLIB/SafeDelete.hpp>
#include <ExLIB/String.hpp>
#include <ExLIB/File.hpp>

namespace ROSE {
	class STB {
	private:
		struct Header {
			char S;
			char T;
			char B;
			char V;

			bool IsValid(){
				return (S == 'S') && (T == 'T') && (B == 'B');
			}
		};

	public:
		STB() : mWidths(0), mColumnTitles(0), mRowNames(0), mData(0) {}
		~STB(){
			SAFE_DELETE_ARRAY(mWidths);
			SAFE_DELETE_ARRAY(mColumnTitles);
			SAFE_DELETE_ARRAY(mRowNames);
			SAFE_DELETE_ARRAY_2D(mData, mRows);
		}

		bool Open(const char* path){
			Ex::File fh(path, "rb");
			if(!fh.IsOpen()) return false;

			Header head = fh.Read<Header>();
			int version = head.V - '0';
			if(!head.IsValid()){ fh.Close(); return false; }

			unsigned int offset = fh.Read<unsigned int>();
			mRows = fh.Read<unsigned int>();
			mColumns = fh.Read<unsigned int>();

			mRowHeight = fh.Read<unsigned int>();
			if(version == 0){
				fh.Skip(4);
			}else if(version == 1){
				mWidths = new unsigned short[mColumns + 1];
				for(unsigned int i = 0; i < mColumns + 1; ++i)
					mWidths[i] = fh.Read<unsigned short>();
			}

			mColumnTitles = new Ex::String[mColumns];
			for(unsigned int i = 0; i < mColumns; ++i)
				mColumnTitles[i] = fh.ReadStringLength<unsigned short>();

			--mRows;
			--mColumns;

			mIDColumnName = fh.ReadStringLength<unsigned short>();
			mRowNames = new Ex::String[mRows];
			for(unsigned int i = 0; i < mRows; ++i){
				mRowNames[i] = fh.ReadStringLength<unsigned short>();
			}

			mData = new Ex::String*[mRows];
			for(unsigned int i = 0; i < mRows; ++i){
				mData[i] = new Ex::String[mColumns];
				for(unsigned int j = 0; j < mColumns; ++j){
					mData[i][j] = fh.ReadStringLength<unsigned short>();
				}
			}

			fh.Close();
			return true;
		}

		const Ex::String& StrValue(int row, int column){
			return mData[row][column];
		}

		int IntValue(int row, int column){
			if(!mData[row][column].IsValid()) return 0;
			return atoi(mData[row][column]);
		}

		unsigned int RowCount(){
			return mRows;
		}

		unsigned int ColumnCount(){
			return mColumns;
		}

	private:
		unsigned int mRows;
		unsigned int mColumns;

		unsigned int mRowHeight;
		unsigned short* mWidths;
		Ex::String* mColumnTitles;

		Ex::String mIDColumnName;
		Ex::String* mRowNames;

		Ex::String** mData;
	};
};

#endif
