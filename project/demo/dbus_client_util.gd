extends DBusClientNode

func _ready():
	open()
	#godots XMLParser is horrible so if you want to display this use some C++ library or plugin
	print(introspect())
	
	# if you have a parser would need to call this down the tree
	# to replicate busctl tree <destination>
	print(introspect_path("/org"))
	
	close()
