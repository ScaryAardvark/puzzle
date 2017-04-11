#pragma once

#include <string>
#include <vector>

#include "wordhint.hpp"

class wordcache
{
public:
	wordcache( std::string const & file, std::vector< wordhint > const & hints, std::string const & letters, std::string const & excludefile = "exclude.txt" );

	bool isValidCharPairing( char c1, char c2 ) const throw();

	std::size_t size() const throw();
	
	bool isWord( std::string const & word ) const throw();

	bool canBeginWith( std::string const & word ) const throw();

private:
	std::vector< std::string >  _words;
	std::set< std::tuple< char, char > > _illegal_char_pairings;
};

