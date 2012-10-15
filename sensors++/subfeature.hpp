/*
 * subfeature.hpp
 *
 *  Created on: 16.12.2011
 *      Author: malte
 */

#pragma once
#ifndef SENSORS_SUBFEATURE_HPP_
#define SENSORS_SUBFEATURE_HPP_

#include "internal/common.hpp"
#include "internal/object_wrapper.hpp"
#include "exceptions.hpp"
#include "../util/self_referenced.hpp"
#include "csensors.hpp"
#include "feature.hpp"


namespace sensors {

	using ::boost::shared_ptr;


	class subfeature
	    : virtual public ::util::self_referenced<subfeature>
	    , public object_wrapper_numbered<const sensors_subfeature, feature>
	{
	public:
	    typedef object_wrapper_numbered<const sensors_subfeature, feature> super;

	    typedef sensors_subfeature_type type_enum;

	    static type_enum type_from_name(sensors_feature_type feature, const string_ref &name);

	    struct flags {
		    enum value {
			    readable,
			    writable,
			    compute_mapping,
			    _size
		    };
	    };

	    typedef flags::value flags_enum;

	    template <class Tag>
	    subfeature(basic_type *subfeature, const shared_ptr<feature> &feature, Tag);

	    bool test_flag(unsigned int flag) const;

	    bool test_flag(flags_enum flag) const;

	    double value() const throw(sensor_error);

	    void value(double) const throw(::util::null_pointer_exception, sensor_error);

	    bool operator==(const super &o) const;
	};

} /* namespace sensors */


template <typename Char, class Traits>
::std::basic_ostream<Char, Traits> &operator<<(::std::basic_ostream<Char, Traits>&, const sensors::subfeature&);



// implementations ========================================

namespace sensors {

	template <class Tag>
	inline
	subfeature::subfeature(basic_type *subfeature, const shared_ptr<feature> &feature, Tag tag)
	    : selfreference_type(tag)
	    , object_wrapper_numbered(subfeature, feature)
	{
	}


	inline
	bool subfeature::test_flag(flags_enum flag) const
	{
	    return test_flag(1U << flag);
	}


	inline
	bool subfeature::operator==(const super &o) const
	{
	    return super::operator==(o);
	}

} /* namespace sensors */


template <typename Char, class Traits>
::std::basic_ostream<Char, Traits> &operator<<(::std::basic_ostream<Char, Traits> &out, const sensors::subfeature &sfeat)
{
	if (!!sfeat) {
	    out << *UTIL_CHECK_POINTER(sfeat.parent()) << '_' << sfeat->name;
	} else {
	    out << "<null>";
	}
	return out;
}

extern
template ::std::ostream &operator<<(::std::ostream &out, const sensors::subfeature &sfeat);

#endif /* SENSORS_SUBFEATURE_HPP_ */
