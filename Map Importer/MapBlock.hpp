class MapBlock
{
public:
	struct BasicEntry
	{
		char name[ MAX_PATH ];
		short unk2;
		short unk3;
		int unk4;
		int meshid;
		int unk6;
		int unk7;
		Point3 p;
		Quat r;
		Point3 s;
	};

	struct DecoEntry : BasicEntry
	{
	};

	struct CnstEntry : BasicEntry
	{
	};

	struct WaterEntry
	{
		Point3 p1;
		Point3 p2;
	};

	int decocount;
	DecoEntry* decos;
	int cnstcount;
	CnstEntry* cnsts;
	int watercount;
	WaterEntry* waters;

	void ReadBasic( haFile& fh, BasicEntry& obj )
	{
		fh.ReadBString( obj.name );
		obj.unk2 = fh.ReadInt16( );
		obj.unk3 = fh.ReadInt16( );
		obj.unk4 = fh.ReadInt32( );
		obj.meshid = fh.ReadInt32( );
		obj.unk6 = fh.ReadInt32( );
		obj.unk7 = fh.ReadInt32( );

		obj.r.x = -fh.ReadReal32( );
		obj.r.y = -fh.ReadReal32( );
		obj.r.z = -fh.ReadReal32( );
		obj.r.w = fh.ReadReal32( );

		obj.p.x = fh.ReadReal32( );
		obj.p.y = fh.ReadReal32( );
		obj.p.z = fh.ReadReal32( );

		obj.s.x = fh.ReadReal32( );
		obj.s.y = fh.ReadReal32( );
		obj.s.z = fh.ReadReal32( );
	};

	void ReadDeco( haFile& fh )
	{
		decocount = fh.ReadInt32( );
		DebugPrint( "Loading %d doodad Objects", decocount );

		decos = new DecoEntry[ decocount ];
		for( int i = 0; i < decocount; i++ )
		{
			ReadBasic( fh, decos[ i ] );
			DebugPrint( "[Ddad:%d] Done", i );
 		}
	};

	void ReadCnst( haFile& fh )
	{
		cnstcount = fh.ReadInt32( );
		DebugPrint( "Loading %d building Objects", cnstcount );

		cnsts = new CnstEntry[ cnstcount ];
		for( int i = 0; i < cnstcount; i++ )
		{
			ReadBasic( fh, cnsts[ i ] );
			DebugPrint( "[Bldg:%d] Done", i );
 		}
	};

	void ReadWater( haFile& fh )
	{
		fh.ReadReal32( );
		watercount = fh.ReadInt32( );
		waters = new WaterEntry[ watercount ];
		for( int i = 0; i < watercount; i++ )
		{
			waters[i].p1.x = fh.ReadReal32( );
			waters[i].p1.z = fh.ReadReal32( );
			waters[i].p1.y = fh.ReadReal32( );
			waters[i].p2.x = fh.ReadReal32( );
			waters[i].p2.z = fh.ReadReal32( );
			waters[i].p2.y = fh.ReadReal32( );
			waters[i].p1 /= 100.0f;
			waters[i].p2 /= 100.0f;
		}
	};

	void Load( char* ifopath )
	{
		DebugPrint( "Loading ifo '%s'", ifopath );

		haFile fh;
		if( !fh.Open( ifopath, "rb" ) )
		{
			DebugPrint( "Could not open '%s'", ifopath );
			return;
		}

		int blockcount = fh.ReadInt32( );
		for( int i = 0; i < blockcount; i++ )
		{
			int blockid = fh.ReadInt32( );
			int blockoffset = fh.ReadInt32( );
			int origoffset = fh.Tell( );
			fh.Seek( blockoffset, SEEK_SET );

			if( blockid == 1 ) ReadDeco( fh );
			if( blockid == 3 ) ReadCnst( fh );
			if( blockid == 9 ) ReadWater( fh );

			fh.Seek( origoffset, SEEK_SET );
		}

		DebugPrint( "Done loading ifo '%s'", ifopath );
	};
};