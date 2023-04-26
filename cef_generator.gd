extends Node

var sample_hz = 22050.0 # Keep the number of samples to mix low, GDScript is not super fast.

var playback: AudioStreamPlayback = null # Actual playback stream, assigned in _ready().

onready var CEFTex = get_tree().root.get_node("Control/CEFTexInputWrapper/CEFTex")

func _fill_buffer():
	var to_fill = playback.get_frames_available()
	var cef_smx = CEFTex.grab_audio_samples(to_fill)
	playback.push_buffer(cef_smx)
	

func _process(_delta):
	_fill_buffer()


func _ready():
	$AudioStreamPlayer.stream.mix_rate = sample_hz # Setting mix rate is only possible before play().
	$AudioStreamPlayer.stream.buffer_length = 0.072 # 72ms delay 
	playback = $AudioStreamPlayer.get_stream_playback()
	_fill_buffer() # Prefill, do before play() to avoid delay.
	$AudioStreamPlayer.play()
