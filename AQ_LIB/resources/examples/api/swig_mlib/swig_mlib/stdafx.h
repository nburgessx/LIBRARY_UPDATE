// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>



#include "MBRoot.h"
#include "InitializeMLibVanilla.h"

extern MBRoot* g_root;  // does not have ownership, ownership is inside the MLIB Singleton and unable to create weak_ptr because ownership ...



// TODO: reference additional headers your program requires here
