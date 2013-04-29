#pragma once
#ifndef UTIL_STATIC_ALLOCATOR_HPP_
#define UTIL_STATIC_ALLOCATOR_HPP_

#include "../in_range.hpp"
#include <memory>
#include <algorithm>
#include <utility>
#include <cstddef>
#include <boost/integer/static_min_max.hpp>
#include <boost/assert.hpp>


#ifndef STATIC_ALLOCATOR_CAPACITY_MAX
	#ifdef PAGE_SIZE
		#define STATIC_ALLOCATOR_CAPACITY_MAX (PAGE_SIZE / 2)
	#else
		#define STATIC_ALLOCATOR_CAPACITY_MAX (2UL << 10)
	#endif
#endif


/* TODO:
 *  - Implement specialization for std::vector<T, static_allocator<T, Size, Extent>>
 *    that combines the statically and the dynamically allocated memory regions.
 */
namespace util {

	namespace helper {

		template <typename Integer>
		constexpr Integer divide_ceil( Integer numerator, Integer denominator );

	} // namespace helper


	namespace detail {

		template <typename T, std::size_t Size>
		class static_container_base
		{
		public:
			typedef char memory_type;
			typedef std::size_t size_type;
			typedef std::ptrdiff_t difference_type;

			static const size_type capacity = Size;

			size_type size() const;
			void size( size_type n );

			size_type free() const;

			size_type max_size() const;

			T *data();
			const T *data() const;

			T *data_end();
			const T *data_end() const;

			bool in_range( const void *p, difference_type offset_begin = 0,
					difference_type offset_end = 0 ) const;

			static_container_base();

			template < std::size_t SizeU>
			explicit static_container_base(
					const static_container_base<T, SizeU> &other );

			template < std::size_t SizeU>
			static_container_base<T, Size> &operator=(
					const static_container_base<T, SizeU> &other );

		private:
			template < std::size_t SizeU>
			void assign( const static_container_base<T, SizeU> &other,
					typename std::enable_if< Size >= SizeU,
					const void*>::type unused = nullptr );

			size_type m_size;

			memory_type m_data[ helper::divide_ceil(
					capacity * sizeof(T), sizeof(memory_type) ) ];
		};


		template <typename T>
		class static_container_base<T, 0>
		{
		public:
			typedef char memory_t;
			typedef std::size_t size_type;
			typedef std::ptrdiff_t difference_type;

			static const size_type capacity = 0;

			size_type size() const;
			void size( size_type n );

			size_type free() const;

			size_type max_size() const;

			T *data() const;

			T *data_end() const;

			bool in_range( const void *p, difference_type offset_begin = 0,
					difference_type offset_end = 0 ) const;

			void assign( const static_container_base<T, 0> &other );
		};


		template <typename T, std::size_t Size>
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
			bool is_inside_static_container( const void *p ) const;

