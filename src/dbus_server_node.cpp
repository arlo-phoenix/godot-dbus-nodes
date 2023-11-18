#include "dbus_server_node.h"

#include "dbus_response.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#define ERR_BUS_FAIL(msg) \
	ERR_FAIL_COND_MSG(r < 0, String(msg) + ": " + strerror(-r))

using namespace godot;

void DBusServerNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_object_path", "p_name"), &DBusServerNode::set_object_path);
	ClassDB::bind_method(D_METHOD("get_object_path"), &DBusServerNode::get_object_path);

	ClassDB::bind_method(D_METHOD("set_interface_name", "p_name"), &DBusServerNode::set_interface_name);
	ClassDB::bind_method(D_METHOD("get_interface_name"), &DBusServerNode::get_interface_name);

	ClassDB::bind_method(D_METHOD("get_methods"), &DBusServerNode::get_methods);
	ClassDB::bind_method(D_METHOD("set_methods", "p_methods"), &DBusServerNode::set_methods);

	ClassDB::bind_method(D_METHOD("set_autostart", "autostart"), &DBusServerNode::set_autostart);
	ClassDB::bind_method(D_METHOD("get_autostart"), &DBusServerNode::get_autostart);

	ClassDB::bind_method(D_METHOD("is_running"), &DBusServerNode::is_running);
	ClassDB::bind_method(D_METHOD("start"), &DBusServerNode::start);
	ClassDB::bind_method(D_METHOD("stop"), &DBusServerNode::stop);

	ClassDB::bind_method(D_METHOD("_server_thread_loop"), &DBusServerNode::_server_thread_loop);
	ClassDB::bind_method(D_METHOD("_process_bus"), &DBusServerNode::_process_bus);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "object_path"), "set_object_path", "get_object_path");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "interface_name"), "set_interface_name", "get_interface_name");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "methods", PROPERTY_HINT_ARRAY_TYPE, vformat("%s/%s:%s", Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE, "DBusMethod")), "set_methods", "get_methods");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "autostart"), "set_autostart", "get_autostart");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "running"), "", "is_running");
}

DBusServerNode::DBusServerNode() :
		_slot(nullptr), _bus(nullptr), _running(false), v_table(nullptr) {
}

DBusServerNode::~DBusServerNode() {
	if (_running) {
		stop();
	}
}

void DBusServerNode::stop() {
	ERR_FAIL_COND_MSG(!_running, "Already stopped");
	_set_running(false);
	if (_thread.is_alive()) {
		_thread.wait_to_finish();
	}
	sd_bus_slot_unref(_slot);
	sd_bus_unref(_bus);
	_slot = nullptr;
	_bus = nullptr;

	delete[] v_table;
	v_table = nullptr;
	_v_table_size = 0;
}

void DBusServerNode::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY) {
		if (Engine::get_singleton()->is_editor_hint()) {
			return;
		}

		if (_autostart) {
			start();
		}
	}
}

void DBusServerNode::_server_thread_loop() {
	int r;
	const uint64_t BUS_WAIT_TIME = 50000; //0.5s
	while (is_running()) {
		//do processing on main thread, since need to wait for callback anyways and sometimes this outputs notifications which crashes outside of the main thread
		r = call_deferred("_process_bus");
		ERR_BUS_FAIL("Failed to process bus");

		if (r > 0) //if processed try to process another one without waiting
			continue;

		//wait for next request
		r = sd_bus_wait(_bus, BUS_WAIT_TIME);
		if (r < 0) {
			if (is_running()) {
				ERR_BUS_FAIL("Failed to wait on bus");
			} else {
				//expected to fail at stop so don't print
				return;
			}
		}
	}
}

int DBusServerNode::_process_bus() {
	return sd_bus_process(_bus, NULL);
}

// Setter and Getter for _object_path
void DBusServerNode::set_object_path(const String &p_name) {
	ERR_FAIL_COND(_running);
	_object_path = p_name.utf8();
}

String DBusServerNode::get_object_path() const {
	return String(_object_path);
}

void DBusServerNode::set_interface_name(const String &p_name) {
	ERR_FAIL_COND(_running);
	_interface_name = p_name.utf8();
}

String DBusServerNode::get_interface_name() const {
	return String(_interface_name);
}

Array DBusServerNode::get_methods() const {
	return _methods;
}

void DBusServerNode::set_methods(const Array &p_methods) {
	ERR_FAIL_COND(_running);
	_methods = p_methods;
}

void DBusServerNode::set_autostart(const bool p_autostart) {
	_autostart = p_autostart;
}

bool DBusServerNode::get_autostart() const {
	return _autostart;
}

void DBusServerNode::_set_running(const bool p_running) {
	_lock.lock();
	_running = p_running;
	_lock.unlock();
}

bool DBusServerNode::is_running() {
	_lock.lock();
	const bool ret = _running;
	_lock.unlock();
	return ret;
}

void DBusServerNode::start() {
	//since start and end + 2
	_v_table_size = _methods.size() + 2;
	v_table = new sd_bus_vtable[_v_table_size];
	v_table[0] = SD_BUS_VTABLE_START(0);

	UtilityFunctions::print("Starting DBus server " + get_interface_name() + " " + get_object_path());
	for (int i = 0; i < _methods.size(); i++) {
		const Ref<DBusMethod> method_definition = _methods[i];
		v_table[i + 1] = method_definition->to_internal(_server_callback_wrapper);
		_method_map.insert(method_definition->get_member(), method_definition);
	}
	v_table[_v_table_size - 1] = SD_BUS_VTABLE_END;

	int r;
	r = sd_bus_open_user(&_bus);
	ERR_BUS_FAIL("Failed to connect system bus");

	r = sd_bus_add_object_vtable(_bus,
			&_slot,
			_object_path,
			_interface_name,
			v_table,
			nullptr);

	ERR_BUS_FAIL("Failed to issue method call");

	sd_bus_slot_set_userdata(_slot, this);

	r = sd_bus_request_name(_bus, _interface_name, 0);
	ERR_BUS_FAIL("Failed to acquire service name " + String(_interface_name));
	_thread.start(Callable(this, "_server_thread_loop"));
	_running = true;
}

int DBusServerNode::_server_callback_wrapper(sd_bus_message *m, void *userdata, sd_bus_error *ret_error) {
	DBusServerNode *server = static_cast<DBusServerNode *>(userdata);
	//callback is static so use userdata pointer to call object callback
	return server->_server_callback(m, userdata, ret_error);
}

int DBusServerNode::_server_callback(sd_bus_message *m, void *userdata, sd_bus_error *ret_error) {
	Ref<DBusResponse> message = DBusResponse::from_internal(m, userdata, ret_error);
	//call callback, DBusResponse contains all required data to read/respond
	if (!_method_map.has(message->get_member())) {
		return 0;
	}

	const Ref<DBusMethod> method = _method_map[message->get_member()];
	sd_bus_error_set_const(ret_error, SD_BUS_ERROR_NOT_SUPPORTED, "Method not implemented");
	ERR_FAIL_COND_V(!method->get_callback().is_valid(), -ENOSYS);
	message->set_signature(method->get_signature());
	message->set_result(method->get_result());
	return method->get_callback().call(message);
}
