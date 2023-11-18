#pragma once

#include <dbus_client.h>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/thread.hpp>

using namespace godot;

/**
 * @brief Just pasted properties from DBusClient and async callback implementation
 *
 */
class DBusClientNode : public Node {
	GDCLASS(DBusClientNode, Node)
private:
	Ref<DBusClient> _client;
	String _destination;
	String _path;
	String _interface;

	/**
	 * @brief internal request which sends and waits for a response, threaded if _use_threads is set
	 *
	 * @param p_request
	 * @param callback
	 */
	void _request(const Ref<DBusMessage> &p_request, Callable callback);
	Thread _thread;
	bool _autostart = true;
	bool _use_threads = false;

protected:
	static void _bind_methods();

public:
	DBusClientNode();
	~DBusClientNode();

	void _notification(int p_what);

	void set_destination(const String &p_destination);
	String get_destination() const;

	void set_path(const String &p_path);
	String get_path() const;

	void set_interface(const String &p_interface);
	String get_interface() const;

	void open();
	void close();
	bool is_open() const;

	void set_autostart(const bool p_autostart);
	bool get_autostart() const;

	void set_use_threads(const bool p_use_threads);
	bool get_use_threads() const;

	/**
	 * @brief Request member, callback, arg1, arg2,... (member and callback are necessary)
	 *
	 * @param p_args
	 * @param p_arg_count
	 * @param error
	 * @return Error
	 */
	Error request(const Variant **p_args, GDExtensionInt p_arg_count, GDExtensionCallError &error);

	//start utility
	/**
	 * @brief busctl introspect the service
	 *
	 * @return String
	 */
	String introspect();
	/**
	 * @brief useful to replicate tree call
	 *
	 * @return String
	 *
	 */
	String introspect_path(const String &object_path);
};