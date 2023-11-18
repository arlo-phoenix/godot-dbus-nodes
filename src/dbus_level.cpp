#include "dbus_level.h"

#include <godot_cpp/core/class_db.hpp>

void DBusLevel::_bind_methods() {
	BIND_ENUM_CONSTANT(USER);
	BIND_ENUM_CONSTANT(SYSTEM);
}