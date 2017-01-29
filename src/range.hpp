#ifndef RANGE_HPP_
#define RANGE_HPP_

#include <cstddef>
#include <iterator>
#include <cassert>
#include <type_traits>


// Macro for iterator comparison operator implementation.
#define ITERATOR_WRAPPER_COMPARISON_IMPL(IteratorName, ComparingPart)   \
    bool operator==(const IteratorName &other) const                    \
    {                                                                   \
        return ComparingPart == other.ComparingPart;                    \
    }                                                                   \
    bool operator!=(const IteratorName &other) const                    \
    {                                                                   \
        return !operator==(other);                                      \
    }                                                                   \
    bool operator>=(const IteratorName &other) const                    \
    {                                                                   \
        return ComparingPart >= other.ComparingPart;                    \
    }                                                                   \
    bool operator>(const IteratorName &other) const                     \
    {                                                                   \
        return ComparingPart > other.ComparingPart;                     \
    }                                                                   \
    bool operator<=(const IteratorName &other) const                    \
    {                                                                   \
        return ComparingPart <= other.ComparingPart;                    \
    }                                                                   \
    bool operator<(const IteratorName &other) const                     \
    {                                                                   \
        return ComparingPart < other.ComparingPart;                     \
    }

// Macro for specific random access iterator implementation used in this module.
#define RANDOM_ACCESS_ITERATOR_WRAPPER_IMPL(IteratorName, ConstructionParams...)    \
    IteratorName &operator++()                                                      \
    {                                                                               \
        ++iter;                                                                     \
        return *this;                                                               \
    }                                                                               \
    IteratorName operator++(int)                                                    \
    {                                                                               \
        auto t(*this);                                                              \
        ++iter;                                                                     \
        return t;                                                                   \
    }                                                                               \
    IteratorName &operator--()                                                      \
    {                                                                               \
        --iter;                                                                     \
        return *this;                                                               \
    }                                                                               \
    IteratorName operator--(int)                                                    \
    {                                                                               \
        auto t(*this);                                                              \
        --iter;                                                                     \
        return t;                                                                   \
    }                                                                               \
    IteratorName operator+(size_t c)                                                \
    {                                                                               \
        return IteratorName(iter + c, ConstructionParams);                          \
    }                                                                               \
    IteratorName operator-(size_t c)                                                \
    {                                                                               \
        return IteratorName(iter - c, ConstructionParams);                          \
    }                                                                               \
    ptrdiff_t operator-(const IteratorName &other) const                            \
    {                                                                               \
        return std::distance(other.iter, iter);                                     \
    }                                                                               \
    bool operator==(const IteratorName &other) const                                \
    {                                                                               \
        return iter == other.iter;                                                  \
    }                                                                               \
    bool operator!=(const IteratorName &other) const                                \
    {                                                                               \
        return !operator==(other);                                                  \
    }                                                                               \
    bool operator>=(const IteratorName &other) const                                \
    {                                                                               \
        return iter >= other.iter;                                                  \
    }                                                                               \
    bool operator>(const IteratorName &other) const                                 \
    {                                                                               \
        return iter > other.iter;                                                   \
    }                                                                               \
    bool operator<=(const IteratorName &other) const                                \
    {                                                                               \
        return iter <= other.iter;                                                  \
    }                                                                               \
    bool operator<(const IteratorName &other) const                                 \
    {                                                                               \
        return iter < other.iter;                                                   \
    }

template<class I> struct GenericRange; // forward declaration used in iterators.

