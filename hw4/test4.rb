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

def expect(socket, response)
	res = socket.recv(response.bytesize)
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
sonny = File.read("sonny1978.jpg")
ospd = File.read("ospd.txt")
s = TCPSocket.open("localhost", ARGV[0])

# Test start
test(s, "SAVE mouse.txt #{mouse.bytesize}\n#{mouse}", "ERROR FILE EXISTS\n")
test(s, "SAVE chicken.txt #{chicken.bytesize}\n#{chicken}", "ERROR FILE EXISTS\n")
test(s, "SAVE ospd.txt #{ospd.bytesize}\n#{ospd}", "ACK\n")
test(s, "READ ospd.txt 104575 26\n", "ACK 26\n")
puts "Running first 'expect'"
expect(s, "coco\ncocoa\ncocoanut\ncocoas")
test(s, "LIST\n", "5 chicken.txt legend.txt mouse.txt ospd.txt sonny1978.jpg\n")