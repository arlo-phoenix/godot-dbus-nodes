#include "dbus_client.h"
/*
Code mostly just copied and packed from
https://www.freedesktop.org/software/systemd/man/latest/sd_bus_call_method.html#

licensed under:
SPDX-License-Identifier: MIT-0

*/
#define ERR_BUS_FAIL(msg) \
	ERR_FAIL_COND_MSG(r < 0, String(msg) + ": " + strerror(-r))

#define ERR_BUS_FAIL_V(msg, ret) \
	ERR_FAIL_COND_V_MSG(r < 0, ret, String(msg) + ": " + strerror(-r))

void DBusClient::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_destination", "p_destination"), &DBusClient::set_destination);
	ClassDB::bind_method(D_METHOD("get_destination"), &DBusClient::get_destination);

	ClassDB::bind_method(D_METHOD("set_path", "p_path"), &DBusClient::set_path);
	ClassDB::bind_method(D_METHOD("get_path"), &DBusClient::get_path);

	ClassDB::bind_method(D_METHOD("set_interface", "p_interface"), &DBusClient::set_interface);
	ClassDB::bind_method(D_METHOD("get_interface"), &DBusClient::get_interface);

	ClassDB::bind_method(D_METHOD("create_request", "member"), &DBusClient::create_request);
	ClassDB::bind_method(D_METHOD("send_request", "request"), &DBusClient::send_request);
	ClassDB::bind_method(D_METHOD("open"), &DBusClient::open);
	ClassDB::bind_method(D_METHOD("close"), &DBusClient::close);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "destination"), "set_destination", "get_destination");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "path"), "set_path", "get_path");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "interface"), "set_interface", "get_interface");
}

DBusClient::DBusClient() {
}

DBusClient::~DBusClient() {
	if (_open) {
		close();
	}
}

void DBusClient::set_destination(const String &p_destination) {
	_destination = p_destination;
}

String DBusClient::get_destination() const {
	return _destination;
}

void DBusClient::set_path(const String &p_path) {
	_path = p_path;
}

String DBusClient::get_path() const {
	return _path;
}

void DBusClient::set_interface(const String &p_interface) {
	_interface = p_interface;
}

String DBusClient::get_interface() const {
	return _interface;
}

void DBusClient::open() {
	int r = sd_bus_open_user(&_bus);
	ERR_BUS_FAIL("Failed to acquire bus");
	_open = true;
}

void DBusClient::close() {
	sd_bus_error_free(&_error);
	sd_bus_unref(_bus);
	_open = false;
}

Ref<DBusMessage> DBusClient::create_request(const String &p_member) {
	ERR_FAIL_COND_V(!_open, nullptr);
	sd_bus_message *msg = nullptr;
	int r = sd_bus_message_new_method_call(_bus, &msg,
			_destination.utf8(),
			_path.utf8(),
			_interface.utf8(),
			p_member.utf8());
	ERR_BUS_FAIL_V(String("Failed to created request: ") + _error.message, nullptr);
	return DBusMessage::from_internal(msg, true);
}

Ref<DBusMessage> DBusClient::send_request(const Ref<DBusMessage> &p_request) {
	ERR_FAIL_COND_V(!_open, nullptr);
	sd_bus_message *reply = nullptr;
	int r = sd_bus_call(_bus, p_request->get_internal(), 0, &_error, &reply);
	ERR_BUS_FAIL_V(String("Failed to send request: ") + _error.message, nullptr);
	return DBusMessage::from_internal(reply, true);
}