namespace detail {

/*
 * Iterator implementation details used to construct range utilities in this module.
 */

template<class I, class Fn>
struct MapIterator :
    public std::iterator<
    typename std::iterator_traits<I>::iterator_category,
    typename std::iterator_traits<I>::value_type >  {

    using value_type = typename std::result_of<Fn( typename std::iterator_traits<I>::value_type )>::type;
    using pointer = value_type;
    using reference = value_type;

    I iter;
    Fn fn;

    MapIterator( I iter, Fn fn ) : iter( iter ), fn( fn ) {}

    value_type operator*() {
        return fn( *iter );
    }

    RANDOM_ACCESS_ITERATOR_WRAPPER_IMPL( MapIterator, fn )
};

template<class I>
struct SplitIterator :
    public std::iterator<
    std::forward_iterator_tag,
    GenericRange<I>,
    ptrdiff_t,
    GenericRange<I>,
    GenericRange<I>
    > {
private:
    using T = typename std::iterator_traits<I>::value_type;

    I iter;
    I end;
    T delimiter;

    I pe_cache;

    void reparse_for_end() {
        if ( pe_cache == iter ) {
            while( ++pe_cache != end && *pe_cache != delimiter ) {}
        }
        assert( iter != pe_cache );
    }

public:

    SplitIterator( I iter, I end, T delimiter ) : iter( iter ), end( end ), delimiter( delimiter ), pe_cache( iter ) {}

    GenericRange<I> operator*() {
        reparse_for_end();
        return GenericRange<I>( iter, pe_cache );
    }

    SplitIterator &operator++() {
        reparse_for_end();
        if ( pe_cache != end )
            ++pe_cache; // skip delimiter element
        iter = pe_cache;
        return *this;
    }

    SplitIterator operator++( int ) {
        auto t( *this );
        reparse_for_end();
        if ( pe_cache != end )
            ++pe_cache; // skip delimiter element;
        iter = pe_cache;
        return t;
    }

    ITERATOR_WRAPPER_COMPARISON_IMPL( SplitIterator, iter )
};

template<class I>
struct TilingIterator :
    public std::iterator<
    std::forward_iterator_tag,
    GenericRange<I>,
    ptrdiff_t,
    GenericRange<I>,
    GenericRange<I>
    > {
private:
    I iter;
    I end; // tile end

public:
    TilingIterator( I iter, I end ) : iter( iter ), end( end ) {}

    GenericRange<I> operator*();

    TilingIterator &operator++() {
        auto d = std::distance( iter, end );
        iter = end;
        end = iter + d;
        return *this;
    }

    TilingIterator operator++( int ) {
        auto t( *this );
        auto d = std::distance( iter, end );
        iter = end;
        end = iter + d;
        return t;
    }

    ITERATOR_WRAPPER_COMPARISON_IMPL( TilingIterator, iter )
};

template<class I, class Fn>
struct FilterIterator :
    public std::iterator<
    std::forward_iterator_tag,
    typename std::iterator_traits<I>::value_type,
    ptrdiff_t,
    typename std::iterator_traits<I>::pointer,
    typename std::iterator_traits<I>::reference>  {

    I iter;
    I end;
    Fn fn;

    FilterIterator( I iter, I end, Fn fn ) : iter( iter ), end( end ), fn( fn ) {}

    auto operator*() {
        while( iter != end && !fn( *iter ) ) {
            ++iter;
        }
        return *iter;
    }

    FilterIterator &operator++()
    {
        if (iter != end)
            ++iter;
        return *this;
    }

    FilterIterator operator++(int)
    {
        auto t(*this);
        if (iter != end)
            ++iter;
        return t;
    }

    ITERATOR_WRAPPER_COMPARISON_IMPL(FilterIterator, iter);
};

template<class I, class J = I>
struct IotaIterator :
    public std::iterator< std::random_access_iterator_tag, I, ptrdiff_t, I, I> {

    I iter;
    J jump;

    IotaIterator( I iter, J jump ) : iter( iter ), jump( jump ) {}

    auto operator*() {
        return iter;
    }
    IotaIterator &operator++() {
        iter += jump;
        return *this;
    }
    IotaIterator operator++( int ) {
        auto t( *this );
        iter += jump;
        return t;
    }
    IotaIterator &operator--() {
        iter -= jump;
        return *this;
    }
    IotaIterator operator--( int ) {
        auto t( *this );
        iter -= jump;
        return t;
    }
    IotaIterator operator+( size_t c ) {
        return IotaIterator( iter + c * jump, jump );
    }
    IotaIterator operator-( size_t c ) {
        return IotaIterator( iter - c * jump, jump );
    }
    ptrdiff_t operator-( const IotaIterator &other ) const {
        assert( jump == other.jump );
        auto d = ( iter - other.iter );
        assert( d % jump == 0 );
        return d / jump;
    }

    ITERATOR_WRAPPER_COMPARISON_IMPL( IotaIterator, iter )
};


template<class T, class O>
T cast( O o ) {
    return static_cast<T>( o );
}

} // end of detail

