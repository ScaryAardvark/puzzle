#define NOMINMAX
#include <iostream>
#include <atomic>
#include <mutex>
#include <algorithm>
#include <fstream>
#include <numeric>
#include <unordered_set>
#include <set>
#include <string>
#include <sstream>
#include <list>
#include <tuple>
#include <bitset>
#include <tbb/tbb.h>

#include "puzzle.hpp"

puzzle::puzzle( wordcache const & words, bool verbose ) :
	_words( words ),
	_verbose( verbose )
{
}

void puzzle::search( grid const & letters, wordhints const & hints ) const throw()
{
	letters.display();

	std::deque< found > f;

	search( letters, f, hints.begin() );
}


void puzzle::search( grid const & letters, std::deque< found > const & f, wordhints::const_iterator const & hint ) const throw()
{
	char firstchar = 0;

	if ( !std::get< 1 >( *hint ).empty() )
		firstchar = std::get< 1 >( *hint )[ 0 ];

	tbb::parallel_for< std::size_t >( 0, letters.size(), [firstchar, &hint, &f, &letters, this]( std::size_t pos )
	{
		auto thischar = letters[ pos ];

		if ( thischar != ' ' && ( firstchar == 0 || ( firstchar != 0 && thischar == firstchar ) ) )
		{
			std::string word;
			wordpath path;

			search( letters, pos, path, word, f, hint );
		}
	} );
}

bool puzzle::isValidLocation( grid const & letters, std::size_t pos, wordpath const & path, char current_char ) const throw()
{
	// have we already been here
	if ( path.test( pos ) == true )	
		return false;

	char proposed_char = letters[ pos ];

	if ( proposed_char == ' ' )
		return false;

	return _words.isValidCharPairing( current_char, proposed_char );
}

void puzzle::addSolution( std::deque< found > const & fl ) const throw()
{
	std::stringstream ss;
	for ( auto const & f : fl )
		ss << std::get< 0 >( f ) << " ";

	std::lock_guard< std::mutex > g( _solutionLock );
	auto iret = _solutions.insert( ss.str() );

	if ( iret.second )
	{
		std::cout << *iret.first << std::endl;

		if ( _verbose )
		{
			for ( auto const & f : fl )
			{
				std::get< 1 >( f ).display( std::get< 0 >( f ), std::get< 2 >( f ) );
				std::cout << std::endl;
			}
		}
	}
}

void puzzle::search( grid const & letters, std::size_t pos, wordpath path, std::string word, std::deque< found > const & f, wordhints::const_iterator const & hint ) const throw()
{
	path.set( pos );

	char newchar = letters[ pos ];

	word += newchar;

	auto word_size_at_this_depth = std::get< 0 >( *hint );

	if ( word.size() == word_size_at_this_depth )
	{
		bool backwards = false;

		if ( isMatch( *hint, word, backwards ) )
		{
			if ( backwards )
				std::reverse( word.begin(), word.end() );

			std::deque< found > newfound( f );

			newfound.emplace_back( word, letters, path );

			grid newgrid = letters.remove( path );

			if ( newgrid.empty() )
			{
				addSolution( newfound );
			}
			else
			{
				search( newgrid, newfound, hint + 1 );
			}
		}
	}
	else
	{
		auto const & word_at_this_depth = std::get< 1 >( *hint );

		if ( ( word_at_this_depth.empty() && _words.canBeginWith( word ) ) ||
			( !word_at_this_depth.empty() && std::strncmp( word_at_this_depth.c_str(), word.c_str(), std::min( word_at_this_depth.size(), word.size() ) ) == 0 ) )
		{
			std::size_t newpos;

			if ( pos >= letters.width() )	// y > 0
			{
				if ( ( pos % letters.width() ) > 0 )
				{
					newpos = pos - letters.width() - 1;
					if ( isValidLocation( letters, newpos, path, newchar ) )
						search( letters, newpos, path, word, f, hint );
				}

				newpos = pos - letters.width();
				if ( isValidLocation( letters, newpos, path, newchar ) )
					search( letters, newpos, path, word, f, hint );

				if ( ( pos % letters.width() ) < ( letters.width() - 1 ) )
				{
					newpos = pos - letters.width() + 1;
					if ( isValidLocation( letters, newpos, path, newchar ) )
						search( letters, newpos, path, word, f, hint );
				}
			}

			if ( ( pos % letters.width() ) > 0 )
			{
				newpos = pos - 1;
				if ( isValidLocation( letters, newpos, path, newchar ) )
					search( letters, newpos, path, word, f, hint );
			}

			if ( ( pos % letters.width() ) < ( letters.width() - 1 ) )
			{
				newpos = pos + 1;
				if ( isValidLocation( letters, newpos, path, newchar ) )
					search( letters, newpos, path, word, f, hint );
			}

			if ( ( pos / letters.width() ) < ( letters.height() - 1 ) )
			{
				if ( ( pos % letters.width() ) > 0 )
				{
					newpos = pos + letters.width() - 1;
					if ( isValidLocation( letters, newpos, path, newchar ) )
						search( letters, newpos, path, word, f, hint );
				}

				newpos = pos + letters.width();
				if ( isValidLocation( letters, newpos, path, newchar ) )
					search( letters, newpos, path, word, f, hint );

				if ( ( pos % letters.width() ) < ( letters.width() - 1 ) )
				{
					newpos = pos + letters.width() + 1;
					if ( isValidLocation( letters, newpos, path, newchar ) )
						search( letters, newpos, path, word, f, hint );
				}
			}
		}
	}
}

bool puzzle::isMatch( wordhint const & hint, std::string const & word, bool backwards ) const throw()
{
	auto const & wsatd = std::get< 0 >( hint );
	auto const & watd = std::get< 1 >( hint );
	auto const & rwatd = std::get< 2 >( hint );

	if ( watd.size() == word.size() )
		return watd == word;

	if ( wsatd == rwatd.size() && rwatd == word )
	{
		backwards = true;
		return true;
	}

	return _words.isWord( word );
}
