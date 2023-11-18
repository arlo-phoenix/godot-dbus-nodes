#!/usr/bin/env python
from glob import glob
from pathlib import Path

env = SConscript("godot-cpp/SConstruct")

# Add source files.
env.Append(CPPPATH=["src/"])
env.ParseConfig(f"pkg-config libsystemd --cflags --libs")
sources = Glob("src/*.cpp")

project_name = "godot_dbus"
extension_path = f"project/addons/{project_name}/{project_name}.gdextension"
addon_path = Path(extension_path).parent

debug_or_release = "release" if env["target"] == "template_release" else "debug"
if env["platform"] == "macos":
    library = env.SharedLibrary(
        "{0}/bin/lib{1}.{2}.{3}.framework/{1}.{2}.{3}".format(
            addon_path,
            project_name,
            env["platform"],
            debug_or_release,
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "{}/bin/lib{}.{}.{}.{}{}".format(
            addon_path,
            project_name,
            env["platform"],
            debug_or_release,
            env["arch"],
            env["SHLIBSUFFIX"],
        ),
        source=sources,
    )

Default(library)