/**
 * Generic range utility type.
 *
 * Represents a thin wrapper over generic iterator range, which is
 * defined with `begin` and `end`.
 */
template<class I>
struct GenericRange {

public:
    using iterator = I;
    using value_type = typename std::iterator_traits<I>::value_type;
    using pointer = typename std::iterator_traits<I>::pointer;
    using referece = typename std::iterator_traits<I>::reference;

private:
    I _b, // beginning iterator of the range.
    _e; // ending iterator of the range.

public:
    GenericRange() = delete; // void range is invalid.

    //! Range constructor.
    GenericRange( I b, I e ) : _b( b ), _e( e ) {}

    //! Beginning of the range.
    auto begin() {
        return _b;
    }

    //! Ending of the range.
    auto end() {
        return _e;
    }

    //! Length of the range.
    auto size() const {
        return std::distance( _b, _e );
    }

    // Range utility wrappers.

    template<class Fn> void each( Fn fn );
    template<class Fn> GenericRange<detail::MapIterator<I, Fn> > map( Fn fn );
    template<class T> GenericRange<detail::MapIterator<I, T( * )( value_type ) > > as();
    template<class Fn> GenericRange<detail::FilterIterator<I, Fn> > filter( Fn fn );
    template<class Fn> value_type reduce( Fn fn );
    template<class Fn> value_type fold( Fn fn, value_type init );
    GenericRange<I> take( size_t n );
    GenericRange<I> drop( size_t n = 1 );
    GenericRange<I> tail( size_t n );
    template<class O> void copyTo( GenericRange<O> other ) const;
    template<class Range> void copyTo( Range &other ) const;
    auto tile( size_t tile_length );
    GenericRange<detail::SplitIterator<I> > split( value_type delimiter );
};

template<class T>
struct is_generic_range : std::false_type {};

template<class I>
struct is_generic_range<GenericRange<I> > : std::true_type {};

//! Generic range wrapper constructor.
template<class I>
auto range( I begin, I end ) {
    return GenericRange<I>( begin, end );
}

//! Generic range wrapper constructor.
template<class Range>
auto range( Range &r ) {
    return GenericRange<typename Range::iterator>( r.begin(), r.end() );
}

//! Generic range wrapper constructor.
template<class Range>
auto range( const Range &r ) {
    return GenericRange<typename Range::const_iterator>( r.begin(), r.end() );
}

/**
 * @brief Tile the range to sub-ranges of given lenght.
 *
 * @param range Range to be tiled.
 * @param tile_size Size of tile.
 *
 * @return Range of tiles.
 */
template<class Range>
auto tile( Range range, size_t tile_size ) {
    int el = static_cast<int>( range.size() );
    el -= el % tile_size;

    assert( el > 0 );

    using I = detail::TilingIterator<typename Range::iterator>;

    return GenericRange<I>(
               I( range.begin(), range.begin() + tile_size ),
               I( range.begin() + el, range.end() )
           );
}

/**
 * @brief Split a range by given delimiter.
 *
 * @param range Range to be split.
 * @param delimiter Delimiter value by which to split the range.
 *
 * @return Range of sub-ranges split by given delimiter.
 */
template<class Range>
auto split( Range range, typename Range::value_type delimiter ) {
    using I = typename Range::iterator;
    return GenericRange<detail::SplitIterator<I> >(
               detail::SplitIterator<I>( range.begin(), range.end(), delimiter ),
               detail::SplitIterator<I>( range.end(), range.end(), delimiter )
           );
}

