# Godot D-Bus Nodes

GDExtension that offers simple D-Bus client and server support build on top of libsystemd.

This implementation is very minimal and does not support all specifiers in arguments. If you want full and efficient DBus support I recommend  the C++ bindings under [sdbus-cpp](https://github.com/Kistler-Group/sdbus-cpp) or using libsystemd's DBus implementation directly. 

A great **tutorial** for libsystemd and DBus in general is [here](https://0pointer.net/blog/the-new-sd-bus-api-of-systemd.html). It's written by systemd maintainer Lennart Poettering. It's a bit old and doesn't showcase advanced usage, but it gets you up and running and was very detailed/helpful even for someone who never worked with DBus before. This is also the tutorial I used myself to build this, so thanks. If you want to understand DBus better it might be worth a read as well since I won't go in depth with node descriptions here.


## Getting Started:
The [main example](project/demo/server_client_example.tscn) showcases all important functionality to create both a DBusServer and DBusClient. You'll have to look into it with the Godot Editor since most of the functionality is build into the nodes properties directly and doesn't require scripting except for setting the callbacks. 

## Nodes:

### DBusServerNode

#### Properties
* Object path
* Interface name
* A list of methods

#### DBusMethods:
To add a new method to the server you just need to append the list of methods in the editor. Here you can create a DBusMethod Resource which contains
* a member name
* a list of input arguments
* a list of output arguments

Within your script you can later access e.g. the method at index 0 and set the callback to it inside the _ready function.

```go
extends DBusServerNode

func _divide(request: DBusRequest) -> int:
	var message_signature=request.read()
	var num1:float=message_signature[0]
	var num2:float=message_signature[1]
	if num2==0:
		request.set_error("net.poettering.DivisionByZero", "Division by zero not allowed")
		return -ERR_INVALID_PARAMETER
	var response=Array()
	response.append(num1/num2)
	request.reply(response)
	return 0

func _ready() -> void:
	methods[0].callback=_divide
```

This callback will take a request which can be used to read the input arguments into a Variant Array with `read`, set errors with `set_error` and reply to the request with `reply` which takes a Variant Array as well.

Here you will also see the current limitation of the library, instead of all int types it only supports int64_t ('x') since it just maps the Godot Variant Types. All other basic types only have one version anyways so float/string/... works normally. This is probably solveable by just creating a Wrapper Resource which packs a variant and a more complete type list, but I only needed basic functionality so this isn't there yet.


### DBusClientNode

The Client in this is just directly making calls over DBus and not creating an interface or anything.

You can set destination, object_path and interface_name inside the editor. To start the client (open the bus) you then call `open`. Afterwards you can send requests to the specified interface

```go
var err=request("Divide", _on_request_finished, 50, 10)
```

`request` is a vararg method which first takes the method name, then a callback and after that all your input arguments to the method. You don't need to close the client again at the end, since it is automatically freed when it goes out of scope.

For using the client directly in GDScript you can also use the lower level `DBusClient`, which is the underlying implementation used by the node.
