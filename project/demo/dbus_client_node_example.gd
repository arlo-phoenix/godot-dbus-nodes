extends DBusClientNode


func _on_request_finished(response: DBusMessage):
	assert(response)
	print(response.read_single(TYPE_FLOAT))

func _on_timer_timeout():
	open()
	var err=request("Divide", _on_request_finished, 50, 10)
