#include <iostream>
#include <vector>
#include <cmath>
#include <ctime>
#include <string>

#include "range.hpp"


#if defined( PROGRAM_REDUCE )
static inline float foo( float i, float e ) {
    return i + e;
}

float reduce_demo(float *v, size_t length)
{
    return range( v, v + length )
        .reduce( foo );
}

#elif defined( PROGRAM_GENRANGE_OPS )

static inline float pow2( float v )
{
    return v * v;
}

void genrangeops_range(int *v, float *other, size_t length)
{
    range( v, v + length )
        .as<float>()
        .map( pow2 )
        .copyTo( range (other, other + length) );
}

#elif defined( PROGRAM_PIPED_CALLS )

void piped_calls(int *v, size_t length)
{
    auto r = range( v, v + length );
    int init = 0;
    for ( auto e : filter( []( int e ) { return e > 10;}, map( []( int e ) { return e * e;}, r ) ) ) {
        init += e;
    }
}

#else

template<class I>
std::ostream &operator<<(std::ostream &stream, GenericRange<I> range)
{
    for (auto i : range) {
        stream << i << " ";
    }
    return stream;
}

void example_header(int no)
{
    std::cout << std::endl << std::endl << "==============================" << std::endl
        << "  Example " << std::to_string(no) << ":" << std::endl << std::endl;
}

int main( int, char *[] ) {

    example_header(1);

    /*
     * Range tiling.
     *
     * Say we have a range [ 1, 2, 3, 4, 5, 6, 7, 8 ]. We can brake it into 2 by 4 tiles with:
     *
     * auto tiled_range = range.tile(4);
     *
     * Variable tiled_range contains a GenericRange<TilingIterator<typename decltype(range)::iterator> > typed range,
     * of 2 element, where each element is a range of 4 original iterator values from the starting range.
     */
    iota( 18 ) // let's use bit more numbers - use number not divisible by 4 on purpose.
        .tile(4)
        .each([] (auto e) { std::cout << e << std::endl; });

    example_header(2);

    /*
     * Let's do some more extensive processing over data.
     *
     * Run `make genrangeops-asm` to generate assembly output
     * of similar processing.
     */

    iota( 10 )
        .filter( [](auto e) { return e > 3; } )          // use only number greater than 3
        .map( [](auto e) { return e * 3 + 2; } )         // map data with given function
        .as<float>()                                     // static cast of data to float
        .map( [](auto e) { return std::pow(e, 2.2f); } ) // to the power of 2.2
        .each( [](auto e) { std::cout << e << " "; } );  // lazy functions are evaluated here.

    // String is a range of characters. Range ops work also on strings.

    example_header(3);

    /*
     * Splitting range with a delimiter.
     *
     * We can split a range by a given delimiter.
     */
    std::string some_string = "This is some String.";

    range( some_string )
        .split(' ')
        .each( [] (auto part) {
            std::cout << std::string( part.begin(), part.end() ) << std::endl; // construct string from range part.
         });

    example_header(4);

    /*
     * Let's split a text by line.
     *
     * Use `byLine` function, which wraps string into a
     * GenericRange<SplitIterator<string::iterator> > with
     * '\n' as delimiters.
     */

    std::string some_text = 
        "This is\n"
        "some text.\n"
        "There's also\n"
        "some more text right here.";

    byLine( some_text )
        .each ([] (auto line) {
            std::cout << std::string( line.begin(), line.end() ) << std::endl;
        });

    return 0;
}

#endif
