#pragma once

#include "systemd/sd-bus-vtable.h"
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

class DBusMethodArgument : public Resource {
	GDCLASS(DBusMethodArgument, Resource);

private:
	String _name;
	Variant::Type _type = Variant::NIL;

protected:
	static void _bind_methods();

public:
	DBusMethodArgument();
	void set_argument_name(const String &p_name);
	String get_argument_name() const;

	void set_type(const Variant::Type p_type);
	Variant::Type get_type() const;
	char get_internal_type() const;

	static String list_to_internal(const Array &p_args);
};

class DBusMethod : public Resource {
	GDCLASS(DBusMethod, Resource);

public:
	enum Flags : uint64_t {
		DEPRECATED = SD_BUS_VTABLE_DEPRECATED,
		HIDDEN = SD_BUS_VTABLE_HIDDEN,
		UNPRIVILEGED = SD_BUS_VTABLE_UNPRIVILEGED,
		METHOD_NO_REPLY = SD_BUS_VTABLE_METHOD_NO_REPLY,
		PROPERTY_CONST = SD_BUS_VTABLE_PROPERTY_CONST,
		PROPERTY_EMITS_CHANGE = SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE,
		PROPERTY_EMITS_INVALIDATION = SD_BUS_VTABLE_PROPERTY_EMITS_INVALIDATION,
		PROPERTY_EXPLICIT = SD_BUS_VTABLE_PROPERTY_EXPLICIT,
		SENSITIVE = SD_BUS_VTABLE_SENSITIVE,
		ABSOLUTE_OFFSET = SD_BUS_VTABLE_ABSOLUTE_OFFSET,
		CAPABILITY_MASK = _SD_BUS_VTABLE_CAPABILITY_MASK
	};

private:
	/**
	 * @brief Name of function
	 *
	 * stored as CharString so that pointer stays for VTable
	 *
	 */
	CharString _member;

	//Array of variant type, will be converted to string signature/result used by dbus
	TypedArray<DBusMethodArgument> _signature;
	TypedArray<DBusMethodArgument> _result;

	//needs to be stored since vtable might access
	CharString _internal_signature;
	CharString _internal_result;

	BitField<Flags> _flags = Flags::UNPRIVILEGED;

	Callable _callback;

	void _replace_null_from_list(TypedArray<DBusMethodArgument> &p_array);

protected:
	static void _bind_methods();

public:
	void set_member(const String &p_member);
	String get_member() const;

	void set_signature(const TypedArray<DBusMethodArgument> &p_signature);
	TypedArray<DBusMethodArgument> get_signature() const;

	void set_result(const TypedArray<DBusMethodArgument> &p_result);
	TypedArray<DBusMethodArgument> get_result() const;

	void set_flags(const BitField<Flags> flags);
	BitField<Flags> get_flags() const;

	void set_callback(const Callable &p_callback);
	Callable get_callback() const;

	sd_bus_vtable to_internal(sd_bus_message_handler_t p_callback) const;
};

VARIANT_BITFIELD_CAST(DBusMethod::Flags);