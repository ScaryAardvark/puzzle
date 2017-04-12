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

typedef std::tuple< std::string, grid, wordpath > found;

class puzzle
{
public:
    puzzle( std::vector< wordhint > const & hints, wordcache const & words, bool verbose );

    void search( grid const & letters ) const throw();

    static bool wellFormedHint( std::string const & word ) throw();

private:

    void search( grid const & letters, std::deque< found > const & f, std::size_t depth ) const throw();

    inline bool isValidLocation( grid const & letters, std::size_t pos, wordpath const & path, char current_char ) const throw();

    void addSolution( std::deque< found > const & fl ) const throw();

    void search( grid const & letters, std::size_t pos, wordpath path, std::string word, std::deque< found > const & f, std::size_t depth ) const throw();

    bool isMatch( wordhint const & hint, std::string const & word, bool backwards ) const throw();

    std::vector< wordhint > const &     _hints;
    wordcache const &                   _words;
    bool                                _verbose;
    mutable std::mutex                  _solutionLock;
    mutable std::set< std::string >     _solutions;
    mutable std::atomic< std::size_t >  _count;
};

