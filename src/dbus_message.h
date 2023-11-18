#pragma once

#include "dbus_method.h"
#include "systemd/sd-bus.h"

#include <godot_cpp/classes/ref_counted.hpp>

using namespace godot;

/**
 * @brief Helper struct to ensure message is always unreffed
 *
 */
struct InternalDBusMessageContainer {
	sd_bus_message *ptr = nullptr;

	~InternalDBusMessageContainer() {
		if (ptr != nullptr) {
			sd_bus_message_unref(ptr);
		}
	}
};

/**
 * @brief Message container with utility methods to append/read
 *
 * Only unrefs message if unref is set
 *
 */
class DBusMessage : public RefCounted {
	GDCLASS(DBusMessage, RefCounted)
private:
	sd_bus_message *_message;
	int _r;
	bool _unref = false;

protected:
	static void _bind_methods();

public:
	static Ref<DBusMessage> from_internal(sd_bus_message *p_message, const bool unref = false);
	DBusMessage();
	~DBusMessage();

	void set_internal(sd_bus_message *p_message);
	sd_bus_message *get_internal() const;
	void set_unref(const bool p_unref);

	Variant read_single(const Variant::Type p_type);
	Variant read_single_argument(const Ref<DBusMethodArgument> &arg);
	Array read_all_arguments(const TypedArray<DBusMethodArgument> &p_args);

	void append(const Variant &p_value);
	void append_all(const Array &p_values);

	/**
	 * @brief If _response_code>=0
	 *
	 * @return true
	 * @return false
	 */
	bool is_valid() const;
	String get_member() const;
};