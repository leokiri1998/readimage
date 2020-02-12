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

#include "stdafx.h"

#include "Util.h"

#include <stdio.h>
#include <malloc.h>
#include <tchar.h>
#include <wchar.h>
#include <ctime>
#include <direct.h>
#include <math.h>
#include <algorithm>

#include <sstream>
#include <cstring>
#include <iostream>
#include <iomanip>

CUtil::CUtil(void)
{
}

CUtil::~CUtil(void)
{
}

//////////////////////////////////////////////////////////////////////////
// Directory/File manipulation functions
//////////////////////////////////////////////////////////////////////////
#ifdef WIN32
void CUtil::GetFileList(const _tstring& strTargetDirectoryPath, const _tstring& strWildCard, bool bLookInSubdirectories, vector<_tstring>& vecstrFileList)
{
	// Check whether target directory string is empty
	if(strTargetDirectoryPath.compare(_T("")) == 0)
	{
		return;
	}

	// Remove "\\" if present at the end of the target directory
	// Then make a copy of it and use as the current search directory
	_tstring strCurrentDirectory = RemoveDirectoryEnding(strTargetDirectoryPath);

	// This data structure stores information about the file/folder that is found by any of these Win32 API functions:
	// FindFirstFile, FindFirstFileEx, or FindNextFile function
	WIN32_FIND_DATA fdDesktop = {0};

	// Format and copy the current directory
	// Note the addition of the wildcard *.*, which represents all files
	// 
	// Below is a list of wildcards that you can use
	// * (asterisk)			- represents zero or more characters at the current character position
	// ? (question mark)	- represents a single character
	//
	// Modify this function so that the function can take in a search pattern with wildcards and use it in the line below to find for e.g. only *.mpg files
	//
	// "\\?\" prefix to the file path means that the file system supports large paths/filenames
	_tstring strDesktopPath = _T("");
	strDesktopPath += _T("\\\\?\\");
	strDesktopPath += strCurrentDirectory;
	strDesktopPath = AddDirectoryEnding(strDesktopPath);

	if(strWildCard.compare(_T("")) == 0)
	{
		strDesktopPath += _T("*.*");
	}
	else
	{
		strDesktopPath += strWildCard;
	}

	// Finds the first file and populates the WIN32_FIND_DATA data structure with its information
	// The return value is a search handle used in subsequent calls to FindNextFile or FindClose functions
	HANDLE hDesktop = ::FindFirstFile(strDesktopPath.c_str(), &fdDesktop);	

	// If an invalid handle is returned by FindFirstFile function, then the directory is empty, so nothing to do but quit
	if(hDesktop == INVALID_HANDLE_VALUE)
	{
		return;
	}

	// Do this on the first file found and repeat for every next file found until all the required files that match the search pattern are found
	do 
	{
		// Reconstruct the path
		_tstring strPath = _T("");
		strPath += strCurrentDirectory;
		strPath = AddDirectoryEnding(strPath);
		strPath += fdDesktop.cFileName;

		// Check if a directory was found
		if(fdDesktop.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// Get the name of the directory
			_tstring strCurrentDirectoryName = GetDirectoryName(strPath);

			// If its a current (.) or previous (..) directory indicator, just skip it
			if((strCurrentDirectoryName.compare(_T(".")) == 0) || (strCurrentDirectoryName.compare(_T("..")) == 0))
			{
				continue;
			}
			// Other wise this is a sub-directory
			else
			{
				// Check whether function was called to include sub-directories in the search
				if(bLookInSubdirectories)
				{
					// If sub-directories are to be searched as well, recursively call the function again, with the target directory as the sub-directory
					GetFileList(strPath, strWildCard, bLookInSubdirectories, vecstrFileList);
				}
			}
		}
		// A file was found
		else
			// if(fdDesktop.dwFileAttributes & FILE_ATTRIBUTE_NORMAL)
		{
			// Add the string to the vector
			vecstrFileList.push_back(_tstring(strPath));
		}
	}
	// Search for the next file that matches the search pattern
	while(::FindNextFile(hDesktop, &fdDesktop) == TRUE);

	// Close the search handle
	::FindClose(hDesktop);
}
#endif

//////////////////////////////////////////////////////////////////////////
// Directory/File path string manipulation functions
//////////////////////////////////////////////////////////////////////////
wstring CUtil::AddDirectoryEnding(const wstring& strDirectoryPath)
{
	if(strDirectoryPath.compare(L"") == 0)
	{
		return wstring(L"");
	}

	wstring strDirectory = strDirectoryPath;

	if(strDirectory[strDirectory.size() - 1] != DIRECTORY_SEPARATOR_W_C)
	{
		strDirectory += DIRECTORY_SEPARATOR_W;
	}

	return strDirectory;
}

string CUtil::AddDirectoryEnding(const string& strDirectoryPath)
{
	if(strDirectoryPath.compare("") == 0)
	{
		return string("");
	}

	string strDirectory = strDirectoryPath;

	if(strDirectory[strDirectory.size() - 1] != DIRECTORY_SEPARATOR_A_C)
	{
		strDirectory += DIRECTORY_SEPARATOR_A;
	}

	return strDirectory;
}

wstring CUtil::RemoveDirectoryEnding(const wstring& strDirectoryPath)
{
	if(strDirectoryPath.compare(L"") == 0)
	{
		return wstring(L"");
	}

	wstring strDirectory = strDirectoryPath;

	if(strDirectory[strDirectory.size() - 1] == DIRECTORY_SEPARATOR_W_C)
	{
		strDirectory.erase(strDirectory.size() - 1);
	}

	return strDirectory;
}

string CUtil::RemoveDirectoryEnding(const string& strDirectoryPath)
{
	if(strDirectoryPath.compare("") == 0)
	{
		return string("");
	}

	string strDirectory = strDirectoryPath;

	if(strDirectory[strDirectory.size() - 1] == DIRECTORY_SEPARATOR_A_C)
	{
		strDirectory.erase(strDirectory.size() - 1);
	}

	return strDirectory;
}

wstring CUtil::GetDirectoryName(const wstring& strDirectoryPath)
{
	if(strDirectoryPath.compare(L"") == 0)
	{
		return wstring(L"");
	}

	wstring strDirectoryName = RemoveDirectoryEnding(strDirectoryPath);

	size_t i64Index = strDirectoryName.find_last_of(DIRECTORY_SEPARATOR_W);

	if(i64Index == wstring::npos)
	{
		return wstring(L"");
	}

	strDirectoryName.erase(0, i64Index + 1);

	return strDirectoryName;
}

string CUtil::GetDirectoryName(const string& strDirectoryPath)
{
	if(strDirectoryPath.compare("") == 0)
	{
		return string("");
	}

	string strDirectoryName = RemoveDirectoryEnding(strDirectoryPath);

	size_t i64Index = strDirectoryName.find_last_of(DIRECTORY_SEPARATOR_A);

	if(i64Index == string::npos)
	{
		return string("");
	}

	strDirectoryName.erase(0, i64Index + 1);

	return strDirectoryName;
}
