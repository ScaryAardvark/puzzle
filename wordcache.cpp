#include <iostream>
#include <numeric>
#include <fstream>
#include <algorithm>
#include <deque>
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
		std::unordered_set< char > chars;
		for ( auto const & c : letters )
			chars.insert( c );

		for ( auto const & c : chars )
			uniqueLetters.push_back( c );
	}

	std::cout << uniqueLetters.size() << " unique letters : ";
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

	std::deque< std::string > char_pairings;

    // we now do some analysis of words and the letters in the grid
    // searching for letter pairings, i.e. if  a combination of
    // two letters from the unique letters isn't found in the words then
    // we get rid of that char pairing as an optimisation. The search
    // algorithm knows which letter it's currently "on" and the next
    // letter it's proposing to search next. if the two pair combinations
    // aren't possible then we can reject that search.
    //
    // for example.
    //
    // say we have a unique set of letters : uhovnalwetirgsc
    // we produce a large set of possible char pairings, i.e. uh, wn, og
    // we then search all the words and see if those char pairings exist.
    // if the char pairing exists, i.e. it's possible that a word could be
    // chosen with it. then we remember it and allow it to be chosen
    // by the search


    // Build all possible character combinations based on letter grid
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

    _pairingsInUse.fill( 2 );   // fill with 2 to indicate that this char pairing wasn't even considered

    // now see if these char pairings can be found in the list of words we've recovered.
    // if not, then they are illegal and will be used to decide whether to 
    // continue searching
    //
    // Note. there are 676 possible 2 character pairings (26^2)
    // for speed we'll use a std::array< char >( 676 ) as flags
    // ( we could use bitset but it'd be slightly slower due to the arithmetic involved )
    //
	for ( auto it = char_pairings.begin() ; it != char_pairings.end() ; ++it )
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

        std::size_t idx = ( ( (*it)[ 0 ] - 'a' ) * 26 ) + ( (*it)[ 1 ] - 'a' );
		if ( found )
        {
            _pairingsInUse[ idx ] = 1;  // mark this as a legal char pairing
        }
        else
        {
            _pairingsInUse[ idx ] = 0;  // mark this as considered, but not found
        }
	}

    std::cout << std::accumulate( _pairingsInUse.begin(), _pairingsInUse.end(), std::size_t{0}, []( std::size_t c1, char c2 ) { return c1 + ( ( c2 == 1 ) ? 1 : 0 ); } ) << " char pairings found in available words" << std::endl;

    std::cout << "These pairings were rejected : ";
    for ( std::size_t p{ 0 } ; p < _pairingsInUse.size() ; ++p )
    {
        if ( _pairingsInUse[ p ] == 0 )
        {
            std::cout << (char)( ( p / 26 ) + 'a' ) << (char)( ( p % 26 ) + 'a' ) << " ";
        }
    }
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
	return _pairingsInUse[ ( ( c1 - 'a' ) * 26 ) + ( c2 - 'a' ) ] == 1;
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
