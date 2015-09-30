#include "Max.h"
#include "istdplug.h"
#include "stdmat.h"
#include "decomp.h"
#include "shape.h"
#include "splshape.h"
#include "dummy.h"
#include "bmmlib.h"
#include "bitmap.h"
#include "haFile.hpp"
#include "ObjectList.hpp"
#include "MapBlock.hpp"
#include "HeightMap.hpp"
#include "TileMap.hpp"
#include "MapData.hpp"
#include "resource.h"
#include "STB.hpp"
#include "STL.hpp"

ROSE::STB gZoneSTB;
ROSE::STL gZoneSTL;
Ex::String gBaseVFS;

HINSTANCE hInstance;
unsigned int gExportMap = 0;

INT_PTR CALLBACK MapImportOptionsDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch(message) {
		case WM_INITDIALOG:
			CenterWindow(hWnd, GetParent(hWnd));
			if(gZoneSTB.RowCount() == 0){
				gZoneSTB.Open(gBaseVFS + "3DDATA\\STB\\LIST_ZONE.STB");
				gZoneSTL.Open(gBaseVFS + "3DDATA\\STB\\LIST_ZONE_S.STL");
			}

			char buffer[255];
			for(unsigned int i = 0; i < gZoneSTB.RowCount(); ++i){
				const Ex::String& zoneName = gZoneSTL.Text(gZoneSTL.FindByStrId(gZoneSTB.StrValue(i, 26)));
				sprintf_s(buffer, 255, "[%u] %s", i, zoneName);				
				SendDlgItemMessage(hWnd, IDC_LIST_MAPS, LB_ADDSTRING, 0, (LPARAM)buffer);
			}

			return TRUE;
		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case IDC_IMP_CANCEL:
					EndDialog(hWnd, 0);
					return TRUE;
				case IDC_IMP_OK:
					gExportMap = SendDlgItemMessage(hWnd, IDC_LIST_MAPS, LB_GETCURSEL, 0, 0);
					EndDialog(hWnd, 1);
					return TRUE;
			}
	}
	return FALSE;
}

enum VertexFormats {
	//RMVF_INVALID = 1,
	RMVF_POSITION = 2,
	RMVF_NORMALS = 4,
	RMVF_COLORS = 8,
	RMVF_BONEINDICES = 16,
	RMVF_BONEWEIGHTS = 32,
	RMVF_TANGENTS = 64,
	RMVF_UVMAP1 = 128,
	RMVF_UVMAP2 = 256,
	RMVF_UVMAP3 = 512,
	RMVF_UVMAP4 = 1024,
};

