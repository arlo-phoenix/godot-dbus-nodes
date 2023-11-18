#include "dbus_response.h"
#include "dbus_method.h"

void DBusRequest::_bind_methods() {
	ClassDB::bind_method(D_METHOD("read_all"), &DBusRequest::read_all);
	ClassDB::bind_method(D_METHOD("reply", "args"), &DBusRequest::reply);
	ClassDB::bind_method(D_METHOD("set_error", "name", "message"), &DBusRequest::set_error);

	ClassDB::bind_method(D_METHOD("set_response_code", "response_code"), &DBusRequest::set_response_code);
	ClassDB::bind_method(D_METHOD("get_response_code"), &DBusRequest::get_response_code);
	ClassDB::bind_method(D_METHOD("is_valid"), &DBusRequest::is_valid);

	ClassDB::bind_method(D_METHOD("get_message"), &DBusRequest::get_message);
	ClassDB::bind_method(D_METHOD("read"), &DBusRequest::read_all);
	ClassDB::bind_method(D_METHOD("read_single", "type"), &DBusRequest::read_single);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "response_code"), "set_response_code", "get_response_code");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "message"), "", "get_message");
}

Ref<DBusRequest> DBusRequest::from_internal(sd_bus_message *m, void *userdata, sd_bus_error *ret_error) {
	Ref<DBusRequest> message;
	message.instantiate();
	message->set_internal_message(m);
	message->set_userdata(userdata);
	message->set_bus_error(ret_error);

	return message;
}

DBusRequest::DBusRequest() :
		_message(nullptr), _userdata(nullptr), _ret_error(nullptr), _response_code(0) {
}

void DBusRequest::set_internal_message(sd_bus_message *p_message) {
	_message = DBusMessage::from_internal(p_message);
}

void DBusRequest::set_userdata(void *p_userdata) {
	_userdata = p_userdata;
}

void DBusRequest::set_bus_error(sd_bus_error *p_ret_error) {
	_ret_error = p_ret_error;
}

void DBusRequest::set_signature(const Array &p_signature) {
	_signature = p_signature;
}

void DBusRequest::set_result(const Array &p_result) {
	_result = p_result;
}

Array DBusRequest::read_all() {
	Array input_arguments;
	input_arguments.resize(_signature.size());

	for (int i = 0; i < _signature.size(); i++) {
		const Ref<DBusMethodArgument> signature_argument = _signature[i];
		input_arguments[i] = _message->read_single_argument(signature_argument);
	}
	return input_arguments;
}

Variant DBusRequest::read_single(const Variant::Type p_type) {
	return _message->read_single(p_type);
}

void DBusRequest::reply(const Array &args) {
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

void DBusRequest::set_error(const String &p_name, const String &p_message) {
	sd_bus_error_set_const(_ret_error, p_name.utf8().get_data(), p_message.utf8().get_data());
}

int DBusRequest::get_response_code() const {
	return _response_code;
}

void DBusRequest::set_response_code(const int p_response_code) {
	_response_code = p_response_code;
}

Ref<DBusMessage> DBusRequest::get_message() const {
	return _message;
}

bool DBusRequest::is_valid() const {
	return _response_code >= 0;
}

String DBusRequest::get_member() const {
	return _message->get_member();
}
