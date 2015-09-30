/******************************************\
                  haFile.h
    A class for simplifying file access

          Part of the Halia Engine
       Designed and Coded by Brett LV
\******************************************/

#pragma once

#include <iostream>
#include "haDatatypes.hpp"

#define HAFILE_BUFFERSIZE 4096

class haFile
{
protected:
	FILE* m_File;
	char m_Buffer[ HAFILE_BUFFERSIZE ];
	haInt32 m_BufferOffset;
	haInt32 m_BufferCurrent;
	haBool32 m_BufferEnabled;

public:
	haFile( )
	{
		m_File = NULL;
		m_BufferOffset = 0;
		m_BufferCurrent = 0;
		m_BufferEnabled = false;
	};

	~haFile( )
	{
		Close( );
	};

	haBool32 Open( const char* filename, const char* mode )
	{
		if( m_File != NULL )
		{
			Close( );
		}
		m_File = fopen( filename, mode );
		if( !m_File )
		{
			return false;
		}

		//EnableBuffer( haTrue );

		return true;
	};

	void Close( )
	{
		if( m_File != NULL )
		{
			fclose( m_File );
			m_File = NULL;
			m_BufferOffset = 0;
			m_BufferCurrent = 0;
			m_BufferEnabled = false;
		}
	};

	haBool32 Seek( haInt32 value, haInt32 dir = SEEK_CUR )
	{
		if( !m_BufferEnabled )
		{
			fseek( m_File, value, dir );
		}
		else
		{
			if( dir == SEEK_SET )
			{
				m_BufferCurrent = value;
			}
			else if( dir == SEEK_CUR )
			{
				m_BufferCurrent += value;
			}
			CheckBuffer( 0 );
		}
		return true;
	};

	haInt32 Tell( )
	{
		if( !m_BufferEnabled )
		{
			return ftell( m_File );
		}
		else
		{
			return m_BufferCurrent;
		}
	};

	haBool32 Eof( )
	{
		return (feof( m_File )==0) ? false : true;
	};

	haBool32 Read( void* buffer, haInt32 amount )
	{
		if( !m_BufferEnabled )
		{
			fread( buffer, 1, amount, m_File );
		}
		else
		{
			if( amount < HAFILE_BUFFERSIZE )
			{
				CheckBuffer( amount );
				memcpy( buffer, &m_Buffer[m_BufferCurrent-m_BufferOffset], amount );
				m_BufferCurrent += amount;
			}
			else
			{
				fseek( m_File, m_BufferCurrent, SEEK_SET );
				fread( buffer, 1, amount, m_File );
				m_BufferCurrent += amount;
				m_BufferOffset = m_BufferCurrent;
			}
		}
		return true;
	};

	haInt8 ReadInt8( )
	{
		haInt8 tmp;
		if( !m_BufferEnabled )
		{
			fread( &tmp, 1, 1, m_File );
		}
		else
		{
			CheckBuffer( 1 );
			tmp = *((haInt8*)&m_Buffer[m_BufferCurrent-m_BufferOffset]);
			m_BufferCurrent += 1;
		}
		return tmp;
	};

	haInt16 ReadInt16( )
	{
		haInt16 tmp;
		if( !m_BufferEnabled )
		{
			fread( &tmp, 2, 1, m_File );
		}
		else
		{
			CheckBuffer( 2 );
			tmp = *((haInt16*)&m_Buffer[m_BufferCurrent-m_BufferOffset]);
			m_BufferCurrent += 2;
		}
		return tmp;
	};

	haInt32 ReadInt32( )
	{
		haInt32 tmp;
		if( !m_BufferEnabled )
		{
			fread( &tmp, 4, 1, m_File );
		}
		else
		{
			CheckBuffer( 4 );
			tmp = *((haInt32*)&m_Buffer[m_BufferCurrent-m_BufferOffset]);
			m_BufferCurrent += 4;
		}
		return tmp;
	};

	haReal32 ReadReal32( )
	{
		haReal32 tmp;
		if( !m_BufferEnabled )
		{
			fread( &tmp, 4, 1, m_File );
		}
		else
		{
			CheckBuffer( 4 );
			tmp = *((haReal32*)&m_Buffer[m_BufferCurrent-m_BufferOffset]);
			m_BufferCurrent += 4;
		}
		return tmp;
	};

	haBool32 ReadSString( haString text )
	{
		haInt16 tmp;
		if( !m_BufferEnabled )
		{
			fread( &tmp, 2, 1, m_File );
			fread( text, 1, tmp, m_File );
		}
		else
		{
			tmp = ReadInt16( );
			Read( text, tmp );
			text[tmp] = 0;
		}
		text[tmp] = 0;
		return true;
	};

	haBool32 ReadBString( haString text )
	{
		haInt8 tmp1 = 0, tmp2 = 0;
		haInt16 tmp;
		if( !m_BufferEnabled )
		{
			fread( &tmp1, 1, 1, m_File );
			if( tmp1 & 0x80 )
				fread( &tmp2, 1, 1, m_File );
			tmp = (tmp2<<7)|(tmp1&0x7F);
			fread( text, 1, tmp, m_File );
		}
		else
		{
			tmp1 = ReadInt8( );
			if( tmp1 & 0x80 )
				tmp2 = ReadInt8( );
			tmp = (tmp2<<7)|(tmp1&0x7F);
			Read( text, tmp );
			text[tmp] = 0;
		}
		text[tmp] = 0;
		return true;
	};

	haBool32 ReadZString( haString text )
	{
		char* tmp = text;
		*tmp = 0;
		while( 1 )
		{
			if( !m_BufferEnabled )
			{
				fread( tmp, 1, 1, m_File );
			}
			else
			{
				*tmp = ReadInt8( );
			}
			if( *tmp == 0 )
				break;
			tmp++;
		}
		return true;
	};

	haBool32 ReadNString( haString text, haInt32 amount )
	{
		if( !m_BufferEnabled )
		{
			fread( text, 1, amount, m_File );
		}
		else
		{
			Read( text, amount );
		}

		text[ amount ] = 0;
		return true;
	};

	haBool32 SkipSString( )
	{
		if( !m_BufferEnabled )
		{
			haInt16 tmp;
			fread( &tmp, 2, 1, m_File );
			fseek( m_File, tmp, SEEK_CUR );
		}
		else
		{
			haInt16 tmp = ReadInt16( );
			Seek( tmp, SEEK_CUR );
		}
		return true;
	};

	haBool32 SkipZString( )
	{
		while( ReadInt8( ) != 0 );
		return true;
	};

	void CheckBuffer( haInt32 iAmount )
	{
		if( m_BufferEnabled )
		{
			if( ( iAmount == -1 ) || ( m_BufferCurrent < m_BufferOffset ) ||
				( m_BufferOffset + HAFILE_BUFFERSIZE < m_BufferCurrent + iAmount ) )
			{
				fseek( m_File, m_BufferCurrent, SEEK_SET );
				fread( m_Buffer, 1, HAFILE_BUFFERSIZE, m_File );
				m_BufferOffset = m_BufferCurrent;
			}
		}
	};

	void EnableBuffer( haBool32 value )
	{
		if( value == m_BufferEnabled )
			return;

		if( value )
		{
			m_BufferEnabled = true;
			m_BufferOffset = 0;
			m_BufferCurrent = ftell( m_File );
			CheckBuffer( -1 );
		}
		else
		{
			m_BufferEnabled = false;
		}
	};

	static haBool32 Exists( haString path )
	{
		FILE* fh = fopen( path, "rb" );
		if( fh == 0 )
			return false;
		fclose( fh );
		return true;
	};
};