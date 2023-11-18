#pragma once

#include <godot_cpp/classes/ref_counted.hpp>

using namespace godot;

#include "dbus_message.h"
#include "dbus_method.h"
#include "systemd/sd-bus.h"

class DBusRequest : public RefCounted {
	GDCLASS(DBusRequest, RefCounted)
private:
	Ref<DBusMessage> _message;
	void *_userdata;
	sd_bus_error *_ret_error;

	TypedArray<DBusMethodArgument> _signature;
	TypedArray<DBusMethodArgument> _result;

	int _response_code = 0;

protected:
	static void _bind_methods();

public:
	/**
	 * @brief
	 *
	 * @param m
	 * @param userdata
	 * @param ret_error
	 * @param p_signature_types Array<DBusMethodArgument>
	 * @param p_result Array<DBusMethodArgument>
	 * @return Ref<DBusMessage>
	 */
	static Ref<DBusRequest> from_internal(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
	DBusRequest();
	void set_internal_message(sd_bus_message *p_message);
	void set_userdata(void *p_userdata);
	void set_bus_error(sd_bus_error *p_ret_error);

	void set_signature(const Array &p_signature);
	void set_result(const Array &p_result);

	Array read_all();
	Variant read_single(const Variant::Type p_type);
	void reply(const Array &args);
	void set_error(const String &p_name, const String &p_message);

	int get_response_code() const;
	void set_response_code(const int p_response_code);

	Ref<DBusMessage> get_message() const;

	/**
	 * @brief If _response_code>=0
	 *
	 * @return true
	 * @return false
	 */
	bool is_valid() const;
	String get_member() const;
};
