#include "csensors.hpp"
#include <boost/functional/hash.hpp>
#include <boost/assert.hpp>
#include <cstring>


namespace sensors {

	using std::size_t;


	const char *const default_config_path = "/etc/sensors3.conf";


	namespace helper {

		bool equals(const char *a, const char *b)
		{
			return a == b || (a && b && std::strcmp(a, b) == 0);
		}

	}


	size_t hash_value(const sensors_bus_id &k)
	{
		size_t seed = 0;
		boost::hash_combine(seed, k.type);
		boost::hash_combine(seed, k.nr);
		return seed;
	}


	size_t hash_value(const sensors_chip_name &k)
	{
		size_t seed = 0;
		boost::hash_combine(seed, k.bus);
		boost::hash_combine(seed, k.addr);
		return seed;
	}


#ifndef NDEBUG

	bool operator==(const sensors_bus_id &a, const sensors_bus_id &b)
	{
		const bool result = helper::equals(a, b);
		if (result) {
			BOOST_ASSERT( a.type == b.type );
		}
		return result;
	}


	bool operator==(const sensors_chip_name &a, const sensors_chip_name &b)
	{
		const bool result = helper::equals(a, b);
		if (result) {
			BOOST_ASSERT(helper::equals(a.prefix, b.prefix));
			BOOST_ASSERT(helper::equals(a.path, b.path));
		}
		return result;
	}

#endif

}
