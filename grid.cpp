#include <iostream>
#include <set>

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
	std::set< std::size_t > xs;

	std::string new_l( _l );

	for ( std::size_t p = 0 ; p < _s ; ++p )
	{
		if ( path.test( p ) )
		{
			new_l[ p ] = ' ';
			xs.insert( p % _w );
		}
	}

	bool modified;

	do {
		modified = false;

		for ( auto const & x : xs )
		{
			for ( std::size_t pos = x + _w ; pos < _s ; pos += _w )
			{
				if ( new_l[ pos ] == ' ' &&
					 new_l[ pos - _w ] != ' ' )
				{
					std::swap( new_l[ pos ], new_l[ pos - _w ] );
					modified = true;
				}
			}
		}
	} while( modified );
	
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

void grid::display( wordpath const & path ) const throw()
{
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