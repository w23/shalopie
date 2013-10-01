import sublime, sublime_plugin, socket

UDP_IP = "127.0.0.1"
UDP_PORT = 5470

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

class SourcesSender(sublime_plugin.EventListener):
	def on_modified(self, view):
		source = view.substr(sublime.Region(0, view.size()))
		sent = sock.sendto(source, (UDP_IP, UDP_PORT)) 
		sublime.status_message("sent: " + str(sent))