/**
 * @brief Range (sequence) of integral values.
 *
 * @param start Start of the sequence.
 * @param end End of sequence.
 * @param jump Stride value from one to the next value in sequence.
 *
 * @return Lazy range (sequence) of values.
 */
template<class I, class J = I>
auto iota( I start, I end, J jump = J( 1 ) ) {
    static_assert(std::is_integral<I>::value && std::is_integral<J>::value, 
            "Value types have to be integral.");
    return GenericRange<detail::IotaIterator<I, J> >(
               detail::IotaIterator<I, J>( start, jump ),
               detail::IotaIterator<I, J>( end, jump )
           );
}

//! Integral sequence from 0 to given number, by jump of 1.
template<class I>
auto iota( I count ) {
    return ::iota(0, count, 1);
}

/**
 * @brief Apply function mapping to the data range.
 *
 * Create lazy mapping range. Function is applied to
 * range elements lazily as they are accesed, generating
 * a resulting range.
 *
 * @param fn Function.
 * @param range Data range.
 *
 * @param Lazy mapping range.
 */
template<class Range, class Fn>
auto map( Fn fn, Range range ) {
    using I = typename Range::iterator;
    using Mi = detail::MapIterator<I, Fn>;

    return GenericRange<Mi>(
               Mi( range.begin(), fn ),
               Mi( range.end(), fn )
           );
}

/**
 * @brief Apply data reduction with given function.
 *
 * @param fn Reduction function.
 * @param range Range to reduce.
 *
 * @return Reduction value.
 */
template<class Fn, class Range>
auto reduce( Fn fn, Range range ) {
    assert(range.size() > 0);
    auto r = ::range( range );
    typename Range::value_type acc = *r.begin();
    for ( auto const &i : range.drop() ) {
        acc = fn( acc, i );
    }
    return acc;
}

/**
 * @brief Apply data folding with given function, and given initial accumulator value.
 *
 * @param fn Reduction function.
 * @param init Initial accumulator value.
 * @param range Range to reduce.
 *
 * @return Reduction value.
 */
template<class Fn, class Range>
auto fold( Fn fn, typename Range::value_type acc, Range range ) {
    for ( auto const &i : range ) {
        acc = fn( acc, i );
    }
    return acc;
}

/**
 * @brief Filter range by given criteria.
 *
 * Evaluate criteria function on each element of the
 * range. If element is to be further evaluated, result
 * of criteria function has to be true.
 *
 * @param fn Criteria function.
 * @param range Range to be filtered.
 *
 * @return Lazy filtering range.
 */
template<class Range, class Fn>
auto filter( Fn fn, Range range ) {
    using I = typename Range::iterator;
    using Mi = detail::FilterIterator<I, Fn>;

    return GenericRange<Mi>(
               Mi( range.begin(), range.end(), fn ),
               Mi( range.end(), range.end(), fn )
           );
}

/**
 * @brief Take first few elements from a range.
 *
 * @param range Range from which to take elements.
 * @param n Number of elements to take.
 *
 * @return Sub-range with taken elements.
 */
template<class Range>
auto take( Range &range, size_t n)
{
    assert(n > 0);
    using I = typename Range::iterator;
    auto b = range.begin();
    auto e = b + n;
    assert( b < e );
    return GenericRange<I>( b, e );
}

//! Drop first few elements from a range.
template<class Range>
auto drop( Range &range, size_t n = 1 ) {
    using I = typename Range::iterator;
    auto b = range.begin() + n;
    auto e = range.end();
    assert( b < e );
    return GenericRange<I>( b, e );
}

//! Take the tail of a range.
template<class Range>
auto tail( Range &range, size_t n ) {
    using I = typename Range::iterator;
    auto b = range.end() - n;
    auto e = range.end();
    assert( b < e );
    assert( b >= range.begin() );
    return GenericRange<I>( b, e );
}

/**
 * @brief Evaluate given unary function on each element of the array.
 */
