#pragma once
#ifndef CSENSORS_HPP_
#define CSENSORS_HPP_

#include <cstdio>
#include <cstddef>


namespace sensors {

#include <sensors/sensors.h>


	extern const char *const default_config_path;


	bool operator==(const sensors_bus_id &a, const sensors_bus_id &b);

	::std::size_t hash_value(const sensors_bus_id &);


	bool operator==(const sensors_chip_name &a, const sensors_chip_name &b);

	::std::size_t hash_value(const sensors_chip_name &);


	namespace helper {

	    bool equals(const char *a, const char *b);

	}



// implementations ========================================

	namespace helper {

	    inline bool equals(const sensors_bus_id &a, const sensors_bus_id &b)
	    {
		    return a.nr == b.nr;
	    }


	    inline bool equals(const sensors_chip_name &a, const sensors_chip_name &b)
	    {
		    return a.addr == b.addr && a.bus == b.bus;
	    }

	} // namespace helper


#ifdef NDEBUG

	inline bool operator==(const sensors_bus_id &a, const sensors_bus_id &b)
	{
	    return helper::equals(a, b);
	}


	inline bool operator==(const sensors_chip_name &a, const sensors_chip_name &b)
	{
	    return helper::equals(a, b);
	}

#endif

} // namespace sensors


#endif
