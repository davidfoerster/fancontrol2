#pragma once
#ifndef UTIL_STATIC_ALLOCATOR_HPP_
#define UTIL_STATIC_ALLOCATOR_HPP_

#include "../in_range.hpp"
#include <memory>
#include <algorithm>
#include <cstddef>
#include <boost/utility/enable_if.hpp>
#include <boost/static_assert.hpp>
#include <boost/integer/static_min_max.hpp>
#include <boost/assert.hpp>


#ifndef STATIC_ALLOCATOR_CAPACITY_MAX
#   define STATIC_ALLOCATOR_CAPACITY_MAX (20UL << 10)
#endif


/* TODO:
 *  - Implement specialization for ::std::vector<T, static_allocator<T, Size, Extent>>
 *    that combines the statically and the dynamically allocated memory regions.
 */


namespace util {

    namespace helper {

        namespace detail {

            struct static_divide_base { typedef ::std::size_t value_type; };

        }


        template< detail::static_divide_base::value_type Numerator,
            detail::static_divide_base::value_type Denominator
        > struct static_divide_ceil : detail::static_divide_base {
            static const value_type value = (Numerator != 0) ? (Numerator - 1) / Denominator + 1 : 0;
        };


        template <typename Integer>
        inline Integer divide_ceil( Integer numerator, Integer denominator )
        {
            return (numerator != 0) ? (numerator -1) / denominator + 1 : 0;
        }

    }


    namespace detail {

        template <typename T, ::std::size_t Size>
        class static_container_base
        {
            BOOST_STATIC_ASSERT( Size > 0 );

        public:
            typedef char memory_type;
            typedef ::std::size_t size_type;
            typedef ::std::ptrdiff_t difference_type;

            static const size_type capacity = Size;

            inline size_type size() const { return m_size; }
            inline void size( size_type n ) { m_size = n; BOOST_ASSERT( n <= max_size() ); }

            inline size_type free() const { return max_size() - size(); }

            inline size_type max_size() const { return capacity; }

            //inline T &operator[](size_type i) { return data()[i]; }
            //inline const T &operator[](size_type i) const { return data()[i]; }

            inline T *data() { return reinterpret_cast<T*>(m_data); }
            inline const T *data() const { return reinterpret_cast<const T*>(m_data); }

            inline T *data_end() { return data() + max_size(); }
            inline const T *data_end() const { return data() + max_size(); }

            inline bool in_range( const void *p,
                difference_type offset_begin = 0, difference_type offset_end = 0
            ) const {
                return util::in_range(
                        reinterpret_cast< ::uintptr_t>(p),
                        reinterpret_cast< ::uintptr_t>(data()) + offset_begin,
                        reinterpret_cast< ::uintptr_t>(data_end()) + offset_end );
            }

            inline static_container_base() : m_size(0) { }

            template <size_type SizeU>
            inline explicit static_container_base( const static_container_base<T, SizeU> &other ){
                assign( other );
            }

            template <size_type SizeU>
            static_container_base<T, Size> &operator=( const static_container_base<T, SizeU> &other ){
                if( &other != this )
                    assign( other );
                return *this;
            }

        private:
            template <size_type SizeU>
            void assign( const static_container_base<T, SizeU> &other,
                typename ::boost::enable_if_c< Size >= SizeU, const void*>::type unused = NULL
            ){
                const size_type size = other.size();
                this->m_size = size;

                const T *const data = other.data();
                ::std::copy( reinterpret_cast<memory_type*>(data), reinterpret_cast<memory_type*>(data + size), m_data );
            }

            size_type m_size;

            memory_type m_data[ helper::static_divide_ceil<capacity * sizeof(T), sizeof(memory_type)>::value  ];
        };


        template <typename T>
        class static_container_base<T, 0>
        {
        public:
            typedef char memory_t;
            typedef ::std::size_t size_type;
            typedef ::std::ptrdiff_t difference_type;

            static const size_type capacity = 0;

            inline size_type size() const { return 0; }
            inline void size( size_type n ) { BOOST_ASSERT( n <= max_size() ); }

            inline size_type free() const { return max_size() - size(); }

            inline size_type max_size() const { return capacity; }

            inline T *data() const { return NULL; }

            inline T *data_end() const { return data() + size(); }

