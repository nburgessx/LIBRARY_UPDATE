// stdafx.cpp : source file that includes just the standard includes
// swigUseCase.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: this could become a dangling pointer if MLIB decides to change its root outside of this scope...
// make Mlib own a shared_ptr instead of scoped_ptr that way weak_ptr are still accessible

extern MBRoot* g_root = nullptr;
