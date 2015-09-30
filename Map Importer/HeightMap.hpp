class HeightMap
{
public:
	float heights[ 65 ][ 65 ];

	void Load( char* path )
	{
		haFile mfh;
		mfh.Open( path, "rb" );
		int vertsizex = mfh.ReadInt32( );
		int vertsizey = mfh.ReadInt32( );
		int wtfunk = mfh.ReadInt32( );
		int range = mfh.ReadInt32( );
		for( int iy = 0; iy < 65; iy++ )
		{
			for( int ix = 0; ix < 65; ix++ )
			{
				heights[ ix ][ iy ] = mfh.ReadReal32( );
			}
		}
		mfh.Close( );
	}
};