            inline bool in_range( const void *p,
                difference_type offset_begin = 0, difference_type offset_end = 0
            ) const {
                // offsets are ignored on purpose
                return p == data();
            }
        };


        template <typename T, ::std::size_t Size>
        class static_container
            : public static_container_base<T, Size>
        {
        private:
            typedef static_container_base<T, Size> base_t;

        public:
            typedef typename base_t::memory_type memory_type;
            typedef typename base_t::size_type size_type;
            typedef typename base_t::difference_type difference_type;

        protected:
            bool is_inside_static_container( const void *p ) const {
                const bool b = this->in_range( p, 0, 2 );
                if( b ){
                    BOOST_ASSERT_MSG( p < this->data_end(),
                        "Trying to access an unallocated region" );
                    BOOST_ASSERT_MSG( (reinterpret_cast< ::uintptr_t>(p) - reinterpret_cast< ::uintptr_t>(this->data())) % sizeof(T) == 0,
                        "Object access at unaligned address" );
                }
                return b;
            }

            bool is_inside_static_container( const void *p, size_type n ) {
                const bool b = this->in_range(p, 0, 2);
                if( b ){
                    BOOST_ASSERT_MSG( p == this->data() || (p == this->data_end() && n == 0),
                        "Trying to deallocate an unallocated memory region" );
                    BOOST_ASSERT_MSG( p != this->data() || n == this->size(),
                        "Trying to deallocate with an unexpected size" );
                }
                return b;
            }

        };

    }


    template <
        typename T,
        ::std::size_t Size,
        class Extent = ::std::allocator<T>
    > class static_allocator
        : private Extent
        , protected detail::static_container<T, ::boost::static_unsigned_min<Size, STATIC_ALLOCATOR_CAPACITY_MAX/sizeof(T)>::value>
    {
    public:
        typedef T value_type;
        typedef T* pointer;
        typedef const T* const_pointer;
        typedef T& reference;
        typedef const T& const_reference;

        typedef Extent extent_allocator_type;
        typedef typename extent_allocator_type::size_type size_type;
        typedef typename extent_allocator_type::difference_type difference_type;

    protected:
        typedef static_allocator<value_type, Size, extent_allocator_type> self_t;
        typedef detail::static_container<value_type, ::boost::static_unsigned_min<Size, STATIC_ALLOCATOR_CAPACITY_MAX/sizeof(value_type)>::value> static_allocator_type;

    public:
        static const size_type initial_capacity = static_allocator_type::capacity;


        inline explicit static_allocator( const Extent &extent = Extent() )
            : Extent( extent )
        { }

        inline static_allocator( const static_allocator<T, Size, Extent> &other )
            : Extent( other )
            , static_allocator_type( other )
        { }

        template <typename U, size_type SizeU, class ExtentU>
        inline explicit static_allocator(
                const static_allocator<U, SizeU, ExtentU> &other,
                const ExtentU &extent = ExtentU() )
            : Extent( extent )
        { }

        size_type max_size() const {
            return ::std::max( static_allocator_type::max_size(), Extent::max_size() );
        }


        inline T *address( T &ref ) const { return &ref; }
        inline const T *address( const T &ref ) const { return &ref; }

        T *allocate( size_type n, const void *hint = 0 ) {
            if( n != 0 ){
                if( (static_allocator_type::size() == 0 && n <= static_allocator_type::free()) ||
                    (hint == static_allocator_type::data() && n <= static_allocator_type::max_size())
                ){
                    static_allocator_type::size( n );
                    return static_allocator_type::data();
                } else {
                    return Extent::allocate( n, hint );
                }
            } else {
                return static_allocator_type::data_end();
            }
        }

        void deallocate( T *p, size_type n ) {
            if( is_inside_static_container(p, n) ){
                static_allocator_type::size( 0 );
            } else {
                Extent::deallocate( p, n );
            }
        }

        void construct( T *p, const T &val ) {
            if( is_inside_static_container(p) ){
                new (p) T(val);
            } else {
                Extent::construct( p, val );
            }
        }

        void destroy( T* p ) {
            if( is_inside_static_container(p) ){
                p->~T();
            } else {
                Extent::destroy( p );
            }
        }

        inline bool operator==( const static_allocator<T, Size, Extent> &other ) const {
            return &other == this;
        }

        inline bool operator!=( const static_allocator<T, Size, Extent> &other ) const {
            return !this->operator==( other );
        }

        template <typename U>
        struct rebind {
            typedef static_allocator< U, Size,
                    typename Extent::template rebind<U>::other
                > other;
        };

    };


