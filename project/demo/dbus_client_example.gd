extends Node

@onready var server = $"../DBusServer"


var num1=0
var num2=0

@onready var thread: Thread=Thread.new()

func test_request():
	var client=DBusClient.new()
	
	client.destination="net.poettering.Calculator"
	client.path="/net/poettering/Calculator"
	client.interface="net.poettering.Calculator"
	
	var member="Multiply"
	client.open()
	
	var request_message=client.create_request(member)
	request_message.append(num1)
	request_message.append(num2)
	var response_message=client.send_request(request_message)
	if response_message==null:
		print("failed request")
		return
	print(response_message.read_single(TYPE_INT))
	
	client.close()


func _on_timer_timeout():
	num1+=1
	num2+=2
	
	#since same process: server and request are blocking 
	#need to use thread here, normally you can use this without them
	if(thread.is_started()):
		thread.wait_to_finish()
	thread.start(test_request)
