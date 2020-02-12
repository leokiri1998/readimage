/*
//////////////////////////////////////////////////////////////////////////
COPYRIGHT NOTICE, DISCLAIMER, and LICENSE:
//////////////////////////////////////////////////////////////////////////

CUtil : Copyright (C) 2008, Kazi Shupantha Imam (shupantha@yahoo.com)

//////////////////////////////////////////////////////////////////////////
Covered code is provided under this license on an "as is" basis, without
warranty of any kind, either expressed or implied, including, without
limitation, warranties that the covered code is free of defects,
merchantable, fit for a particular purpose or non-infringing. The entire
risk as to the quality and performance of the covered code is with you.
Should any covered code prove defective in any respect, you (not the
initial developer or any other contributor) assume the cost of any
necessary servicing, repair or correction. This disclaimer of warranty
constitutes an essential part of this license. No use of any covered code
is authorized hereunder except under this disclaimer.

Permission is hereby granted to use, copy, modify, and distribute this
source code, or portions hereof, for any purpose, including commercial
applications, freely and without fee, subject to the following
restrictions: 

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.
//////////////////////////////////////////////////////////////////////////
*/

#pragma once

#ifdef WIN32
#include <windows.h>
#include "atlbase.h"
#endif

#include <sys\types.h> 
#include <sys\stat.h> 

#include <vector>
#include <string>

using std::vector;

using std::string;
using std::wstring;

#ifdef UNICODE
#define _tstring		wstring
#else
#define _tstring		string
#endif

#ifdef WIN32
#define DIRECTORY_SEPARATOR_W		L"\\"
#define DIRECTORY_SEPARATOR_W_C		L'\\'

#define DIRECTORY_SEPARATOR_A		"\\"
#define DIRECTORY_SEPARATOR_A_C		'\\'
#else
#define DIRECTORY_SEPARATOR_W		L"/"
#define DIRECTORY_SEPARATOR_W_C		L'/'

#define DIRECTORY_SEPARATOR_A		"/"
#define DIRECTORY_SEPARATOR_A_C		'/'
#endif

#ifdef UNICODE
#define DIRECTORY_SEPARATOR		DIRECTORY_SEPARATOR_W
#define DIRECTORY_SEPARATOR_C	DIRECTORY_SEPARATOR_W_C
#else
#define DIRECTORY_SEPARATOR		DIRECTORY_SEPARATOR_A
#define DIRECTORY_SEPARATOR_C	DIRECTORY_SEPARATOR_A_C
#endif

class CUtil
{
public:
	CUtil(void);
	~CUtil(void);

public:
	// Get list of all files in the target directory
	static void GetFileList(const _tstring& strTargetDirectoryPath, const _tstring& strWildCard, bool bLookInSubdirectories, vector<_tstring>& vecstrFileList);

public:
	// Add "\" to the end of a directory path, if not present
	static wstring AddDirectoryEnding(const wstring& strDirectoryPath);
	static string AddDirectoryEnding(const string& strDirectoryPath);

	// Remove "\" from the end of a directory path, if present
	static wstring RemoveDirectoryEnding(const wstring& strDirectoryPath);
	static string RemoveDirectoryEnding(const string& strDirectoryPath);

public:
	// Get the name of the directory form a given directory path: e.g. C:\Program Files\XYZ, will return XYZ
	static wstring GetDirectoryName(const wstring& strDirectoryPath);
	static string GetDirectoryName(const string& strDirectoryPath);
};
