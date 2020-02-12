#pragma once
#include "stdafx.h"
class AFileFilter
{
public:
	AFileFilter( LPCTSTR pszFilter ) :
	  m_strFilter( pszFilter ),
	  m_pszFilter( NULL )
	{
		m_pszFilter = m_strFilter.GetBuffer( 0 );

		LPTSTR		psz = m_pszFilter;

		while ( *psz )
		{
			LPTSTR		pszNext = ::CharNext( psz );

			if ( *psz == _T('|') )
				*psz = _T('\0');

			psz = pszNext;
		}

		return;
	}

	virtual ~AFileFilter( )
	{
		m_strFilter.ReleaseBuffer( );

		return;
	}

public:
	operator LPCTSTR( ) const
	{
		return ( m_pszFilter );
	}

protected:
	CString		m_strFilter;
	LPTSTR		m_pszFilter;
};
