#include <iostream>
#include <cassert>
#include <set>
#include <memory>
#include <cstring>
#include <bitset>

#include "grid.hpp"

grid::grid( std::size_t h, std::size_t w, std::string letters ) :
	_h( h ),
	_w( w ),
	_s( h * w ),
	_l( letters )
{
}

grid grid::remove( wordpath const & path ) const throw()
{
    // use a bitset to hold which columns we need to process
    // for moving characters down when letters are removed
    std::bitset< MAX_BITS > col_modified;

	std::string new_l( _l );

	for ( std::size_t p = 0 ; p < _s ; ++p )
	{
		if ( path.test( p ) )
		{
			new_l[ p ] = ' ';
			col_modified.set( p % _w );
		}
	}

	bool modified;
	char *lower, *upper;

	for ( std::size_t c = 0 ; c < _w ; ++c )
	{
		if ( col_modified.test( c ) == 0 )
			continue;

		do {
			modified = false;

			for ( std::size_t pos = c + _w ; pos < _s ; pos += _w )
			{
				lower = &new_l[ pos ];
				upper = lower - _w;

				if ( *lower == ' ' &&
					 *upper != ' ' )
				{
					std::swap( *upper, *lower );
					modified = true;
				}
			}
		} while( modified );
	}
	
	return grid( _h, _w, new_l );
}


void grid::display( ) const throw()
{
	for ( std::size_t p = 0 ; p < _s ; ++p )
	{
		if ( p > 0 && ( p % _w ) == 0 )
			std::cout << std::endl;

		std::cout << _l[ p ] << " ";
	}
	std::cout << std::endl;
}

void grid::display( std::string const & word,  wordpath const & path ) const throw()
{
	std::cout << word << " = " << std::endl;

	for ( std::size_t pos = 0 ; pos < _s ; ++pos )
	{
		if ( pos > 0 && ( pos % _w ) == 0 )
			std::cout << std::endl;

		if ( path.test( pos ) )
			std::cout << (char)::toupper( _l[ pos ] );
		else
			std::cout << _l[ pos ];

		std::cout << " ";
	}

	std::cout << std::endl;
}

bool grid::empty() const throw()
{
	for ( auto const & c : _l )
	{
		if ( c != ' ' )
			return false;
	}
	return true;
}
