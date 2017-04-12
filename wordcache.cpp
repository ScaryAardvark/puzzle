#include <iostream>
#include <fstream>
#include <set>
#include <algorithm>
#include <list>
#include <cstring>

#include "wordcache.hpp"

wordcache::wordcache( std::string const & file, std::vector< wordhint > const & hints, std::string const & letters, std::string const & excludefile )
{
	std::vector< std::string > excluded;

	{
		std::fstream f( excludefile );
		std::string word;
		while( f >> word )
		{
			std::transform( word.begin(), word.end(), word.begin(), ::tolower );

			excluded.emplace_back( std::move( word ) );
		}

		std::sort( excluded.begin(), excluded.end() );
	}

	std::string uniqueLetters;

	{
		std::set< char > chars;
		for ( auto const & c : letters )
			chars.insert( c );

		for ( auto const & c : chars )
			uniqueLetters.push_back( c );
	}

	std::cout << uniqueLetters.size() << " unique letters" << std::endl;
	std::cout << "	";
	for ( auto const & c : uniqueLetters )
		std::cout << c << " ";
	std::cout << std::endl;

	{
		std::fstream f( file );
		std::string word;
		while( f >> word )
		{
			if ( std::binary_search( excluded.begin(), excluded.end(), word ) )
				continue;

			if ( std::find_if( hints.begin(), hints.end(), [ &word ]( wordhint const & h ) { return std::get< 0 >( h ) == word.size(); } ) == hints.end() )
				continue;

			if ( word.find_first_not_of( uniqueLetters ) != std::string::npos )
				continue;

			std::transform( word.begin(), word.end(), word.begin(), ::tolower );

			_words.emplace_back( std::move( word ) );
		}

		std::sort( _words.begin(), _words.end() );
	}

	std::list< std::string > char_pairings;

	{
		char buf[ 2 ];
		for ( auto const & i : uniqueLetters )
		{
			for ( auto const & j : uniqueLetters )
			{
				buf[ 0 ] = i;
				buf[ 1 ] = j;
				char_pairings.emplace_back( buf, 2 );
			}
		}
	}

	std::cout << char_pairings.size() << " possible char pairings" << std::endl;

	for ( auto it = char_pairings.begin() ; it != char_pairings.end() ; )
	{
		bool found = false;

		for ( auto const & w : _words )
		{
			auto f = w.find( *it );

			if ( f != std::string::npos )
			{
				found = true;
				break;
			}
		}

		if ( found )
			char_pairings.erase( it++ );
		else
			++it;
	}

	for ( auto const & cp : char_pairings )
		_illegal_char_pairings.insert( std::make_tuple( cp[ 0 ], cp[ 1 ] ) );

	std::cout << _illegal_char_pairings.size() << " illegal char pairings" << std::endl;
	std::cout << "	";
	for ( auto const & cp : _illegal_char_pairings )
		std::cout << std::get< 0 >( cp ) << std::get< 1 >( cp ) << " ";
	std::cout << std::endl;

	// can't remove hints until now as they must participate in char_pairing analysis
	for ( auto const & h : hints )
	{
		if ( std::get< 0 >( h ) == std::get< 1 >( h ).size() )
			_words.erase( std::remove( _words.begin(), _words.end(), std::get< 1 >( h ) ), _words.end() );
	}
}

bool wordcache::isValidCharPairing( char c1, char c2 ) const throw()
{
	return _illegal_char_pairings.find( std::make_tuple( c1, c2 ) ) == _illegal_char_pairings.end();
}

std::size_t wordcache::size() const throw()
{
	return _words.size();
}
	
bool wordcache::isWord( std::string const & word ) const throw()
{
	return std::binary_search( _words.begin(), _words.end(), word );
}

bool wordcache::canBeginWith( std::string const & word ) const throw()
{
	auto it = std::upper_bound( _words.begin(), _words.end(), word );

	if ( it == _words.end() )
		return false;

	return std::strncmp( it->c_str(), word.c_str(), word.size() ) == 0;
}
