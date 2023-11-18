
/*
Code mostly just copied and packed from
https://www.freedesktop.org/software/systemd/man/latest/sd_bus_call_method.html#

licensed under:
SPDX-License-Identifier: MIT-0

*/
#include "dbus_client.h"
#include "dbus_privilege.h"

#define ERR_BUS_FAIL(msg) \
	ERR_FAIL_COND_MSG(r < 0, String(msg) + ": " + strerror(-r))

#define ERR_BUS_FAIL_V(msg, ret) \
	ERR_FAIL_COND_V_MSG(r < 0, ret, String(msg) + ": " + strerror(-r))

void DBusClient::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_request", "destination", "path", "interface", "member"), &DBusClient::create_request);
	ClassDB::bind_method(D_METHOD("send_request", "request"), &DBusClient::send_request);
	ClassDB::bind_method(D_METHOD("open"), &DBusClient::open);
	ClassDB::bind_method(D_METHOD("close"), &DBusClient::close);
	ClassDB::bind_method(D_METHOD("is_open"), &DBusClient::is_open);
}

DBusClient::DBusClient() {
}

DBusClient::~DBusClient() {
	if (_open) {
		close();
	}
}

void DBusClient::open() {
	int r = D_BUS_OPEN(&_bus);
	ERR_BUS_FAIL("Failed to acquire bus");
	_open = true;
}

void DBusClient::close() {
	sd_bus_error_free(&_error);
	sd_bus_unref(_bus);
	_open = false;
}

bool DBusClient::is_open() const {
	return _open;
}

Ref<DBusMessage> DBusClient::create_request(const String &p_destination, const String &p_path, const String &p_interface, const String &p_member) {
	ERR_FAIL_COND_V(!_open, nullptr);
	sd_bus_message *reply = nullptr;
	int r = sd_bus_message_new_method_call(_bus, &reply,
			p_destination.utf8(),
			p_path.utf8(),
			p_interface.utf8(),
			p_member.utf8());
	ERR_BUS_FAIL_V(String("Failed to created request: ") + _error.message, nullptr);
	return DBusMessage::from_internal(reply, true);
}

Ref<DBusMessage> DBusClient::send_request(const Ref<DBusMessage> &p_request) {
	ERR_FAIL_COND_V(!_open, nullptr);
	sd_bus_message *reply = nullptr;
	int r = sd_bus_call(_bus, p_request->get_internal(), 0, &_error, &reply);
	ERR_BUS_FAIL_V(String("Failed to send request: ") + _error.message, nullptr);
	return DBusMessage::from_internal(reply, true);
}
