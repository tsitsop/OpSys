#!/usr/bin/env ruby
require 'socket'

def test(socket, command, response)
	socket.print(command)
	res = socket.gets
	res.gsub!("\n", "\\n")
	response.gsub!("\n", "\\n")
	unless(res == response)
		puts "!!! Expected \"#{response}\" but got \"#{res}\" !!!"
		exit 1
	end
end

if( ARGV.size != 1 )
	puts "USAGE: #{$0} <port number>"
	exit
end

# Setup
mouse = File.read("mouse.txt")
chicken = File.read("chicken.txt")
legend = File.read("legend.txt")
s = TCPSocket.open("localhost", ARGV[0])

# Test start
test(s, "SAVE mouse.txt #{mouse.bytesize}\n#{mouse}", "ACK\n")
test(s, "READ xyz.jpg 5555 2000\n", "ERROR NO SUCH FILE\n")
test(s, "LIST\n", "1 mouse.txt\n")