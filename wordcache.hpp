#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <tuple>

#include "wordhint.hpp"

size_t hash_value( std::tuple< char, char > const & t );

class wordcache
{
public:
	wordcache( std::string const & file, std::vector< wordhint > const & hints, std::string const & letters, std::string const & excludefile = "exclude.txt" );

	bool isValidCharPairing( char c1, char c2 ) const throw();

	std::size_t size() const throw();
	
	bool isWord( std::string const & word ) const throw();

	bool canBeginWith( std::string const & word ) const throw();

private:

    struct key_hash : public std::unary_function< std::tuple< char, char >, std::size_t>
    {
        std::size_t operator()( std::tuple< char, char > const & k ) const
        {
            return std::get< 0 >( k ) * 26 + std::get< 1 >( k ) ;
        }
    };

	std::vector< std::string >                                  _words;
	std::unordered_set< std::tuple< char, char >, key_hash >    _illegal_char_pairings;

    std::array< char, 676 > _pairingsInUse;
};

