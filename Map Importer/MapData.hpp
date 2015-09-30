class MapData
{
public:
	struct Tile
	{
		char path[ 256 ];
	};
	struct TileComb
	{
		int tile1;
		int tile2;
		int rot;
	};

	int tilecount;
	Tile* tiles;
	int combcount;
	TileComb* combs;

	void ReadTiles( haFile& fh )
	{
		tilecount = fh.ReadInt32( );
		tiles = new Tile[ tilecount ];
		for( int i = 0; i < tilecount; i++ )
		{
			fh.ReadBString( tiles[i].path );
			if( _strcmpi( tiles[i].path, "end" ) == 0 )
				break;
		}
	};

	void ReadTileCombs( haFile& fh )
	{
		combcount = fh.ReadInt32( );
		combs = new TileComb[ combcount ];
		for( int i = 0; i < combcount; i++ )
		{
			combs[i].tile1 = fh.ReadInt32( );
			combs[i].tile2 = fh.ReadInt32( );
			combs[i].tile1 += fh.ReadInt32( );
			combs[i].tile2 += fh.ReadInt32( );
			fh.ReadInt32( );
			combs[i].rot = fh.ReadInt32( );
			fh.ReadInt32( );
		}
	};

	void Load( const char* path )
	{
		DebugPrint( "Loading zon '%s'", path );

		haFile fh;
		if( !fh.Open( path, "rb" ) )
		{
			DebugPrint( "Could not open '%s'", path );
			return;
		}

		int blockcount = fh.ReadInt32( );
		for( int i = 0; i < blockcount; i++ )
		{
			int blockid = fh.ReadInt32( );
			int blockoffset = fh.ReadInt32( );
			int origoffset = fh.Tell( );
			fh.Seek( blockoffset, SEEK_SET );

			if( blockid == 2 ) ReadTiles( fh );
			if( blockid == 3 ) ReadTileCombs( fh );

			fh.Seek( origoffset, SEEK_SET );
		}

		DebugPrint( "Done loading zon '%s'", path );
	};
};