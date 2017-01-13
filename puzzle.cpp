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

using namespace tbb;

#define MAX_BITS 64

typedef std::bitset< MAX_BITS > wordpath;
typedef std::tuple< std::size_t, std::string, std::string > hint;
typedef std::tuple< std::string, std::string, wordpath > found;


class wordcache
{
public:
	wordcache( std::string const & file, std::vector< hint > const & hints, std::string const & excludefile = "exclude.txt" )
	{
		std::string word;

		std::set< std::size_t > sizes;
		for ( auto const & h : hints )
			sizes.insert( std::get< 0 >( h ) );

		std::set< std::string > excluded;

		{
			std::fstream f( excludefile );
			while( f >> word )
			{
				std::transform( word.begin(), word.end(), word.begin(), ::tolower );

				excluded.insert( std::move( word ) );
			}
		}

		for ( auto const & h : hints )
		{
			if ( std::get< 0 >( h ) == std::get< 1 >( h ).size() )
				excluded.insert( std::get< 1 >( h ) );
		}

		std::fstream f( file );
		while( f >> word )
		{
			if ( sizes.find( word.size() ) == sizes.end() )
				continue;

			std::transform( word.begin(), word.end(), word.begin(), ::tolower );

			if ( excluded.find( word ) != excluded.end() )
			{
				std::cout << "Excluding \"" << word << "\"" << std::endl;
				continue;
			}

			_words.push_back( word );
		}

		std::sort( _words.begin(), _words.end() );
	}

	inline std::size_t size() const { return _words.size(); }
	
	bool isWord( std::string const & word ) const
	{
		return std::binary_search( _words.begin(), _words.end(), word );
	}

	bool canBeginWith( std::string const & word ) const
	{
		auto it = std::upper_bound( _words.begin(), _words.end(), word );

		if ( it == _words.end() )
			return false;

		return std::strncmp( it->c_str(), word.c_str(), word.size() ) == 0;
	}

private:
	std::vector< std::string >  _words;
};

class puzzle
{
public:
	puzzle( std::size_t my, std::size_t mx, std::vector< hint > const & hints, wordcache const & words, bool verbose ) :
		maxy( my ),
		maxx( mx ),
		_hints( hints ),
		_words( words ),
		_verbose( verbose )
	{
	}

	std::size_t const maxy;
	std::size_t const maxx;

	std::vector< hint > const & _hints;
	wordcache const & _words;

	bool _verbose;
	
	std::mutex _solutionLock;
	std::set< std::string > _solutions;

	std::atomic< std::size_t > _count;

	void search( std::string const & letters )
	{
		display( letters );

		std::list< found > f;

		internalSearch( letters, f, 0 );

//		std::cout << _count << " searches" << std::endl;
	}
	
private:
	inline std::size_t loc( std::size_t y, std::size_t x ) const throw()
	{
		return ( y * maxx ) + x;
	}
	
	inline char at( std::string const & letters, std::size_t y, std::size_t x ) const
	{
		return letters[ loc( y, x ) ];
	}

	void internalSearch( std::string const & letters, std::list< found > const & f, std::size_t depth )
	{
		if ( depth >= _hints.size() )
			return;

		char firstchar = 0;
		if ( !std::get< 1 >( _hints[ depth ] ).empty() )
			firstchar = std::get< 1 >( _hints[ depth ] )[ 0 ];
#ifdef _DEBUG
		for ( std::size_t y = 0 ; y < maxy ; ++y )
		{
			for ( std::size_t x = 0 ; x < maxx ; ++x )
			{
#else
		tbb::parallel_for< std::size_t >( 0, maxy, [firstchar, depth, &f, &letters, this]( std::size_t y )
		{
			tbb::parallel_for< std::size_t >( 0, maxx, [firstchar, depth, &f, &letters, y, this]( std::size_t x )
			{
#endif
				auto thischar = at( letters, y, x );

				if ( thischar != ' ' && ( firstchar == 0 || ( firstchar != 0 && thischar == firstchar ) ) )
				{
					std::string word;
					wordpath path;

					search( letters, y, x, path, word, f, depth );
				}
#ifdef _DEBUG
			}
		}
#else
			} );
		} );
#endif
	}

	std::string remove( std::string letters, wordpath const & path )
	{
		std::set< std::size_t > xs;

		for ( std::size_t p = 0 ; p < letters.size() ; ++p )
		{
			if ( path.test( p ) )
			{
				letters[ p ] = ' ';
				xs.insert( p % maxx );
			}
		}

		for ( auto const & x : xs )
		{
			level( letters, x );
		}

		return letters;
	}