class MapImport
	: public SceneImport
{
public:
	MapImport( )
	{
	};

	~MapImport( )
	{
	};

	int ExtCount( )
	{
		return 1;
	};

	const TCHAR * Ext( int n )
	{
		return ( n == 0 ) ? _T("STB") : _T("");
	};

	const TCHAR * LongDesc( )
	{
		return _T("ROSE Map Loader");
	};

	const TCHAR * ShortDesc( )
	{
		return _T("LIST_ZONE.STB");
	};

	const TCHAR * AuthorName( )
	{
		return _T("Brett19 (AruaRose)");
	};

	const TCHAR * CopyrightMessage( )
	{
		return _T("Copyright Brett19 2008-Present");
	};

	const TCHAR * OtherMessage1( )
	{
		return _T("");
	};

	const TCHAR * OtherMessage2( )
	{
		return _T("");
	};

	unsigned int Version( )
	{
		return 100;
	};

	void ShowAbout( HWND hWnd )
	{
	};

	int DoImport(const TCHAR* name, ImpInterface* i,Interface* gi, BOOL suppressPrompts=FALSE){
		Ex::String zoneStbPath = name;
		zoneStbPath.ToUpper();
		unsigned int pos = zoneStbPath.Find("3DDATA\\STB\\LIST_ZONE.STB");
		if(!pos){
			MessageBox(gi->GetMAXHWnd(), _T("Did not detect 3DDATA in path... Poopy :D"), _T("U WUT U WUT"), MB_ICONERROR);
			return IMPEXP_FAIL;
		}

		gBaseVFS = zoneStbPath.SubStr(0, pos);

		if(!DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAP_IMP_OPTIONS), GetActiveWindow(), MapImportOptionsDlgProc, (LPARAM)this) || gExportMap == 0)
			return IMPEXP_CANCEL;

		Matrix3 mtrx;

		DebugPrint( "--- STARTED IMPORT ---" );

		DebugPrint( "Disabling Viewport..." );
		//gi->DisableSceneRedraw( );

		DebugPrint( "Importing..." );

		const char* planets[] = { "JUNON", "LUNAR", "ELDEON", "ORO", 0 };
		Ex::String zonpath = gZoneSTB.StrValue(gExportMap, 1);
		
		//3DDATA\Maps\Junon\JDT01\JDT01.zon
		int start = strlen("3DDATA\\Maps\\");
		int end = zonpath.Find("\\", start);
		Ex::String planet = zonpath.SubStr(start, end - start);
		start = end + 1;
		end = zonpath.Find("\\", start);
		Ex::String map = zonpath.SubStr(start, end - start);
		Ex::String decopath = gZoneSTB.StrValue(gExportMap, 11);
		Ex::String cnstpath = gZoneSTB.StrValue(gExportMap, 12);

		DebugPrint( "Got Def" );

		char mappath[ 256 ],  mapnodename[ 256 ];
		sprintf( mappath, "%s3DDATA\\MAPS\\%s\\%s", gBaseVFS, planet, map );
		DebugPrint( "Generated Map Path : %s", mappath );

		decopath = gBaseVFS + decopath;
		cnstpath = gBaseVFS + cnstpath;
		zonpath = gBaseVFS + zonpath;

		MapData md;
		md.Load( zonpath );

		MultiMtl* mmtl = NewDefaultMultiMtl( );
		mmtl->SetNumSubMtls( md.combcount );
		for( int i = 0; i < md.combcount; i++ )
		{
			char texpath[ 256 ];
			sprintf( texpath, "%s%s", gBaseVFS, md.tiles[md.combs[i].tile1].path );
			BitmapTex* btex1 = NewDefaultBitmapTex( );
			btex1->SetMapName( texpath );
			sprintf( texpath, "%s%s", gBaseVFS, md.tiles[md.combs[i].tile2].path );
			BitmapTex* btex2 = NewDefaultBitmapTex( );
			btex2->SetMapName( texpath );
			BitmapTex* btex3 = NewDefaultBitmapTex( );
			btex3->SetMapName( texpath );

			btex1->SetAlphaSource( ALPHA_NONE );
			btex2->SetAlphaSource( ALPHA_NONE );
			btex3->SetAlphaAsMono( true );
			
			if( md.combs[i].rot == 2 ) {
				btex3->GetUVGen( )->SetUAng( 0.0f, 0.0f );
				btex3->GetUVGen( )->SetVAng( 3.14159265f, 0.0f );
			} else if( md.combs[i].rot == 3 ) {
				btex3->GetUVGen( )->SetUAng( 3.14159265f, 0.0f );
				btex3->GetUVGen( )->SetVAng( 0.0f, 0.0f );
			} else if( md.combs[i].rot == 4 ) {
				btex3->GetUVGen( )->SetUAng( 3.14159265f, 0.0f );
				btex3->GetUVGen( )->SetVAng( 3.14159265f, 0.0f );
			}

			MultiTex* tex = NewDefaultMixTex( );
			tex->SetNumSubTexmaps( 3 );
			tex->SetSubTexmap( 0, btex1 );
			tex->SetSubTexmap( 1, btex2 );
			tex->SetSubTexmap( 2, btex3 );

			StdMat2* mtl3 = NewDefaultStdMat( );
			mtl3->SetSubTexmap( ID_DI, tex );
			
			mmtl->SetSubMtl( i, mtl3 );
		}

		ObjectList decolist;
		ObjectList cnstlist;
		decolist.Load( decopath );
		cnstlist.Load( cnstpath );

		char nodename[ 256 ], ifopath[ 256 ], himpath[ 256 ], tilpath[ 256 ];
		for( int ix = 0; ix <= 64; ix++ )
		{
			for( int iy = 0; iy <= 64; iy++ )
			{
				sprintf( himpath, "%s\\%d_%d.HIM", mappath, ix, iy );
				DebugPrint( "Generated Him Path : %s", himpath );

				FILE* fh;
				fopen_s(&fh, himpath, "rb");
				if(!fh) continue;

				sprintf( nodename, "%d_%d", ix, iy );

				sprintf( ifopath, "%s\\%d_%d.IFO", mappath, ix, iy );
				DebugPrint( "Generated Ifo Path : %s", ifopath );
				sprintf( tilpath, "%s\\%d_%d.TIL", mappath, ix, iy );
				DebugPrint( "Generated Him Path : %s", himpath );

				LoadMap( ifopath, nodename, decolist, cnstlist, gi );

				TriObject* terrainobj = CreateNewTriObject( );
				INode* terrainnode = gi->CreateObjectNode( terrainobj );
				sprintf( mapnodename, "%s_PLANE", nodename );
				terrainnode->SetName( mapnodename );
				mtrx.IdentityMatrix( );
				Point3 p( 0.0f, 0.0f, 0.0f );
				mtrx.SetTranslate( p );
				terrainnode->SetNodeTM( 0.0f, mtrx );
				terrainnode->SetWireColor( 0xFFCCCCCC );
				LoadHeightmap( himpath, tilpath, md, ix, iy, terrainobj );
				terrainnode->SetMtl( mmtl );


				/*
				TriObject* obj = CreateNewTriObject( );
				INode* objnode = gi->CreateObjectNode( obj );
				sprintf( objnodename, "%s_%d_%d", nodename, 0, 0 );
				objnode->SetName( objnodename );
				mtrx.IdentityMatrix( );
				objnode->SetNodeTM( 0.0f, mtrx );
				LoadMesh( "C:\\Program Files\\Triggersoft\\Rose Online\\3ddata\\JUNON\\VILLAGE\\CASTLEGATE01\\CASTLEGATE01.ZMS", obj );
				*/
			}
		}

		DebugPrint( "Done Importing!" );

		DebugPrint( "Enabling Viewport..." );
		//gi->EnableSceneRedraw( );

		DebugPrint( "--- ENDED IMPORT ---" );

		return 1;
	};

	void LoadHeightmap( char* himpath, char* tilpath, MapData& md, short chunkx, short chunky, TriObject*& obj )
	{
		HeightMap hm;
		hm.Load( himpath );

		TileMap tm;
		tm.Load( tilpath );

		Mesh& msh = obj->mesh;

		msh.setMapSupport( 1 );
		msh.maps[1].setNumVerts( 16 * 16 * 5 * 5 );
		msh.setMapSupport( 2 );
		msh.maps[2].setNumVerts( 16 * 16 * 5 * 5 );

		msh.setNumVerts( 16 * 16 * 5 * 5 );
		int vertid = 0;
		for( int ix = 0; ix < 16; ix++ )
		{
			for( int iy = 0; iy < 16; iy++ )
			{
				for( int vy = 0; vy < 5; vy++ )
				{
					for( int vx = 0; vx < 5; vx++ )
					{
						int dx4 = ix * 4 + vx;
						int dy4 = iy * 4 + vy;
						int dx5 = ix * 5 + vx;
						int dy5 = iy * 5 + vy;

						Point3 vert;
						vert.x = ((dx4*2.5f) + (chunkx*160.0f));
						vert.y = 10400.0f - ((dy4*2.5f) + (chunky*160.0f));
						vert.z = hm.heights[dx4][dy4] / 100.0f;
						vert.x -= 5200.0f;
						vert.y -= 5200.0f;
						msh.setVert( vertid, vert );

						msh.maps[2].tv[vertid].x = (float)dx4 / 64.0f;
						msh.maps[2].tv[vertid].y = (float)dx4 / 64.0f;
						msh.maps[1].tv[vertid].x = (float)vx / 4.0f;
						msh.maps[1].tv[vertid].y = (float)vy / 4.0f;

						msh.maps[1].tv[vertid].y = 1 - msh.maps[1].tv[vertid].y;
						msh.maps[2].tv[vertid].y = 1 - msh.maps[2].tv[vertid].y;

						vertid++;
					}
				}
			}
		}

		short predefindices[32][3] = {
			{ 5, 0, 6 }, { 6, 0, 1 }, { 10, 5, 11 }, { 11, 5, 6 }, { 15, 10, 16 },
			{ 16, 10, 11 }, { 20, 15, 21 }, { 21, 15, 16 }, { 6, 1, 7 }, { 7, 1, 2 },
			{ 11, 6, 12 }, { 12, 6, 7 }, { 16, 11, 17 }, { 17, 11, 12 }, { 21, 16, 22 },
			{ 22, 16, 17 }, { 7, 2, 8 }, { 8, 2, 3 }, { 12, 7, 13 }, { 13, 7, 8 },
			{ 17, 12, 18 }, { 18, 12, 13 }, { 22, 17, 23 }, { 23, 17, 18 }, { 8, 3, 9 },
			{ 9, 3, 4 }, { 13, 8, 14 }, { 14, 8, 9 }, { 18, 13, 19 }, { 19, 13, 14 },
			{ 23, 18, 24 }, { 24, 18, 19 }
		};

		msh.setNumFaces( 16 * 16 * 4 * 4 * 2 );
		msh.maps[1].setNumFaces( 16 * 16 * 4 * 4 * 2 );
		msh.maps[2].setNumFaces( 16 * 16 * 4 * 4 * 2 );
		for( int ix = 0; ix < 16; ix++ )
		{
			for( int iy = 0; iy < 16; iy++ )
			{
				int i = ix*16+iy;
				for( int j = 0; j < 32; j++ )
				{
					msh.faces[i*32+j].v[0] = i*25 + predefindices[j][0];
					msh.faces[i*32+j].v[1] = i*25 + predefindices[j][1];
					msh.faces[i*32+j].v[2] = i*25 + predefindices[j][2];

					for( int k = 1; k < 3; k++ )
					{
						msh.maps[k].tf[i*32+j].t[0] = i*25 + predefindices[j][0];
						msh.maps[k].tf[i*32+j].t[1] = i*25 + predefindices[j][1];
						msh.maps[k].tf[i*32+j].t[2] = i*25 + predefindices[j][2];
					}

					msh.faces[i*32+j].setMatID( tm.tiles[ix][iy].tileid );
				}
			}
		}

		// 32 Faces * 3 Verts/Face
		for( int i = 0; i < 16 * 16 * 4 * 4 * 2; i++ )
			msh.FlipNormal( i );
		
		msh.InvalidateTopologyCache( );
		msh.InvalidateGeomCache( );
		msh.InvalidateEdgeList( );
		msh.buildBoundingBox( );
		msh.buildNormals( );
	};

	void LoadMap( char* ifopath, char* blockname, ObjectList& decolist, ObjectList& cnstlist, Interface*& gi )
	{
		Matrix3 tmpm;
		MapBlock ifo;
		ifo.Load( ifopath );

		for( int i = 0; i < ifo.decocount; i++ )
		{
			MapBlock::DecoEntry ifoe = ifo.decos[i];
			ObjectList::Object& obj = decolist.objects[ifoe.meshid];
			
			Matrix3 ifom;
			ifom.IdentityMatrix( );

			tmpm.IdentityMatrix( );
			tmpm.SetScale( ifoe.s );
			ifom *= tmpm;

			tmpm.IdentityMatrix( );
			tmpm.SetRotate( ifoe.r );
			ifom *= tmpm;

			tmpm.IdentityMatrix( );
			tmpm.SetTrans( ifoe.p / 100 );
			ifom *= tmpm;

			for( int j = 0; j < obj.partcount; j++ )
			{
				ObjectList::ObjectPart part = obj.parts[j];

				Matrix3 partm;
				partm.IdentityMatrix( );

				tmpm.IdentityMatrix( );
				tmpm.SetRotate( part.r );
				partm *= tmpm;

				tmpm.IdentityMatrix( );
				tmpm.SetScale( part.s );
				partm *= tmpm;

				tmpm.IdentityMatrix( );
				tmpm.SetTrans( part.p );
				partm *= tmpm;

				partm *= ifom;

				TriObject* obj = CreateNewTriObject( );
				INode* objnode = gi->CreateObjectNode( obj );
				char objnodename[ 256 ];
				sprintf( objnodename, "%s_DECO_%d_%d", blockname, i, j );
				objnode->SetName( objnodename );
				objnode->SetNodeTM( 0.0f, partm );
				objnode->SetWireColor( 0xFFCCCCCC );

				char texpath[ 256 ];
				sprintf( texpath, "%s%s", gBaseVFS, decolist.mats[part.texid].path );

				BitmapTex* btex = NewDefaultBitmapTex( );
				btex->SetMapName( texpath );

				StdMat2* mtl = NewDefaultStdMat( );
				mtl->SetSubTexmap( ID_DI, btex );
				if( decolist.mats[part.texid].alphaenabled )
				{
					BitmapTex* btex2 = NewDefaultBitmapTex( );
					btex2->SetMapName( texpath );
					btex2->SetAlphaAsMono( true );
					mtl->SetSubTexmap( ID_OP, btex2 );
				}
				if( decolist.mats[part.texid].twosided )
				{
					mtl->SetTwoSided( true );
				}
				objnode->SetMtl( mtl );

				char meshpath[ 256 ];
				sprintf( meshpath, "%s%s", gBaseVFS, decolist.meshs[part.meshid].path );
				LoadMesh( meshpath, obj );
			}
		}
		//*/
	};

	void LoadMesh( char* meshpath, TriObject*& obj )
	{
		DebugPrint( "Loading mesh '%s'", meshpath );

		Mesh& msh = obj->mesh;

		haFile fh;
		if( !fh.Open( meshpath, "rb" ) )
		{
			DebugPrint( "Failed to load mesh '%s'", meshpath );
			return;
		}

		fh.Seek( 8 ); // Format Name
		int vertformat = fh.ReadInt32( );
		fh.Seek( 24 ); // Bounding Box

		short bonelookupcount = fh.ReadInt16( );
		fh.Seek( bonelookupcount * 2 );

		short vertcount = fh.ReadInt16( );
		msh.setNumVerts( vertcount );

		if( vertformat & RMVF_POSITION )
		{
			for( int i = 0; i < vertcount; i++ )
			{
				msh.verts[i].x = fh.ReadReal32( );
				msh.verts[i].y = fh.ReadReal32( );
				msh.verts[i].z = fh.ReadReal32( );
			}
		}

		if( vertformat & RMVF_NORMALS )
		{
			for( int i = 0; i < vertcount; i++ )
			{
				fh.ReadReal32( );
				fh.ReadReal32( );
				fh.ReadReal32( );
			}
		}

		if( vertformat & RMVF_COLORS )
		{
			for( int i = 0; i < vertcount; i++ )
			{
				fh.ReadInt32( );
			}
		}


		for( int i = 0; i < vertcount; i++ )
		{
			if( vertformat & RMVF_BONEWEIGHTS )
			{
				fh.ReadReal32( );
				fh.ReadReal32( );
				fh.ReadReal32( );
				fh.ReadReal32( );
			};
			if( vertformat & RMVF_BONEINDICES )
			{
				fh.ReadInt16( );
				fh.ReadInt16( );
				fh.ReadInt16( );
				fh.ReadInt16( );
			}
		}

		if( vertformat & RMVF_TANGENTS )
		{
			for( int i = 0; i < vertcount; i++ )
			{
				fh.ReadReal32( );
				fh.ReadReal32( );
				fh.ReadReal32( );
			}
		}

		if( vertformat & RMVF_UVMAP1 )
		{
			msh.setMapSupport( 1 );
			msh.maps[1].setNumVerts( vertcount );
			for( int i = 0; i < vertcount; i++ )
			{
				msh.maps[1].tv[i].x = fh.ReadReal32( );
				msh.maps[1].tv[i].y = 1.0f - fh.ReadReal32( );
			}
		}
		if( vertformat & RMVF_UVMAP2 )
		{
			msh.setMapSupport( 2 );
			msh.maps[2].setNumVerts( vertcount );
			for( int i = 0; i < vertcount; i++ )
			{
				msh.maps[2].tv[i].x = fh.ReadReal32( );
				msh.maps[2].tv[i].y = 1.0f - fh.ReadReal32( );
			}
		}
		if( vertformat & RMVF_UVMAP3 )
		{
			for( int i = 0; i < vertcount; i++ )
			{
				fh.ReadReal32( );
				fh.ReadReal32( );
			}
		}
		if( vertformat & RMVF_UVMAP4 )
		{
			for( int i = 0; i < vertcount; i++ )
			{
				fh.ReadReal32( );
				fh.ReadReal32( );
			}
		}

		int facecount = fh.ReadInt16( );
		msh.setNumFaces( facecount );
		//msh.setNumTVFaces( facecount );
		for( int i = 0; i < facecount; i++ )
		{
			msh.faces[i].v[0] = fh.ReadInt16( );
			msh.faces[i].v[1] = fh.ReadInt16( );
			msh.faces[i].v[2] = fh.ReadInt16( );
			//msh.tvFace[i].setTVerts( msh.faces[i].v );
		}
		///*
		int mc = msh.getNumMaps( );
		for( int i = 1; i < mc; i++ )
		{
			msh.maps[i].setNumFaces( facecount );
			for( int j = 0; j < facecount; j++ )
			{
				msh.maps[i].tf[j].t[0] = msh.faces[j].v[0];
				msh.maps[i].tf[j].t[1] = msh.faces[j].v[1];
				msh.maps[i].tf[j].t[2] = msh.faces[j].v[2];
			}
		}
		//*/

		int stripcount = fh.ReadInt16( );
		fh.Seek( stripcount * 2 );

		fh.ReadInt16( );

		fh.Close( );

		msh.InvalidateTopologyCache( );
		msh.InvalidateGeomCache( );
		msh.InvalidateEdgeList( );
		msh.buildBoundingBox( );
		msh.buildNormals( );
	};
};

