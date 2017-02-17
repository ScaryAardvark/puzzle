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
#include <tbb\tbb.h>

#include "wordcache.hpp"
#include "wordhint.hpp"
#include "wordpath.hpp"
#include "grid.hpp"

using namespace tbb;

typedef std::tuple< std::string, grid, wordpath > found;

class puzzle
{
public:
	puzzle( std::vector< wordhint > const & hints, wordcache const & words, bool verbose ) :
		_hints( hints ),
		_words( words ),
		_verbose( verbose )
	{
	}

	std::vector< wordhint > const & _hints;
	wordcache const & _words;

	bool _verbose;
	
	std::mutex _solutionLock;
	std::set< std::string > _solutions;

	std::atomic< std::size_t > _count;

	void search( grid const & letters )
	{
		letters.display();

		std::list< found > f;

		internalSearch( letters, f, 0 );

//		std::cout << _count << " searches" << std::endl;
	}
	
private:
	void internalSearch( grid const & letters, std::list< found > const & f, std::size_t depth )
	{
		if ( depth >= _hints.size() )
			return;

		char firstchar = 0;
		if ( !std::get< 1 >( _hints[ depth ] ).empty() )
			firstchar = std::get< 1 >( _hints[ depth ] )[ 0 ];
#ifdef _DEBUG
		for ( std::size_t pos = 0 ; pos < letters.size() ; ++pos )
		{
#else
		tbb::parallel_for< std::size_t >( 0, letters.size(), [firstchar, depth, &f, &letters, this]( std::size_t pos )
		{
#endif
			auto thischar = letters[ pos ];

			if ( thischar != ' ' && ( firstchar == 0 || ( firstchar != 0 && thischar == firstchar ) ) )
			{
				std::string word;
				wordpath path;

				search( letters, pos, path, word, f, depth );
			}
#ifdef _DEBUG
		}
#else
		} );
#endif
	}


	inline bool isValidLocation( grid const & letters, std::size_t pos, wordpath const & path, char current_char ) const 
	{
		// have we already been here
		if ( path.test( pos ) == true )	
			return false;

		char proposed_char = letters[ pos ];

		if ( proposed_char == ' ' )
			return false;

		if ( !_words.isValidCharPairing( current_char, proposed_char ) )
			return false;

		return true;
	}

	void addSolution( std::list< found > const & f, found const & last )
	{
		std::stringstream ss;
		for ( auto const & fp : f )
			ss << std::get< 0 >( fp ) << " ";
		ss << std::get< 0 >( last );
		std::lock_guard< std::mutex > g( _solutionLock );

		auto dsp = [ this ]( found const & f ) {
			std::cout << " : " << std::get< 0 >( f ) << " = " << std::endl;
			std::get< 1 >( f ).display( std::get< 2 >( f ) );
			std::cout << std::endl;
		};

		auto iret = _solutions.insert( ss.str() );
		if ( iret.second )
		{
			std::cout << ss.str() << std::endl;

			if ( _verbose )
			{
		 		for ( auto const & fp : f )
				{
					dsp( fp );
				}

				dsp( last );
			}
		}
	}

	void search( grid const & letters, std::size_t pos, wordpath path, std::string word, std::list< found > const & f, std::size_t depth )
	{
//		++_count;
		path.set( pos );

		char newchar = letters[ pos ];

		word += newchar;

#ifdef _DEBUG
		std::cout << "pos = " << pos << ", word = \"" << word << "\"" << std::endl;
#endif

		auto hint = _hints[ depth ];
		auto word_size_at_this_depth = std::get< 0 >( hint );

		if ( word.size() == word_size_at_this_depth )
		{
			bool backwards = false;

			if ( isMatch( hint, word, backwards ) )
			{
				if ( backwards )
					std::reverse( word.begin(), word.end() );

				grid newgrid = letters.remove( path );

				if ( newgrid.empty() )
				{
					addSolution( f, found( word, letters, path ) );
				}
				else
				{
					std::list< found > newfound( f );
					newfound.emplace_back( word, letters, path );

					internalSearch( newgrid, newfound, depth + 1 );
				}
			}
		}
		else
		{
			auto const & word_at_this_depth = std::get< 1 >( hint );
			if ( ( word_at_this_depth.empty() && _words.canBeginWith( word ) ) ||
				( !word_at_this_depth.empty() && std::strncmp( word_at_this_depth.c_str(), word.c_str(), min( word_at_this_depth.size(), word.size() ) ) == 0 ) )
			{
				std::size_t newpos;

				if ( pos >= letters.width() )	// y > 0
				{
					if ( ( pos % letters.width() ) > 0 )
					{
						newpos = pos - letters.width() - 1;
						if ( isValidLocation( letters, newpos, path, newchar ) )
							search( letters, newpos, path, word, f, depth );
					}

					newpos = pos - letters.width();
					if ( isValidLocation( letters, newpos, path, newchar ) )
						search( letters, newpos, path, word, f, depth );

					if ( ( pos % letters.width() ) < ( letters.width() - 1 ) )
					{
						newpos = pos - letters.width() + 1;
						if ( isValidLocation( letters, newpos, path, newchar ) )
							search( letters, newpos, path, word, f, depth );
					}
				}

				if ( ( pos % letters.width() ) > 0 )
				{
					newpos = pos - 1;
					if ( isValidLocation( letters, newpos, path, newchar ) )
						search( letters, newpos, path, word, f, depth );
				}

				if ( ( pos % letters.width() ) < ( letters.width() - 1 ) )
				{
					newpos = pos + 1;
					if ( isValidLocation( letters, newpos, path, newchar ) )
						search( letters, newpos, path, word, f, depth );
				}

				if ( ( pos / letters.width() ) < ( letters.height() - 1 ) )
				{
					if ( ( pos % letters.width() ) > 0 )
					{
						newpos = pos + letters.width() - 1;
						if ( isValidLocation( letters, newpos, path, newchar ) )
							search( letters, newpos, path, word, f, depth );
					}

					newpos = pos + letters.width();
					if ( isValidLocation( letters, newpos, path, newchar ) )
						search( letters, newpos, path, word, f, depth );

					if ( ( pos % letters.width() ) < ( letters.width() - 1 ) )
					{
						newpos = pos + letters.width() + 1;
						if ( isValidLocation( letters, newpos, path, newchar ) )
							search( letters, newpos, path, word, f, depth );
					}
				}
			}
		}
	}

