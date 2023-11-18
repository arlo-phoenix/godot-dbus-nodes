#include "dbus_message.h"
#include "conversion_utils.h"

#define READ_VARIANT(_variant_type, _type)                                                                        \
	case Variant::Type::_variant_type: {                                                                          \
		_type _variable;                                                                                          \
		_r = sd_bus_message_read_basic(_message, variant_to_specifier(Variant::Type::_variant_type), &_variable); \
		ret = Variant(_variable);                                                                                 \
		break;                                                                                                    \
	}

#define APPEND_VARIANT(_variant_type, _type)                                                                        \
	case Variant::Type::_variant_type: {                                                                            \
		_type _variable = p_value;                                                                                  \
		_r = sd_bus_message_append_basic(_message, variant_to_specifier(Variant::Type::_variant_type), &_variable); \
		break;                                                                                                      \
	}

void DBusMessage::_bind_methods() {
	ClassDB::bind_method(D_METHOD("read_single", "p_type"), &DBusMessage::read_single);
	ClassDB::bind_method(D_METHOD("read_single_argument", "arg"), &DBusMessage::read_single_argument);
	ClassDB::bind_method(D_METHOD("read_all_arguments", "p_args"), &DBusMessage::read_all_arguments);
	ClassDB::bind_method(D_METHOD("append", "p_value"), &DBusMessage::append);
	ClassDB::bind_method(D_METHOD("append_all", "p_values"), &DBusMessage::append_all);
	ClassDB::bind_method(D_METHOD("is_valid"), &DBusMessage::is_valid);
	ClassDB::bind_method(D_METHOD("get_member"), &DBusMessage::get_member);
}

Ref<DBusMessage> DBusMessage::from_internal(sd_bus_message *p_message, const bool unref) {
	Ref<DBusMessage> msg;
	msg.instantiate();
	msg->set_internal(p_message);
	msg->set_unref(unref);
	return msg;
}

DBusMessage::DBusMessage() :
		_message(nullptr), _r(0) {
}

DBusMessage::~DBusMessage() {
	if (_unref && _message != nullptr) {
		sd_bus_message_unref(_message);
	}
}

void DBusMessage::set_internal(sd_bus_message *p_message) {
	_message = p_message;
}

sd_bus_message *DBusMessage::get_internal() const {
	return _message;
}

void DBusMessage::set_unref(const bool p_unref) {
	_unref = p_unref;
}

bool DBusMessage::is_valid() const {
	return _r >= 0;
}

String DBusMessage::get_member() const {
	return sd_bus_message_get_member(_message);
}

Variant DBusMessage::read_single_argument(const Ref<DBusMethodArgument> &arg) {
	return read_single(arg->get_type());
}

Variant DBusMessage::read_single(const Variant::Type p_type) {
	//see https://www.freedesktop.org/software/systemd/man/latest/sd_bus_message_read.html
	//for all options
	Variant ret;
	switch (p_type) {
		READ_VARIANT(BOOL, int)
		READ_VARIANT(INT, int64_t)
		READ_VARIANT(FLOAT, double)
		READ_VARIANT(STRING, const char *)
		default:
			ERR_FAIL_V_MSG(Variant(), "Unsupported type");
	}

	ERR_FAIL_COND_V_MSG(_r < 0, Variant(), "Failed to parse parameters");
	return ret;
}

Array DBusMessage::read_all_arguments(const TypedArray<DBusMethodArgument> &p_args) {
	Array ret;
	ret.resize(p_args.size());
	for (int i = 0; i < p_args.size(); i++) {
		const Ref<DBusMethodArgument> &arg = p_args[i];
		ret[i] = read_single_argument(arg);
	}
	return ret;
}

void DBusMessage::append(const Variant &p_value) {
	switch (p_value.get_type()) {
		APPEND_VARIANT(BOOL, int)
		APPEND_VARIANT(INT, int64_t)
		APPEND_VARIANT(FLOAT, double)
		case Variant::Type::STRING: {
			_r = sd_bus_message_append_basic(_message, p_value.get_type(), String(p_value).utf8().get_data());
			break;
		}
		default:
			ERR_FAIL_MSG("Unsupported type");
			break;
	}
	ERR_FAIL_COND_MSG(_r < 0, "Failed to append value");
}

void DBusMessage::append_all(const Array &p_values) {
	for (int i = 0; i < p_values.size(); i++) {
		append(p_values[i]);
	}
}
