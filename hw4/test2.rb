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
s = TCPSocket.open("localhost", ARGV[0])

# Test start
test(s, "SAVE mouse.txt #{mouse.bytesize}\n#{mouse}", "ERROR FILE EXISTS\n")
test(s, "SAVE legend.txt #{legend.bytesize}\n#{legend}", "ACK\n")
test(s, "SAVE chicken.txt #{chicken.bytesize}\n#{chicken}", "ACK\n")
test(s, "LIST\n", "3 chicken.txt legend.txt mouse.txt\n")
test(s, "READ chicken.txt 4 5\n", "ACK 5\n")
puts "Running first 'expect'"
expect(s, "quick")
test(s, "READ legend.txt 50092 39\n", "ACK 39\n")
puts "Running second 'expect'"
expect(s, "broken rocks and trunks of fallen trees")