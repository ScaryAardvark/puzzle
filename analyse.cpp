#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <iterator>
#include <vector>

typedef std::map< std::string, int >    wordcount;

typedef std::map< int, wordcount > wordmap;

int main( int c, char *v[] )
{
    wordmap wm;
    std::size_t maxprint = 5;

    if ( c > 1 )
    {
        maxprint = atoi( v[ 1 ] );
    }

    for ( std::string line ; std::getline( std::cin, line ) ; )
    {
        std::vector< std::string > words;

        std::stringstream ss( line );

        std::copy( std::istream_iterator< std::string >( ss ), std::istream_iterator< std::string >(), std::back_inserter( words ) );

        for ( std::size_t i = 0 ; i < words.size() ; ++i )
        {
            auto wmi = wm.find( i );
            if ( wmi == wm.end() )
            {
                auto iret = wm.insert( std::make_pair( i, wordmap::mapped_type{} ) );
                wmi = iret.first;
            }

            auto it = wmi->second.find( words[ i ] );
            if ( it == wmi->second.end() )
            {
                auto iret = wmi->second.insert( std::make_pair( words[ i ], 0 ) );
                it = iret.first;
            }

            ++it->second;
        }
    }

    // ok, we have our word map. we need to print out the top 5
    // words by count for each level

    for ( auto const & wcm : wm )
    {
        std::cout << wcm.first + 1 << std::endl;

        // ok need to sort wm by value rather than key

        std::multimap< int, std::string, std::greater< int > > sm;

        for ( auto const & wc : wcm.second )
        {
            sm.insert( std::make_pair( wc.second, wc.first ) );
        }

        std::size_t n = 0u;
        for ( auto const & smi : sm )
        {
            std::cout << "    " << smi.second << " " << smi.first << std::endl;
            ++n;
            if ( n >= maxprint )
                break;
        }
    }
}
