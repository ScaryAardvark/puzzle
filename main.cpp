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

#include "wordcache.hpp"
#include "wordhint.hpp"
#include "wordpath.hpp"
#include "grid.hpp"
#include "puzzle.hpp"

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
        {
            // mainly for windows but we need to remove any quotes from the hints
            std::string::size_type quote;
            while( (quote = a.find_first_of( "\"'" ) ) != std::string::npos )
                a.replace( quote, 1, "" );
            args.push_back( a );
        }
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

    if ( (height * width) > MAX_BITS )
    {
        std::cerr << "Sorry, you need to recompile with MAX_BITS increased to " << ( height * width ) << std::endl;
        exit( 1 );
    }

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
                if ( args[ s ].empty() )
                {
                    std::cerr << v[ 0 ] << ": empty hint" << std::endl;
                    exit( 1 );
                }

                if ( !puzzle::wellFormedHint( args[ s ] ) )
                {
                    std::cerr << v[ 0 ] << ": hint \"" << args[ s ] << "\" is badly formed" << std::endl;
                    exit( 1 );
                }

                std::string w( args[ s ] );

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