			bool is_inside_static_container( const void *p, size_type n ) const;

		};

	} // namespace detail


	template <
		typename T,
		std::size_t Size,
		class Extent = std::allocator<T>
	> class static_allocator
		: protected detail::static_container<T, boost::static_unsigned_min<Size, STATIC_ALLOCATOR_CAPACITY_MAX/sizeof(T)>::value>
		, private Extent
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
		typedef detail::static_container<value_type, boost::static_unsigned_min<Size, STATIC_ALLOCATOR_CAPACITY_MAX/sizeof(value_type)>::value> static_container_type;

	public:
		static const size_type initial_capacity = static_container_type::capacity;


		explicit static_allocator( const Extent &extent = Extent() );

		static_allocator( const static_allocator<T, Size, Extent> &other );

		template <typename U, std::size_t SizeU, class ExtentU>
		explicit static_allocator(
				const static_allocator<U, SizeU, ExtentU> &other,
				const ExtentU &extent = ExtentU() );

		size_type max_size() const;


		T *address( T &ref ) const;
		const T *address( const T &ref ) const;

		T *allocate( size_type n, const void *hint = 0 );

		void deallocate( T *p, size_type n );

		template <class U, class... Args>
		void construct( U *p, Args&&... args );

		void destroy( T* p );

		bool operator==( const static_allocator<T, Size, Extent> &other ) const;

		bool operator!=( const static_allocator<T, Size, Extent> &other ) const;

		template <typename U>
		struct rebind {
			typedef static_allocator< U, Size,
					typename Extent::template rebind<U>::other
				> other;
		};

	};


	template <typename T, std::size_t Size>
	class static_allocator<T, Size, void>
		: protected detail::static_container<T, Size>
	{
		static_assert( Size <= STATIC_ALLOCATOR_CAPACITY_MAX / sizeof(T),
				"Static allocator exceeds pre-defined maximum size; use a dynamic allocator instead." );

	public:
		typedef T value_type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;

		typedef void extent_allocator_type;

	protected:
		typedef static_allocator<value_type, Size, extent_allocator_type> self_t;
		typedef detail::static_container<value_type, Size> static_container_type;

	public:
		typedef typename static_container_type::size_type size_type;
		typedef typename static_container_type::difference_type difference_type;

		static const size_type initial_capacity = static_container_type::capacity;


		static_allocator();

		static_allocator( const static_allocator<T, Size, void> &other );

		template <typename U, std::size_t SizeU, class ExtentU>
		explicit static_allocator( const static_allocator<U, SizeU, ExtentU> &other );

		template <typename U, std::size_t SizeU, class ExtentU>
		explicit static_allocator(
				const static_allocator<U, SizeU, ExtentU> &other,
				const ExtentU &extent );

		size_type max_size() const;


		T *address( T &ref ) const;
		const T *address( const T &ref ) const;

		T *allocate( size_type n, const void *hint = 0 );

		void deallocate( T *p, size_type n );

		template <class U, class... Args>
		void construct( U *p, Args&&... args );

		void destroy( T* p );

		bool operator==( const static_allocator<T, Size, void> &other ) const;

		bool operator!=( const static_allocator<T, Size, void> &other ) const;

		template <typename U>
		struct rebind {
			typedef static_allocator<U, Size, void> other;
		};

	};


	template <class ContainerT>
	class statically_allocated_container_wrapper
		: public ContainerT
	{
	public:
		typedef typename ContainerT::allocator_type allocator_type;
		typedef typename ContainerT::value_type value_type;
		typedef typename ContainerT::size_type size_type;

		statically_allocated_container_wrapper();

		explicit statically_allocated_container_wrapper( size_type count,
				const value_type &value = value_type() );

		template <class InputIterator>
		statically_allocated_container_wrapper( InputIterator first, InputIterator last );

		statically_allocated_container_wrapper( const ContainerT &other );

		bool outgrown() const;

		void clear();
	};


	template <class>
	struct is_static_allocator
		: std::false_type
	{ };

	template <typename T, std::size_t Size, class Extent>
	struct is_static_allocator< static_allocator<T, Size, Extent> >
		: std::true_type
	{ };


	template <class C>
	struct has_static_allocator
		: is_static_allocator<typename C::allocator_type>
	{ };