	void level( std::string & letters, std::size_t x )
	{
		bool modified = false;

		for ( std::size_t y = 1 ; y < maxy ; ++y )
		{
			if ( at( letters, y, x ) == ' ' &&
				 at( letters, y - 1, x ) != ' ' )
			{
				std::swap( letters[ loc( y, x ) ], letters[ loc( y - 1, x ) ] );
				modified = true;
			}
		}

		if ( modified )
			level( letters, x );
	}

	inline bool isValidLocation( std::string const & letters, std::size_t pos, wordpath const & path ) const 
	{
		if ( letters[ pos ] == ' ' )
			return false;

		return path.test( pos ) == false;
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
			display( std::get< 1 >( f ), std::get< 2 >( f ) );
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

	void search( std::string const & letters, std::size_t y, std::size_t x, wordpath path, std::string word, std::list< found > const & f, std::size_t depth )
	{
//		++_count;
		std::size_t pl( loc( y, x ) );
		
		path.set( pl );

		char newchar = letters[ pl ];
		word += newchar;

#ifdef _DEBUG
		std::cout << "word = \"" << word << "\"" << std::endl;
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

				std::string newletters = remove( letters, path );

				std::size_t nonspace = newletters.find_first_not_of( " " );

				if ( nonspace == std::string::npos )
				{
					addSolution( f, found( word, letters, path ) );
				}
				else
				{
					std::list< found > newfound( f );
					newfound.emplace_back( word, letters, path );

					internalSearch( newletters, newfound, depth + 1 );
				}
			}
		}
		else
		{
			auto const & word_at_this_depth = std::get< 1 >( hint );
			if ( ( word_at_this_depth.empty() && _words.canBeginWith( word ) ) ||
				( !word_at_this_depth.empty() && std::strncmp( word_at_this_depth.c_str(), word.c_str(), min( word_at_this_depth.size(), word.size() ) ) == 0 ) )
			{
				if ( y > 0 )
				{
					if ( x > 0 )
					{
						if ( isValidLocation( letters, loc( y - 1, x - 1 ), path ) )
							search( letters, y - 1, x - 1, path, word, f, depth );
					}

					if ( isValidLocation( letters, loc( y - 1, x ), path ) )
						search( letters, y - 1, x, path, word, f, depth );

					if ( x < ( maxx - 1 ) )
					{
						if ( isValidLocation( letters, loc( y - 1, x + 1 ), path ) )
							search( letters, y - 1, x + 1, path, word, f, depth );
					}
				}

				if ( x > 0 )
				{
					if ( isValidLocation( letters, loc( y, x - 1 ), path ) )
						search( letters, y, x - 1, path, word, f, depth );
				}

				if ( x < ( maxx - 1 ) )
				{
					if ( isValidLocation( letters, loc( y, x + 1 ), path ) )
						search( letters, y, x + 1, path, word, f, depth );
				}

				if ( y < ( maxy - 1 ) )
				{
					if ( x > 0 )
					{
						if ( isValidLocation( letters, loc( y + 1, x - 1 ), path ) )
						search( letters, y + 1, x - 1, path, word, f, depth );
					}

					if ( isValidLocation( letters, loc( y + 1, x ), path ) )
						search( letters, y + 1, x, path, word, f, depth );

					if ( x < ( maxx - 1 ) )
					{
						if ( isValidLocation( letters, loc( y + 1, x + 1 ), path ) )
							search( letters, y + 1, x + 1, path, word, f, depth );
					}
				}
			}
		}
	}

	bool isMatch( hint const & hint, std::string const & word, bool backwards ) const
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

	void display( std::string const & letters ) const
	{
		for ( std::size_t y = 0 ; y < maxy ; ++y )
		{
			for ( std::size_t x = 0 ; x < maxx ; ++x )
			{
				std::cout << letters[ (y * maxx) + x ] << " ";
			}
			std::cout << std::endl;
		}
	}

	void display( std::string const & letters, wordpath const & path ) const
	{
		for ( std::size_t y = 0 ; y < maxy ; ++y )
		{
			for ( std::size_t x = 0 ; x < maxx ; ++x )
			{
				std::size_t pos = loc( y, x );

				if ( path.test( pos ) )
					std::cout << (char)::toupper( letters[ pos ] );
				else
					std::cout << letters[ pos ];
				
				std::cout << " ";
			}
			std::cout << std::endl;
		}
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

	std::vector< hint > hints;

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
			[]( std::size_t a, hint const & b )
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

	wordcache words( "words.txt", hints );

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

	puzzle p( height, width, hints, words, verbose );

	p.search( letters );
}
