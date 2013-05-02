/*
 * memory.hpp
 *
 *  Created on: 02.05.2013
 *      Author: malte
 */

#pragma once
#ifndef MEMORY_HPP_
#define MEMORY_HPP_

#include <memory>


namespace util {

#if defined(BOOST_DISABLE_THREADS) || defined(BOOST_SP_DISABLE_THREADS)

	template <typename T>
	using shared_ptr = std::__shared_ptr<T, __gnu_cxx::_S_single>;

	template <typename T>
	using weak_ptr = std::__weak_ptr<T, __gnu_cxx::_S_single>;

	template <typename T>
	using enable_shared_from_this = std::__enable_shared_from_this<T, __gnu_cxx::_S_single>;

	template <typename T, typename... Args>
	inline shared_ptr<T> make_shared( Args&&... args )
	{
		return std::__make_shared<T, __gnu_cxx::_S_single>(std::forward<Args>(args)...);
	}

#else

	using std::shared_ptr;
	using std::weak_ptr;
	using std::enable_shared_from_this;
	using std::make_shared;

#endif

} // namespace util
#endif /* MEMORY_HPP_ */
