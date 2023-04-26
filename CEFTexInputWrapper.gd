extends CenterContainer


# Declare member variables here. Examples:
# var a = 2
# var b = "text"

onready var cef = get_node("CEFTex")
# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass


func _on_CEFTexInputWrapper_gui_input(event):
	# print(event.as_text())
	if event is InputEventMouseMotion:
		cef.simulate_motion(event.position.x, event.position.y, event.relative.x, event.relative.y, event.button_mask)
	if event is InputEventMouseButton and event.button_index == BUTTON_LEFT:
		cef.simulate_click(event.position.x, event.position.y, event.pressed)
	
	if event is InputEventKey:
		cef.simulate_key(event.physical_scancode, event.pressed)
	
	pass # Replace with function body.

func _on_Control_gui_input(event):
	if event is InputEventMouseButton and event.pressed:
		cef.go_back()
	pass # Replace with function body.
