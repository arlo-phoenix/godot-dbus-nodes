#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/binder_common.hpp>

using namespace godot;

class DBusLevel : public Object {
	GDCLASS(DBusLevel, Object)

protected:
	static void _bind_methods();

public:
	enum Level : int {
		USER = 0,
		SYSTEM = 1
	};
};

VARIANT_ENUM_CAST(DBusLevel::Level);