// implementation =============================================================

	namespace helper {

		template <typename Integer>
		inline constexpr Integer divide_ceil( Integer numerator, Integer denominator )
		{
			return (numerator != 0) ? (numerator - 1) / denominator + 1 : 0;
		}

	} // namespace helper


	namespace detail {

		template <typename T, std::size_t S>
		inline
		typename static_container_base<T,S>::size_type static_container_base<T,S>::size() const
		{
			return m_size;
		}


		template <typename T, std::size_t S>
		inline
		void static_container_base<T,S>::size( size_type n )
		{
			m_size = n;
			BOOST_ASSERT( n <= max_size() );
		}


		template <typename T, std::size_t S>
		inline
		typename static_container_base<T,S>::size_type static_container_base<T,S>::free() const
		{
			return max_size() - size();
		}


		template <typename T, std::size_t S>
		inline
		typename static_container_base<T,S>::size_type static_container_base<T,S>::max_size() const
		{
			return capacity;
		}


		template <typename T, std::size_t S>
		inline
		T *static_container_base<T,S>::data()
		{
			return reinterpret_cast<T*>(m_data);
		}

		template <typename T, std::size_t S>
		inline
		const T *static_container_base<T,S>::data() const
		{
			return reinterpret_cast<const T*>(m_data);
		}


		template <typename T, std::size_t S>
		inline
		T *static_container_base<T,S>::data_end()
		{
			return data() + max_size();
		}

		template <typename T, std::size_t S>
		inline
		const T *static_container_base<T,S>::data_end() const
		{
			return data() + max_size();
		}


		template <typename T, std::size_t S>
		bool static_container_base<T,S>::in_range( const void *p,
			difference_type offset_begin, difference_type offset_end
		) const {
			return util::in_range(
					reinterpret_cast< ::uintptr_t>(p),
					reinterpret_cast< ::uintptr_t>(data()) + offset_begin,
					reinterpret_cast< ::uintptr_t>(data_end()) + offset_end );
		}


		template <typename T, std::size_t S>
		inline static_container_base<T,S>::static_container_base()
			: m_size(0)
		{ }


		template <typename T, std::size_t S>
		template < std::size_t SizeU>
		inline static_container_base<T,S>::static_container_base(
				const static_container_base<T, SizeU> &other
		) {
			assign( other );
		}


		template <typename T, std::size_t S>
		template < std::size_t SizeU>
		static_container_base<T,S> &static_container_base<T,S>::operator=(
				const static_container_base<T, SizeU> &other
		) {
			if( &other != this )
				assign( other );
			return *this;
		}


		template <typename T, std::size_t Size>
		template < std::size_t SizeU>
		void static_container_base<T, Size>::assign(
			const static_container_base<T, SizeU> &other,
			typename std::enable_if< Size >= SizeU, const void*>::type
		){
			T *out = this->data();
			const T *in = other.data();

			for( const T *const last = this->data() + std::min(this->size(), other.size()); out < last; ++out, ++in )
				*out = *in;

			for( const T *const last = this->data() + other.size(); out < last; ++out, ++in )
				new (out) T(*in);

			for( const T *const last = this->data() + this->size(); out < last; ++out )
				(out++)->~T();

			this->size( other.size() );
		}


		template <typename T>
		inline
		typename static_container_base<T,0>::size_type static_container_base<T,0>::size() const
		{
			return 0;
		}


		template <typename T>
		inline
		void static_container_base<T,0>::size( size_type n )
		{
			BOOST_ASSERT( n <= max_size() );
		}


		template <typename T>
		inline
		T *static_container_base<T,0>::data() const
		{
			return nullptr;
		}


		template <typename T>
		inline
		bool static_container_base<T,0>::in_range(
			const void *p, difference_type offset_begin, difference_type offset_end
		) const {
			// offsets are ignored on purpose
			return p == data();
		}


		template <typename T>
		inline
		void static_container_base<T,0>::assign( const static_container_base<T, 0> &other )
		{ }


		template <typename T, std::size_t S>
		bool static_container<T,S>::is_inside_static_container( const void *p ) const
		{
			const bool b = this->in_range( p, 0, 2 );
			if( b ){
				BOOST_ASSERT_MSG( p < this->data_end(),
					"Trying to access an unallocated region" );
				BOOST_ASSERT_MSG( (reinterpret_cast< ::uintptr_t>(p) - reinterpret_cast< ::uintptr_t>(this->data())) % sizeof(T) == 0,
					"Object access at unaligned address" );
			}
			return b;
		}


		template <typename T, std::size_t S>
		bool static_container<T,S>::is_inside_static_container( const void *p, size_type n __attribute__((unused))) const
		{
			const bool b = this->in_range(p, 0, 2);
			if( b ){
				BOOST_ASSERT_MSG( p == this->data() || (p == this->data_end() && n == 0),
					"Trying to deallocate an unallocated memory region" );
				BOOST_ASSERT_MSG( p != this->data() || n == this->size(),
					"Trying to deallocate with an unexpected size" );
			}
			return b;
		}

	} // namespace detail


	template <typename T, std::size_t Size, class Extent>
	inline
	static_allocator<T, Size, Extent>::static_allocator( const Extent &extent )
		: Extent( extent )
	{ }


	template <typename T, std::size_t Size, class Extent>
	inline
	static_allocator<T, Size, Extent>::static_allocator(
			const static_allocator<T, Size, Extent> &other
	)	: static_container_type( other )
		, Extent( other )
	{ }


	template <typename T, std::size_t Size , class Extent >
	template <typename U, std::size_t SizeU, class ExtentU>
	inline
	static_allocator<T, Size, Extent>::static_allocator(
			const static_allocator<U, SizeU, ExtentU> &other __attribute__((unused)),
			const ExtentU &extent )
		: Extent( extent )
	{ }


	template <typename T, std::size_t Size, class Extent>
	inline
	typename static_allocator<T, Size, Extent>::size_type
	static_allocator<T, Size, Extent>::max_size() const
	{
		return std::max( static_container_type::max_size(), Extent::max_size() );
	}


	template <typename T, std::size_t Size, class Extent>
	inline
	T *static_allocator<T, Size, Extent>::address( T &ref ) const
	{
		return &ref;
	}

	template <typename T, std::size_t Size, class Extent>
	inline
	const T *static_allocator<T, Size, Extent>::address( const T &ref ) const
	{
		return &ref;
	}


	template <typename T, std::size_t Size, class Extent>
	T *static_allocator<T, Size, Extent>::allocate( size_type n, const void *hint )
	{
		if( n != 0 ){
			if( hint == static_container_type::data() && n <= static_container_type::max_size() ){ // TODO
				static_container_type::size( n );
				return static_container_type::data();
			} else if( static_container_type::size() == 0 && n <= static_container_type::free() ){
				static_container_type::size( n );
				return static_container_type::data();
			} else {
				return Extent::allocate( n, hint );
			}
		} else {
			return static_container_type::data_end();
		}
	}


	template <typename T, std::size_t Size, class Extent>
	void
	static_allocator<T, Size, Extent>::deallocate( T *p, size_type n )
	{
		if( this->is_inside_static_container(p, n) ){
			static_container_type::size( 0 );
		} else {
			Extent::deallocate( p, n );
		}
	}


	template <typename T, std::size_t Size, class Extent>
	template <class U, class... Args>
	void
	static_allocator<T, Size, Extent>::construct( U *p, Args&&... args )
	{
		if( this->is_inside_static_container(p) ){
			new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
		} else {
			Extent::construct( p, std::forward<Args>(args)... );
		}
	}


	template <typename T, std::size_t Size, class Extent>
	void
	static_allocator<T, Size, Extent>::destroy( T* p )
	{
		if( this->is_inside_static_container(p) ){
			p->~T();
		} else {
			Extent::destroy( p );
		}
	}


	template <typename T, std::size_t Size, class Extent>
	inline
	bool static_allocator<T, Size, Extent>::operator==(
			const static_allocator<T, Size, Extent> &other
	) const {
		return this == &other;
	}

	template <typename T, std::size_t Size, class Extent>
	inline
	bool static_allocator<T, Size, Extent>::operator!=(
			const static_allocator<T, Size, Extent> &other
	) const {
		return !operator==( other );
	}


	template <typename T, std::size_t Size>
	inline
	static_allocator<T, Size, void>::static_allocator()
	{ }


	template <typename T, std::size_t Size>
	inline
	static_allocator<T, Size, void>::static_allocator(
			const static_allocator<T, Size, void> &other
	)	: static_container_type( other )
	{ }


	template <typename T, std::size_t Size>
	template <typename U, std::size_t SizeU, class ExtentU>
	inline
	static_allocator<T, Size, void>::static_allocator(
		const static_allocator<U, SizeU, ExtentU> &other )
	{ }


	template <typename T, std::size_t Size>
	template <typename U, std::size_t SizeU, class ExtentU>
	inline
	static_allocator<T, Size, void>::static_allocator(
			const static_allocator<U, SizeU, ExtentU> &other,
			const ExtentU &extent )
	{ }


	template <typename T, std::size_t Size>
	inline
	typename static_allocator<T, Size, void>::size_type
	static_allocator<T, Size, void>::max_size() const
	{
		return static_container_type::max_size();
	}


	template <typename T, std::size_t Size>
	T *static_allocator<T, Size, void>::allocate( size_type n, const void *hint )
	{
		if( n != 0 ){
			if( (static_container_type::size() == 0 && n <= static_container_type::free()) ||
				(hint == static_container_type::data() && n <= static_container_type::max_size())
			){
				static_container_type::size( n );
				return static_container_type::data();
			} else {
				throw std::bad_alloc("statically allocated memory exhausted");
			}
		} else {
			return static_container_type::data_end();
		}
	}


	template <typename T, std::size_t Size>
	void static_allocator<T, Size, void>::deallocate( T *p, size_type n )
	{
		if( BOOST_VERIFY(is_inside_static_container(p, n)) ){
			static_container_type::size( 0 );
		}
	}


	template <typename T, std::size_t Size>
	template <class U, class... Args>
	void
	static_allocator<T, Size, void>::construct( U *p, Args&&... args )
	{
		BOOST_ASSERT( is_inside_static_container(p) );
		new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
	}


	template <typename T, std::size_t Size>
	inline
	void static_allocator<T, Size, void>::destroy( T* p )
	{
		BOOST_ASSERT( is_inside_static_container(p) );
		p->~T();
	}


	template <class C>
	inline
	statically_allocated_container_wrapper<C>::statically_allocated_container_wrapper()
	{
		this->reserve( allocator_type::initial_capacity );
	}

	template <class C>
	statically_allocated_container_wrapper<C>::statically_allocated_container_wrapper(
			size_type count, const value_type &value )
	{
		if( count < allocator_type::initial_capacity )
			this->reserve( allocator_type::initial_capacity );
		this->assign( count, value );
	}


	template <class C>
	template <class InputIterator>
	statically_allocated_container_wrapper<C>::statically_allocated_container_wrapper(
			InputIterator first, InputIterator last )
	{
		this->reserve( allocator_type::initial_capacity );
		this->assign( first, last );
	}


	template <class C>
	statically_allocated_container_wrapper<C>::statically_allocated_container_wrapper(
			const C &other )
	{
		if( other.size() < allocator_type::initial_capacity )
			this->reserve( allocator_type::initial_capacity );
		this->operator=( other );
	}


	template <class C>
	inline
	bool statically_allocated_container_wrapper<C>::outgrown() const
	{
		return this->capacity() > allocator_type::initial_capacity;
	}


	template <class C>
	inline
	void statically_allocated_container_wrapper<C>::clear()
	{
		if( outgrown() ){
			C(std::move(*this));
		} else {
			C::clear();
		}
	}

} // namespace util
#endif // UTIL_STATIC_ALLOCATOR_HPP_
