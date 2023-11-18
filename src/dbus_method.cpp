#include "dbus_method.h"
#include "conversion_utils.h"
#include <godot_cpp/variant/utility_functions.hpp>

DBusMethodArgument::DBusMethodArgument() :
		_type(Variant::Type::INT) {
}

void DBusMethodArgument::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_argument_name", "name"), &DBusMethodArgument::set_argument_name);
	ClassDB::bind_method(D_METHOD("get_argument_name"), &DBusMethodArgument::get_argument_name);

	ClassDB::bind_method(D_METHOD("set_type", "type"), &DBusMethodArgument::set_type);
	ClassDB::bind_method(D_METHOD("get_type"), &DBusMethodArgument::get_type);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "arg_name"), "set_argument_name", "get_argument_name");
	//would probably be safer to use custom enum, but currently those values are at the top so can be used as hint
	ADD_PROPERTY(PropertyInfo(Variant::INT, "type", PROPERTY_HINT_ENUM, "None, bool, int, float, string"), "set_type", "get_type");
}

void DBusMethodArgument::set_argument_name(const String &p_name) {
	_name = p_name;
}

String DBusMethodArgument::get_argument_name() const {
	return _name;
}

void DBusMethodArgument::set_type(const Variant::Type p_type) {
	_type = p_type;
}

Variant::Type DBusMethodArgument::get_type() const {
	return _type;
}

char DBusMethodArgument::get_internal_type() const {
	return variant_to_specifier(_type);
}

String DBusMethodArgument::list_to_internal(const Array &p_args) {
	String internal_args;
	for (int i = 0; i < p_args.size(); i++) {
		const Ref<DBusMethodArgument> arg = p_args[i];
		internal_args += arg->get_internal_type();
	}
	return internal_args;
}

/*

Start DBusMethod

*/

void DBusMethod::_bind_methods() {
	BIND_BITFIELD_FLAG(DEPRECATED);
	BIND_BITFIELD_FLAG(HIDDEN);
	BIND_BITFIELD_FLAG(UNPRIVILEGED);
	BIND_BITFIELD_FLAG(METHOD_NO_REPLY);
	BIND_BITFIELD_FLAG(PROPERTY_CONST);
	BIND_BITFIELD_FLAG(PROPERTY_EMITS_CHANGE);
	BIND_BITFIELD_FLAG(PROPERTY_EMITS_INVALIDATION);
	BIND_BITFIELD_FLAG(PROPERTY_EXPLICIT);
	BIND_BITFIELD_FLAG(SENSITIVE);

	ClassDB::bind_method(D_METHOD("set_member", "member"), &DBusMethod::set_member);
	ClassDB::bind_method(D_METHOD("get_member"), &DBusMethod::get_member);

	ClassDB::bind_method(D_METHOD("set_signature", "signature"), &DBusMethod::set_signature);
	ClassDB::bind_method(D_METHOD("get_signature"), &DBusMethod::get_signature);

	ClassDB::bind_method(D_METHOD("set_result", "result"), &DBusMethod::set_result);
	ClassDB::bind_method(D_METHOD("get_result"), &DBusMethod::get_result);

	ClassDB::bind_method(D_METHOD("set_flags", "flags"), &DBusMethod::set_flags);
	ClassDB::bind_method(D_METHOD("get_flags"), &DBusMethod::get_flags);

	ClassDB::bind_method(D_METHOD("set_callback", "callback"), &DBusMethod::set_callback);
	ClassDB::bind_method(D_METHOD("get_callback"), &DBusMethod::get_callback);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "member"), "set_member", "get_member");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "signature", PROPERTY_HINT_ARRAY_TYPE, vformat("%s/%s:%s", Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE, "DBusMethodArgument")), "set_signature", "get_signature");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "result", PROPERTY_HINT_ARRAY_TYPE, vformat("%s/%s:%s", Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE, "DBusMethodArgument")), "set_result", "get_result");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "flags", PROPERTY_HINT_FLAGS, "deprecated, hidden, unprivileged, method no reply, property const, property emits change, property emits invalidation, property explicit, sensitive"), "set_flags", "get_flags");
	ADD_PROPERTY(PropertyInfo(Variant::CALLABLE, "callback"), "set_callback", "get_callback");
}

void DBusMethod::_replace_null_from_list(TypedArray<DBusMethodArgument> &p_array) {
	for (int i = 0; i < p_array.size(); i++) {
		if (p_array[i].get_type() == Variant::Type::NIL) {
			Ref<DBusMethodArgument> arg;
			arg.instantiate();
			p_array[i] = arg;
		}
	}
}

void DBusMethod::set_member(const String &p_member) {
	_member = p_member.utf8();
}

String DBusMethod::get_member() const {
	return String(_member);
}

void DBusMethod::set_signature(const TypedArray<DBusMethodArgument> &p_signature) {
	_signature = p_signature;
	_replace_null_from_list(_signature);
	_internal_signature = DBusMethodArgument::list_to_internal(p_signature).utf8();
}

TypedArray<DBusMethodArgument> DBusMethod::get_signature() const {
	return _signature;
}

void DBusMethod::set_result(const TypedArray<DBusMethodArgument> &p_result) {
	_result = p_result;
	_replace_null_from_list(_result);
	_internal_result = DBusMethodArgument::list_to_internal(p_result).utf8();
}

TypedArray<DBusMethodArgument> DBusMethod::get_result() const {
	return _result;
}

void DBusMethod::set_flags(const BitField<Flags> flags) {
	_flags = flags;
}

BitField<DBusMethod::Flags> DBusMethod::get_flags() const {
	return _flags;
}

void DBusMethod::set_callback(const Callable &p_callback) {
	_callback = p_callback;
}

Callable DBusMethod::get_callback() const {
	return _callback;
}

sd_bus_vtable DBusMethod::to_internal(sd_bus_message_handler_t p_callback) const {
	UtilityFunctions::print("Adding method " + get_member() + ": " + DBusMethodArgument::list_to_internal(_signature) + " -> " + DBusMethodArgument::list_to_internal(_result));
	ERR_FAIL_COND_V_EDMSG(_member.size() == 0, sd_bus_vtable(), "Missing method name");
	return SD_BUS_METHOD(_member,
			_internal_signature,
			_internal_result,
			p_callback,
			static_cast<uint64_t>(_flags));
}