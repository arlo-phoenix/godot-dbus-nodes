#pragma once

#include "dbus_level.h"
#include "dbus_method.h"
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/hash_map.hpp>

#include <mutex>
#include <thread>

using namespace godot;

class DBusServerNode : public Node {
	GDCLASS(DBusServerNode, Node);

private:
	std::thread _thread;
	CharString _object_path;
	CharString _interface_name;
	Array _methods;
	bool _autostart;
	DBusLevel::Level _bus_level = DBusLevel::USER;

	std::mutex _lock;
	bool _running;

	HashMap<String, Ref<DBusMethod>> _method_map;

	sd_bus_slot *_slot;
	sd_bus *_bus;
	sd_bus_vtable *v_table;
	int _v_table_size;

	void _server_thread_loop();
	void _set_running(const bool p_running);

protected:
	static void _bind_methods();

public:
	DBusServerNode();
	~DBusServerNode();
	void _notification(int p_what);

	void set_object_path(const String &p_name);
	String get_object_path() const;

	void set_interface_name(const String &p_name);
	String get_interface_name() const;

	Array get_methods() const;
	void set_methods(const Array &p_methods);

	void set_autostart(const bool p_autostart);
	bool get_autostart() const;

	void set_bus_level(const DBusLevel::Level p_bus_level);
	DBusLevel::Level get_bus_level() const;

	bool is_running();

	void start();
	void stop();

	static int _server_callback_wrapper(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
	int _server_callback(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
};