template<class Range, class Fn>
std::enable_if_t<is_generic_range<Range>::value, void>
each( Fn fn, Range range ) {
    // special case for generic ranges that follow copy-on-pass semantics.
    for ( auto e : range ) {
        fn( e );
    }
}

template<class Range, class Fn>
std::enable_if_t<!is_generic_range<Range>::value, void>
each( Fn fn, Range& range ) {
    // non-const special case for other types of ranges. c++ ranges (e.g. vector) should be passed by reference.
    for ( auto &e : range ) {
        fn( e );
    }
}

template<class Range, class Fn>
std::enable_if_t<!is_generic_range<Range>::value, void>
each( Fn fn, const Range& range ) {
    // read-onnly (const) case for stl ranges.
    for ( const auto &e : range ) {
        fn( e );
    }
}

//! Parse string by line. Creates a range of lines.
template<class String>
auto byLine( String &string ) {
    using I = detail::SplitIterator<typename String::iterator>;
    return GenericRange<I>(
               I( string.begin(), string.end(), '\n' ),
               I( string.end(), string.end(), '\n' )
           );
}

/////////////////////////////////////////////////////////
// Implementation of class items
/////////////////////////////////////////////////////////

template<class I>
GenericRange<I> detail::TilingIterator<I>::operator*() {
    return GenericRange<I>( iter, end );
}
template<class I>
template<class Fn>
void GenericRange<I>::each( Fn fn ) {
    ::each( fn, *this );
}

template<class I>
template<class Fn>
GenericRange<detail::MapIterator<I, Fn> >
GenericRange<I>::map( Fn fn ) {
    return ::map( fn, *this );
}

template<class I>
template<class T>
GenericRange<detail::MapIterator<I, T( * )( typename GenericRange<I>::value_type )> >
GenericRange<I>::as( ) {
    return ::map( detail::cast<T,typename GenericRange<I>::value_type>, *this );
}

template<class I>
template<class Fn>
GenericRange<detail::FilterIterator<I, Fn> >
GenericRange<I>::filter( Fn fn ) {
    return ::filter( fn, *this );
}

template<class I>
template<class Fn>
typename GenericRange<I>::value_type
GenericRange<I>::reduce( Fn fn ) {
    return ::reduce( fn, *this );
}

template<class I>
template<class Fn>
typename GenericRange<I>::value_type
GenericRange<I>::fold( Fn fn, typename GenericRange<I>::value_type init ) {
    return ::fold( fn, init, *this );
}

template<class I>
GenericRange<I> GenericRange<I>::take( size_t n ) {
    return ::take( *this, n );
}

template<class I>
GenericRange<I> GenericRange<I>::drop( size_t n ) {
    return ::drop( *this, n );
}

template<class I>
GenericRange<I> GenericRange<I>::tail( size_t n ) {
    return ::tail( *this, n );
}

template<class I>
template<class O>
void GenericRange<I>::copyTo( GenericRange<O> other ) const {
    assert( this->size() == other.size() );
    auto ti = _b;
    auto oi = other.begin();

    while( ti != _e ) {
        *( oi++ ) = *( ti++ );
    }
}

template<class I>
template<class Range>
void GenericRange<I>::copyTo( Range &other ) const {
    assert( this->size() == other.size() );
    auto ti = _b;
    auto oi = other.begin();

    while( ti != _e ) {
        *( oi++ ) = *( ti++ );
    }
}

template<class I>
auto GenericRange<I>::tile( size_t tile_length ) {
    return ::tile( *this, tile_length );
}

template<class I>
GenericRange<detail::SplitIterator<I> >
GenericRange<I>::split( typename GenericRange<I>::value_type delimiter ) {
    return ::split( *this, delimiter );
}

#undef ITERATOR_WRAPPER_COMPARISON_IMPL
#undef RANDOM_ACCESS_ITERATOR_WRAPPER_IMPL

#endif /* end of include guard: RANGE_HPP_S0FY4EJW */

