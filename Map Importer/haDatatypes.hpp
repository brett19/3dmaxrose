/******************************************\
  haDatatypes.hpp

  Part of the Halia Engine
  Designed and Coded by Brett LV
\******************************************/

#pragma once

typedef unsigned char haUInt8;
typedef signed char haInt8;
typedef unsigned short haUInt16;
typedef short haInt16;
typedef unsigned long haUInt32;
typedef long haInt32;
typedef float haReal32;
typedef double haReal64;
typedef bool haBool32;

typedef char haChar;
typedef haChar* haString;

#ifndef HA_NODIRECTX
#include <d3dx9.h>
typedef D3DXCOLOR haColor;
typedef POINT haScalar2;
typedef D3DXVECTOR2 haVector2;
typedef D3DXVECTOR3 haVector3;
typedef D3DXQUATERNION haQuaternion;
typedef D3DXMATRIX haMatrix;
typedef D3DXPLANE haPlane;
#endif

#ifndef HA_NOWINDOWS
#include <windows.h>
typedef HANDLE haHandle;
typedef HWND haHwnd;
typedef HINSTANCE haInstance;
#endif