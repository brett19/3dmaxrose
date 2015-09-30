class TileMap
{
public:
	struct Block
	{
		int tileid;
	};

	Block tiles[ 16 ][ 16];

	void Load( char* path )
	{
		haFile fh;
		fh.Open( path, "rb" );
		int tilesizex = fh.ReadInt32( );
		int tilesizey = fh.ReadInt32( );
		for( int iy = 0; iy < 16; iy++ )
		{
			for( int ix = 0; ix < 16; ix++ )
			{
				fh.ReadInt8( );
				fh.ReadInt8( );
				fh.ReadInt8( );
				tiles[ ix ][ iy ].tileid = fh.ReadInt32( );
			}
		}
		fh.Close( );
	}
};