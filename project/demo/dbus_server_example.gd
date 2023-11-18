extends DBusServerNode

@onready var client=$"../DBusClient"

func _multiply(request: DBusRequest) -> int:
	var message_signature=request.read()
	var response=Array()
	response.append(message_signature[0] * message_signature[1])
	request.reply(response)
	return 0
	
func _empty(request: DBusRequest) -> int:
	var response=Array()
	print("do sth")
	response.append(true)
	request.reply(response)
	return 0
	
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
	methods[0].callback=_multiply
	methods[1].callback=_empty
	methods[2].callback=_divide

