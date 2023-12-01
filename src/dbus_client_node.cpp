#include "dbus_client_node.h"
#include <godot_cpp/classes/engine.hpp>

void DBusClientNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_destination", "destination"), &DBusClientNode::set_destination);
	ClassDB::bind_method(D_METHOD("get_destination"), &DBusClientNode::get_destination);

	ClassDB::bind_method(D_METHOD("set_path", "path"), &DBusClientNode::set_path);
	ClassDB::bind_method(D_METHOD("get_path"), &DBusClientNode::get_path);

	ClassDB::bind_method(D_METHOD("set_interface", "interface"), &DBusClientNode::set_interface);
	ClassDB::bind_method(D_METHOD("get_interface"), &DBusClientNode::get_interface);

	ClassDB::bind_method(D_METHOD("open"), &DBusClientNode::open);
	ClassDB::bind_method(D_METHOD("close"), &DBusClientNode::close);
	ClassDB::bind_method(D_METHOD("is_open"), &DBusClientNode::is_open);

	ClassDB::bind_method(D_METHOD("set_autostart", "autostart"), &DBusClientNode::set_autostart);
	ClassDB::bind_method(D_METHOD("get_autostart"), &DBusClientNode::get_autostart);

	ClassDB::bind_method(D_METHOD("set_use_threads", "use_threads"), &DBusClientNode::set_use_threads);
	ClassDB::bind_method(D_METHOD("get_use_threads"), &DBusClientNode::get_use_threads);

	ClassDB::bind_method(D_METHOD("set_bus_level", "bus_level"), &DBusClientNode::set_bus_level);
	ClassDB::bind_method(D_METHOD("get_bus_level"), &DBusClientNode::get_bus_level);

	{
		MethodInfo mi;
		mi.arguments.push_back(PropertyInfo(Variant::STRING, "member"));
		mi.arguments.push_back(PropertyInfo(Variant::CALLABLE, "callback"));
		mi.name = "request";
		ClassDB::bind_vararg_method(METHOD_FLAGS_DEFAULT, "request", &DBusClientNode::request, mi);
	}
	ClassDB::bind_method(D_METHOD("_request", "request_message", "callback"), &DBusClientNode::_request);

	//start util
	ClassDB::bind_method(D_METHOD("introspect"), &DBusClientNode::introspect);
	ClassDB::bind_method(D_METHOD("introspect_path", "object_path"), &DBusClientNode::introspect_path);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "destination"), "set_destination", "get_destination");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "path"), "set_path", "get_path");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "interface"), "set_interface", "get_interface");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "autostart"), "set_autostart", "get_autostart");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_threads"), "set_use_threads", "get_use_threads");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "bus_level", PROPERTY_HINT_ENUM, "USER, SYSTEM"), "set_bus_level", "get_bus_level");
}

DBusClientNode::DBusClientNode() {
	_client.instantiate();
}

DBusClientNode::~DBusClientNode() {
	if (is_open()) {
		close();
	}
}

void DBusClientNode::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY) {
		if (Engine::get_singleton()->is_editor_hint()) {
			return;
		}

		if (_autostart) {
			open();
		}
	}
}

void DBusClientNode::set_destination(const String &p_destination) {
	_destination = p_destination;
}

String DBusClientNode::get_destination() const {
	return _destination;
}

void DBusClientNode::set_path(const String &p_path) {
	_path = p_path;
}

String DBusClientNode::get_path() const {
	return _path;
}

void DBusClientNode::set_interface(const String &p_interface) {
	_interface = p_interface;
}

String DBusClientNode::get_interface() const {
	return _interface;
}

void DBusClientNode::open() {
	_client->open(_bus_level);
}

void DBusClientNode::close() {
	if (_thread_running) {
		_thread.join();
	}

	_client->close();
}

bool DBusClientNode::is_open() const {
	return _client->is_open();
}

void DBusClientNode::set_autostart(const bool p_autostart) {
	_autostart = p_autostart;
}

bool DBusClientNode::get_autostart() const {
	return _autostart;
}

void DBusClientNode::set_use_threads(const bool p_use_threads) {
	_use_threads = p_use_threads;
}

bool DBusClientNode::get_use_threads() const {
	return _use_threads;
}

void DBusClientNode::set_bus_level(const DBusLevel::Level p_bus_level) {
	_bus_level = p_bus_level;
}

DBusLevel::Level DBusClientNode::get_bus_level() const {
	return _bus_level;
}

Error DBusClientNode::request(const Variant **p_args, GDExtensionInt p_arg_count, GDExtensionCallError &error) {
	if (p_arg_count < 2) {
		error.error = GDEXTENSION_CALL_ERROR_TOO_FEW_ARGUMENTS;
		error.expected = 2;
		return ERR_INVALID_PARAMETER;
	}

	Variant::Type type = p_args[0]->get_type();
	if (type != Variant::STRING && type != Variant::STRING_NAME) {
		error.error = GDEXTENSION_CALL_ERROR_INVALID_ARGUMENT;
		error.argument = 0;
		error.expected = Variant::STRING;
		return ERR_INVALID_PARAMETER;
	}
	const String member = *p_args[0];

	type = p_args[1]->get_type();
	if (type != Variant::CALLABLE) {
		error.error = GDEXTENSION_CALL_ERROR_INVALID_ARGUMENT;
		error.argument = 1;
		error.expected = Variant::CALLABLE;
		return ERR_INVALID_PARAMETER;
	}
	const Callable callback = *p_args[1];

	if (_thread_running) {
		_thread.join();
	}
	_thread_running = true;

	Ref<DBusMessage> request_message = _client->create_request(_destination,
			_path,
			_interface,
			member);

	for (int i = 2; i < p_arg_count; i++) {
		request_message->append(*p_args[i]);
	}

	if (_use_threads) {
		_thread = std::thread([this, request_message, callback]() {
			_request(request_message, callback);
		});
	} else {
		_request(request_message, callback);
	}

	return OK;
}

String DBusClientNode::introspect() {
	ERR_FAIL_COND_V(!is_open(), String());
	Ref<DBusMessage> request_message = _client->create_request(get_destination(), get_path(), "org.freedesktop.DBus.Introspectable", "Introspect");
	ERR_FAIL_COND_V(request_message.is_null(), String());
	Ref<DBusMessage> response = _client->send_request(request_message);
	ERR_FAIL_COND_V(response.is_null(), String());
	return response->read_single(Variant::Type::STRING);
}

String DBusClientNode::introspect_path(const String &object_path) {
	ERR_FAIL_COND_V(!is_open(), String());
	Ref<DBusMessage> request_message = _client->create_request(get_destination(), object_path, "org.freedesktop.DBus.Introspectable", "Introspect");
	ERR_FAIL_COND_V(request_message.is_null(), String());
	Ref<DBusMessage> response = _client->send_request(request_message);
	ERR_FAIL_COND_V(response.is_null(), String());
	return response->read_single(Variant::Type::STRING);
}

void DBusClientNode::_request(const Ref<DBusMessage> &p_request, Callable callback) {
	Ref<DBusMessage> response = _client->send_request(p_request);
	if (_use_threads) {
		callback.call(response);
	} else {
		callback.call(response);
	}
}
