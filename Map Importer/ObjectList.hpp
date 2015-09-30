#pragma once
#include "haFile.hpp"

class ObjectList
{
public:
	struct Mesh
	{
		char path[ MAX_PATH ];
	};

	struct Material
	{
		char path[ MAX_PATH ];
		short rotation;
		short alphaenabled;
		short twosided;
		short alphatestenabled;
		short alpharefenabled;
		short zwriteenabled;
		short ztestenabled;
		short blendingmode;
		short specularenabled;
		float alpha;
		short glowtype;
		int glowcolor;
	};

	struct ObjectPart
	{
		short meshid;
		short texid;
		Point3 p;
		Quat r;
		Point3 s;
	};

	struct Object
	{
		short partcount;
		ObjectPart* parts;
	};

	short meshcount;
	Mesh* meshs;
	short matcount;
	Material* mats;
	short objcount;
	Object* objects;

	ObjectPart ReadPart( haFile& fh )
	{
		ObjectPart part;

		part.meshid = fh.ReadInt16( );
		part.texid = fh.ReadInt16( );
		
		part.p.x = 0; part.p.y = 0; part.p.z = 0;
		part.r.x = 0; part.r.y = 0; part.r.z = 0; part.r.w = 1; 
		part.s.x = 1; part.s.y = 1; part.s.z = 1;

		while( 1 )
		{
			short flagcmd = fh.ReadInt8( );
			if( flagcmd == 0 )
				break;
			short flagsize = fh.ReadInt8( );

			switch( flagcmd )
			{
			case 1:
				part.p.x = fh.ReadReal32( ) / 100.0f;
				part.p.y = fh.ReadReal32( ) / 100.0f;
				part.p.z = fh.ReadReal32( ) / 100.0f;
				DebugPrint( "    |- Got Position" );
				break;
			case 2:
				part.r.w = fh.ReadReal32( );
				part.r.x = fh.ReadReal32( );
				part.r.y = fh.ReadReal32( );
				part.r.z = fh.ReadReal32( );
				DebugPrint( "    |- Got Rotation" );
				break;
			case 3:
				part.s.x = fh.ReadReal32( );
				part.s.y = fh.ReadReal32( );
				part.s.z = fh.ReadReal32( );
				DebugPrint( "    |- Got Scale" );
				break;
			case 4:
				fh.ReadReal32( );
				fh.ReadReal32( );
				fh.ReadReal32( );
				break;
			case 5:
				fh.ReadInt16( );
				break;
			case 6:
				fh.ReadInt16( );
				break;
			case 7:
				fh.ReadInt16( );
				break;
			case 29:
				fh.ReadInt16( );
				break;
			case 30:
				fh.Seek( flagsize );
				break;
			case 31:
				fh.ReadInt16( );
				break;
			case 32:
				fh.ReadInt16( );
				break;
			default:
				break;
			}
		}

		return part;
	};

	void Load( const char* path )
	{
		DebugPrint( "Reading ObjectList '%s'", path );

		haFile fh;
		if( !fh.Open( path, "rb" ) )
		{
			DebugPrint( "Could not open '%s'", path );
			return;
		}
		
		meshcount = fh.ReadInt16( );
		meshs = new Mesh[ meshcount ];
		for( int i = 0; i < meshcount; i++ )
		{
			fh.ReadZString( meshs[ i ].path );
			DebugPrint( "[Msh:%d] %s", i, meshs[ i ].path );
		}

		int matcount = fh.ReadInt16( );
		mats = new Material[ matcount ];
		for( int i = 0; i < matcount; i++ )
		{
			fh.ReadZString( mats[i].path );
			fh.ReadInt16( );
			mats[i].alphaenabled = fh.ReadInt16( );
			mats[i].twosided = fh.ReadInt16( );
			fh.ReadInt16( );
			fh.ReadInt16( );
			fh.ReadInt16( );
			fh.ReadInt16( );
			fh.ReadInt16( );
			fh.ReadInt16( );
			fh.ReadReal32( );
			fh.ReadInt16( );
			fh.ReadReal32( );
			fh.ReadReal32( );
			fh.ReadReal32( );
			DebugPrint( "[Mtr:%d] Done", i );
		}

		int effectcount = fh.ReadInt16( );
		for( int i = 0; i < effectcount; i++ )
		{
			fh.SkipZString( );
			DebugPrint( "[Eft:%d] Done", i );
		}

		int objectcount = fh.ReadInt16( );
		objects = new Object[ objectcount ];
		for( int i = 0; i < objectcount; i++ )
		{
			fh.ReadInt32( );
			fh.ReadInt32( );
			fh.ReadInt32( );

			objects[i].partcount = fh.ReadInt16( );
			if( objects[i].partcount > 0 )
			{
				objects[i].parts = new ObjectPart[ objects[i].partcount ];
				for( int j = 0; j < objects[i].partcount; j++ )
				{
					objects[i].parts[j] = ReadPart( fh );
				}

				int partcount2 = fh.ReadInt16( );
				for( int j = 0; j < partcount2; j++ )
				{
					ReadPart( fh );
				}

				fh.ReadReal32( );
				fh.ReadReal32( );
				fh.ReadReal32( );
				fh.ReadReal32( );
				fh.ReadReal32( );
				fh.ReadReal32( );
			}

			DebugPrint( "[Obj:%d] Done", i );
		}
	}
};