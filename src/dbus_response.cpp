#include "dbus_response.h"
#include "dbus_method.h"

void DBusResponse::_bind_methods() {
	ClassDB::bind_method(D_METHOD("read_all"), &DBusResponse::read_all);
	ClassDB::bind_method(D_METHOD("reply", "args"), &DBusResponse::reply);
	ClassDB::bind_method(D_METHOD("set_error", "name", "message"), &DBusResponse::set_error);

	ClassDB::bind_method(D_METHOD("set_response_code", "response_code"), &DBusResponse::set_response_code);
	ClassDB::bind_method(D_METHOD("get_response_code"), &DBusResponse::get_response_code);
	ClassDB::bind_method(D_METHOD("is_valid"), &DBusResponse::is_valid);

	ClassDB::bind_method(D_METHOD("get_message"), &DBusResponse::get_message);
	ClassDB::bind_method(D_METHOD("read"), &DBusResponse::read_all);
	ClassDB::bind_method(D_METHOD("read_single", "type"), &DBusResponse::read_single);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "response_code"), "set_response_code", "get_response_code");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "message"), "", "get_message");
}

Ref<DBusResponse> DBusResponse::from_internal(sd_bus_message *m, void *userdata, sd_bus_error *ret_error) {
	Ref<DBusResponse> message;
	message.instantiate();
	message->set_internal_message(m);
	message->set_userdata(userdata);
	message->set_bus_error(ret_error);

	return message;
}

DBusResponse::DBusResponse() :
		_message(nullptr), _userdata(nullptr), _ret_error(nullptr), _response_code(0) {
}

void DBusResponse::set_internal_message(sd_bus_message *p_message) {
	_message = DBusMessage::from_internal(p_message);
}

void DBusResponse::set_userdata(void *p_userdata) {
	_userdata = p_userdata;
}

void DBusResponse::set_bus_error(sd_bus_error *p_ret_error) {
	_ret_error = p_ret_error;
}

void DBusResponse::set_signature(const Array &p_signature) {
	_signature = p_signature;
}

void DBusResponse::set_result(const Array &p_result) {
	_result = p_result;
}

Array DBusResponse::read_all() {
	Array input_arguments;
	input_arguments.resize(_signature.size());

	for (int i = 0; i < _signature.size(); i++) {
		const Ref<DBusMethodArgument> signature_argument = _signature[i];
		input_arguments[i] = _message->read_single_argument(signature_argument);
	}
	return input_arguments;
}

Variant DBusResponse::read_single(const Variant::Type p_type) {
	return _message->read_single(p_type);
}

void DBusResponse::reply(const Array &args) {
	ERR_FAIL_COND(args.size() != _result.size());
	InternalDBusMessageContainer reply;

	_response_code = sd_bus_message_new_method_return(_message->get_internal(), &reply.ptr);
	ERR_FAIL_COND_MSG(_response_code < 0, "failed to create return message");
	Ref<DBusMessage> reply_message = DBusMessage::from_internal(reply.ptr);

	for (int i = 0; i < args.size(); i++) {
		const Ref<DBusMethodArgument> result_argument = _result[i];
		const Variant &arg = args[i];
		ERR_FAIL_COND(result_argument->get_type() != arg.get_type());
		reply_message->append(arg);
	}
	ERR_FAIL_COND(!reply_message->is_valid());
	// Send the reply message
	_response_code = sd_bus_send(nullptr, reply.ptr, NULL);
	ERR_FAIL_COND_MSG(_response_code < 0, "Failed to send message");
}

void DBusResponse::set_error(const String &p_name, const String &p_message) {
	sd_bus_error_set_const(_ret_error, p_name.utf8().get_data(), p_message.utf8().get_data());
}

int DBusResponse::get_response_code() const {
	return _response_code;
}

void DBusResponse::set_response_code(const int p_response_code) {
	_response_code = p_response_code;
}

Ref<DBusMessage> DBusResponse::get_message() const {
	return _message;
}

bool DBusResponse::is_valid() const {
	return _response_code >= 0;
}

String DBusResponse::get_member() const {
	return _message->get_member();
}
