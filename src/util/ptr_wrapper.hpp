#ifndef UTIL_PTR_WRAPPER_HPP_
#define UTIL_PTR_WRAPPER_HPP_

#include <boost/functional/hash.hpp>


namespace util {

	namespace detail {

template <typename T>
struct ptr_traits {
	typedef typename T::element_type element_type;
	static constexpr bool is_raw_ptr = false;
};

template <typename T>
struct ptr_traits<T*> {
	typedef T element_type;
	static constexpr bool is_raw_ptr = true;
};


template <class W>
class ptr_wrapper_base
{
public:
	typedef typename detail::ptr_traits<W>::element_type element_type;

	ptr_wrapper_base();

	ptr_wrapper_base( const W &ptr );

	ptr_wrapper_base<W> &operator=( const W &ptr );

	inline element_type *operator&() { return &*m_ptr; }
	inline const element_type *operator&() const { return &*m_ptr; }

	inline W &ptr() { return m_ptr; }
	inline const W &ptr() const { return m_ptr; }

	inline element_type &ref() { return *m_ptr; }
	inline const element_type &ref() const { return *m_ptr; }

	inline operator element_type&() { return ref(); }
	inline operator const element_type&() const { return ref(); }

	bool operator==( const element_type &o ) const;

	bool operator<( const element_type &o ) const;

protected:
	W m_ptr;
};

} // namespace detail


template <class W>
class ptr_wrapper
	: public detail::ptr_wrapper_base<W>
{
public:
	typedef typename detail::ptr_wrapper_base<W>::element_type element_type;

	ptr_wrapper();

	ptr_wrapper( element_type* p );

	ptr_wrapper( const W &ptr );

	ptr_wrapper<W> &operator=( element_type *p );

	inline operator W&() { return this->ptr(); };
	inline operator const W&() const { return this->ptr(); };
};


template <class W>
class ptr_wrapper<W*>
	: public detail::ptr_wrapper_base<W*>
{
public:
	typedef typename detail::ptr_wrapper_base<W*>::element_type element_type;

	ptr_wrapper();

	ptr_wrapper( W *p );
};


template <class W>
std::size_t hash_value( const ptr_wrapper<W> &k );



// implementations ============================================================

template <class W>
inline ptr_wrapper<W>::ptr_wrapper()
	: detail::ptr_wrapper_base<W>()
{ }


template <class W>
inline ptr_wrapper<W>::ptr_wrapper( element_type* p )
	: detail::ptr_wrapper_base<W>(W(p))
{ }


template <class W>
inline ptr_wrapper<W>::ptr_wrapper( const W &ptr )
	: detail::ptr_wrapper_base<W>(ptr)
{ }


template <class W>
inline ptr_wrapper<W> &ptr_wrapper<W>::operator=( element_type *p )
{
	this->m_ptr = p;
	return *this;
}


template <class W>
inline ptr_wrapper<W*>::ptr_wrapper()
	: detail::ptr_wrapper_base<W*>()
{ }


template <class W>
inline ptr_wrapper<W*>::ptr_wrapper( W *p )
	: detail::ptr_wrapper_base<W*>(p)
{ }


template <class W>
inline std::size_t hash_value( const ptr_wrapper<W> &k )
{
	return boost::hash<typename ptr_wrapper<W>::element_type>()(k.ref());
}


namespace detail {

template <class W>
inline ptr_wrapper_base<W>::ptr_wrapper_base()
	: m_ptr()
{ }


template <class W>
inline ptr_wrapper_base<W>::ptr_wrapper_base( const W &ptr )
	: m_ptr(ptr)
{ }


template <class W>
inline ptr_wrapper_base<W> &ptr_wrapper_base<W>::operator=( const W &ptr )
{
	m_ptr = ptr;
	return *this;
}


template <class W>
inline bool ptr_wrapper_base<W>::operator==( const element_type &o ) const
{
	return ref() == o;
}


template <class W>
inline bool ptr_wrapper_base<W>::operator<( const element_type &o ) const
{
	return ref() < o;
}

} // namespace detail

} // namespace util
#endif // UTIL_PTR_WRAPPER_HPP_
