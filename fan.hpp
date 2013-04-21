/*
 * fan.hpp
 *
 *  Created on: 22.12.2011
 *      Author: malte
 */

#pragma once
#ifndef FANCONTROL_FAN_HPP_
#define FANCONTROL_FAN_HPP_

#include "sensors++/exceptions.hpp"

#include "util/property_wrapper.hpp"
#include <boost/shared_ptr.hpp>

#include <string>


namespace sensors {
	class subfeature;
	class pwm;
}

namespace fancontrol {

	using boost::shared_ptr;

	using sensors::sensor_error;
	using util::io_error;
	using sensors::subfeature;
	using sensors::pwm;

	class control;
	class config;


	class fan
	{
	public:
		typedef const subfeature SF;

		typedef float value_t;

	private:
		struct gauge_type_guard {
			static bool check(const SF *gauge);
			static bool check(const shared_ptr<SF> &, const shared_ptr<SF> &gauge);
		};

		struct valve_type_guard {
			static bool check(const pwm *valve);
			static bool check(const shared_ptr<pwm> &, const shared_ptr<pwm> &valve);
		};

	public:
		fan();

		void write_valve(value_t value);

		void update_valve(bool force = false);

		void reset();

		bool operator==(const fan &o) const;

		shared_ptr<const control> m_dependency;

		value_t m_min_start, m_max_stop, m_reset_rate;

		class label_wrapper: public util::const_property_wrapper<std::string, util::guards::old_empty<std::string> > {
			friend class config;
		}
		m_label;

		class gauge_wrapper: public util::property_wrapper<shared_ptr<SF>, gauge_type_guard> {
		public:
			double read() const throw (sensor_error);
			friend class config;
		}
		m_gauge;

		class valve_wrapper: public util::property_wrapper<shared_ptr<pwm>, valve_type_guard> {
		public:
			value_t read() throw(io_error);
			void write(value_t value) throw(io_error);
			friend class config;
		}
		m_valve;

	private:
		value_t effective_value(value_t) const;

		void update_valve(bool force, value_t);

		value_t m_last_update;
	};



// implementation =============================================================

	inline
	bool fan::gauge_type_guard::check(const shared_ptr<SF> &, const shared_ptr<SF> &gauge)
	{
		return check(gauge.get());
	}


	inline
	bool fan::valve_type_guard::check(const shared_ptr<pwm> &, const shared_ptr<pwm> &valve)
	{
		return check(valve.get());
	}

} /* namespace fancontrol */
#endif /* FANCONTROL_FAN_HPP_ */
