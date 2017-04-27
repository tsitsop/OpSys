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
s = TCPSocket.open("localhost", ARGV[0])

# Test start
test(s, "SAVE chicken.txt #{chicken.bytesize}\n#{chicken}", "ERROR FILE EXISTS\n")
test(s, "SAVE sonny1978.jpg #{sonny.bytesize}\n#{sonny}", "ACK\n")
test(s, "LIST\n", "4 chicken.txt legend.txt mouse.txt sonny1978.jpg\n")
test(s, "READ sonny1978.jpg 920 11\n", "ACK 11\n")
puts "Running first 'expect'"
expect(s, "Cocoa Puffs")
test(s, "READ sonny1978.jpg 95898 3\n", "ACK 3\n")
puts "Running second 'expect'"
expect(s, "Yum")