    template <typename T, ::std::size_t Size>
    class static_allocator<T, Size, void>
        : protected detail::static_container<T, Size>
    {
    public:
        typedef T value_type;
        typedef T* pointer;
        typedef const T* const_pointer;
        typedef T& reference;
        typedef const T& const_reference;

        typedef void extent_allocator_type;

    protected:
        typedef static_allocator<value_type, Size, extent_allocator_type> self_t;
        typedef detail::static_container<value_type, Size> static_allocator_type;

    private:
        BOOST_STATIC_ASSERT( STATIC_ALLOCATOR_CAPACITY_MAX/sizeof(value_type) >= Size );

    public:
        typedef typename static_allocator_type::size_type size_type;
        typedef typename static_allocator_type::difference_type difference_type;

        static const size_type initial_capacity = static_allocator_type::capacity;


        inline static_allocator() { }

        inline static_allocator( const static_allocator<T, Size, void> &other )
            : static_allocator_type( other )
        { }

        template <typename U, size_type SizeU, class ExtentU>
        inline explicit static_allocator( const static_allocator<U, SizeU, ExtentU> &other ) { }

        template <typename U, size_type SizeU, class ExtentU>
        inline explicit static_allocator(
                const static_allocator<U, SizeU, ExtentU> &other,
                const ExtentU &extent )
        { }

        size_type max_size() const {
            return static_allocator_type::max_size();
        }


        inline T *address( T &ref ) const { return &ref; }
        inline const T *address( const T &ref ) const { return &ref; }

        T *allocate( size_type n, const void *hint = 0 ) {
            if( n != 0 ){
                if( (static_allocator_type::size() == 0 && n <= static_allocator_type::free()) ||
                    (hint == static_allocator_type::data() && n <= static_allocator_type::max_size())
                ){
                    static_allocator_type::size( n );
                    return static_allocator_type::data();
                } else {
                    throw ::std::bad_alloc("statically allocated memory exhausted");
                }
            } else {
                return static_allocator_type::data_end();
            }
        }

        void deallocate( T *p, size_type n ) {
            if( BOOST_VERIFY(is_inside_static_container(p, n)) ){
                static_allocator_type::size( 0 );
            }
        }

        void construct( T *p, const T &val ) {
            BOOST_ASSERT( is_inside_static_container(p) );
            new (p) T(val);
        }

        void destroy( T* p ) {
            BOOST_ASSERT( is_inside_static_container(p) );
            p->~T();
        }

        inline bool operator==( const static_allocator<T, Size, void> &other ) const {
            return &other == this;
        }

        inline bool operator!=( const static_allocator<T, Size, void> &other ) const {
            return !this->operator==( other );
        }

        template <typename U>
        struct rebind {
            typedef static_allocator<U, Size, void> other;
        };

    };


    template <class>
    struct is_static_allocator
        : ::boost::false_type
    { };

    template <typename T, ::std::size_t Size, class Extent>
    struct is_static_allocator< static_allocator<T, Size, Extent> >
        : ::boost::true_type
    { };


    template <class C>
    struct has_static_allocator
        : is_static_allocator<typename C::allocator_type>
    { };


	template <class C>
	inline
	typename ::boost::disable_if< has_static_allocator<C>, void>::type
	init( C &c )
	{ }


	template <class C>
	inline
	typename ::boost::enable_if< has_static_allocator<C>, void>::type
	init( C &c )
	{
		c.reserve(C::allocator_type::initial_capacity);
	}


    template <class C>
    inline
    typename ::boost::disable_if< has_static_allocator<C>, void>::type
    clear( C &c )
    {
        c.clear();
    }


    template <class C>
    inline
    typename ::boost::enable_if< has_static_allocator<C>, void>::type
    clear( C &c )
    {
        c.clear();
        if( c.capacity() > C::allocator_type::initial_capacity ){
            C().swap(c);
        } else {
            BOOST_ASSERT( c.capacity() == 0 || in_range(&c.front(), &c, &c+1) );
        }
    }

}

#endif // UTIL_STATIC_ALLOCATOR_HPP_
