#pragma once

#include <systemd/sd-bus.h>
#include <godot_cpp/classes/ref_counted.hpp>

#include "dbus_message.h"

using namespace godot;

class DBusClient : public RefCounted {
	GDCLASS(DBusClient, RefCounted)
private:
	sd_bus *_bus = nullptr;
	sd_bus_error _error = SD_BUS_ERROR_NULL;

	bool _open = false;

protected:
	static void _bind_methods();

public:
	DBusClient();
	~DBusClient();

	void open();
	void close();
	bool is_open() const;

	Ref<DBusMessage> create_request(const String &p_destination, const String &p_path, const String &p_interface, const String &p_member);
	Ref<DBusMessage> send_request(const Ref<DBusMessage> &p_request);
};