#pragma once
#include <string>

#include "wordpath.hpp"

class grid
{
public:
	grid( std::size_t h, std::size_t w, std::string const & letters );

	inline std::size_t height() const  throw() { return _h; }
	inline std::size_t width() const  throw() { return _w; }
	inline std::size_t size() const  throw() { return _s; }

	inline char operator[]( std::size_t pos ) const  throw() { return _l[ pos ]; }

	grid remove( wordpath const & path ) const throw();

	void display() const throw();
	void display( std::string const & word,  wordpath const & path ) const throw();

	bool empty() const throw();

private:
	std::size_t _h;
	std::size_t	_w;
	std::size_t	_s;
	std::string	_l;
};
