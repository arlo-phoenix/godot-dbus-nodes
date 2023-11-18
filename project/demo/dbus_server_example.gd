extends DBusServerNode

@onready var client=$"../DBusClient"

func _multiply(message: DBusResponse) -> int:
	var message_signature=message.read()
	var response=Array()
	response.append(message_signature[0] * message_signature[1])
	message.reply(response)
	return 0
	
func _empty(message: DBusResponse) -> int:
	var response=Array()
	print("do shit")
	response.append(true)
	message.reply(response)
	return 0
	
func _divide(message: DBusResponse) -> int:
	var message_signature=message.read()
	var num1:float=message_signature[0]
	var num2:float=message_signature[1]
	if num2==0:
		message.set_error("net.poettering.DivisionByZero", "Division by zero not allowed")
		return -ERR_INVALID_PARAMETER
	var response=Array()
	response.append(num1/num2)
	message.reply(response)
	return 0

func _ready() -> void:
	methods[0].callback=_multiply
	methods[1].callback=_empty
	methods[2].callback=_divide