	bool isMatch( wordhint const & hint, std::string const & word, bool backwards ) const
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
};

int main( int c, char *v[] )
{
	std::vector< std::string > args;

	bool verbose = false;

	for ( auto i = 1 ; i < c ; ++i )
	{
		std::string a( v[ i ] );
		if ( a == "-v" )
			verbose = true;
		else
			args.push_back( a );
	}

	if ( args.size() < 4  )
	{
		std::cerr << "usage: " << v[ 0 ] << " [ -v ] height width letters wordsize [ wordsize .. ]" << std::endl;
		std::cerr << "  Note. The sum of your words sizes much match ( height * width )" << std::endl;
		exit( 1 );
	}

	std::size_t height( std::atoi( args[ 0 ].c_str() ) );
	std::size_t width( std::atoi( args[ 1 ].c_str() ) );
	std::string letters( args[ 2 ] );

	if ( letters.size() != ( height * width ) )
	{
		std::cerr << v[ 0 ] << ": letter count (" << letters.size() << ") doesn't match grid size (" << height << "x" << width << ")" << std::endl;
		exit( 1 );
	}

	std::vector< wordhint > hints;

	{
		for ( std::size_t s = 3 ; s < args.size() ; ++s )
		{
			std::size_t val( std::atoi( args[ s ].c_str() ) );
			if ( val == 0 )
			{
				std::string w( args[ s ] );

				std::string::size_type quote;
				while( (quote = w.find_first_of( "\"'" ) ) != std::string::npos )
					w.replace( quote, 1, "" );
				
				val = w.size();
				
				auto last_char = w.find_last_not_of( " " );
				if ( last_char != std::string::npos )
					w = w.substr( 0, last_char + 1 );
				
				if ( val != w.size() )
					hints.emplace_back( val, w, "" );
				else
				{
					std::string reversed( w );
					std::reverse( reversed.begin(), reversed.end() );
					hints.emplace_back( val, w, reversed );
				}
			}
			else
				hints.emplace_back( val, "", "" );
		}

		std::size_t numletters = std::count_if( letters.begin(), letters.end(), []( char & c ) { return c != ' '; } );
		std::size_t numspaces = letters.size() - numletters;

		std::size_t sumsizes = std::accumulate( hints.begin(), hints.end(), std::size_t( 0 ),
			[]( std::size_t a, wordhint const & b )
			{
				return a + std::get< 0 >( b );
			}
		);

		if ( sumsizes != numletters )
		{
			std::cerr << v[ 0 ] << ": sum of word sizes (" << sumsizes << ") does not match number of letters (" << numletters << ")" << std::endl;
			if ( numspaces > 0 )
			{
				std::cerr << "You've supplied a partial grid, you maybe need to reduce your hints" << std::endl;
			}
			exit( 1 );
		}
	}

	wordcache words( "words.txt", hints, letters );

	std::cout << "loaded " << words.size() << " words" << std::endl;

	std::cout << "hints : ";
	for ( auto const & h : hints )
	{
		if ( std::get< 1 >( h ).empty() )
			std::cout << std::get< 0 >( h );
		else
		{
			std::cout << "\"" << std::get< 1 >( h ) << "\"";
			if ( std::get< 1 >( h ).size() != std::get< 0 >( h ) )
				std::cout << "(" << std::get< 0 >( h ) << ")";
		}
		std::cout << " ";
	}
	std::cout << std::endl;

	puzzle p( hints, words, verbose );

	p.search( grid( height, width, letters ) );
}