class MapClassDesc
	: public ClassDesc
{
public:
	int IsPublic( ) { return 1; };
	void* Create( BOOL loading = false ) { return new MapImport; }
	const TCHAR* ClassName() { return _T("ROSE Map Importer"); }
	SClass_ID SuperClassID() { return SCENE_IMPORT_CLASS_ID; }
	Class_ID ClassID() { return Class_ID(0x3da419c2,0x26461612); }
	const TCHAR* Category() { return _T("Scene Import"); }
};

static MapClassDesc MapDesc;
int controlsInit = FALSE;

bool WINAPI DllMain( HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved )
{
	switch( fdwReason )
	{
		case DLL_PROCESS_ATTACH:
		{
			hInstance = hinstDLL;

			if (!controlsInit) {
				controlsInit = TRUE;
				InitCustomControls(hInstance);	// Initialize MAX's custom controls
				InitCommonControls();			// Initialize Win95 controls
			}

			DisableThreadLibraryCalls( hInstance );
			break;
		}
	}
	return true;
};

__declspec( dllexport ) const TCHAR* LibDescription( ) { return _T("ROSE Map Importer"); }
__declspec( dllexport ) int LibNumberClasses( ) { return 1; }
__declspec( dllexport ) ClassDesc* LibClassDesc( int i ) { return ( i == 0 ) ? &MapDesc : 0; }
__declspec( dllexport ) ULONG LibVersion( ) { return VERSION_3DSMAX; }
__declspec( dllexport ) ULONG CanAutoDefer() { return 1; }
