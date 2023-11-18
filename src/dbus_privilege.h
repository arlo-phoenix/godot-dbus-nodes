#pragma once

#ifdef GODOT_DBUS_SYSTEM_LEVEL
#define D_BUS_OPEN sd_bus_open_system
#else
#define D_BUS_OPEN sd_bus_open_user
#endif