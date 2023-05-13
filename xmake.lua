set_warnings("all", "error")
set_languages("c++17")

add_rules("mode.release", "mode.debug")

add_includedirs("include")
add_includedirs("asio")

if is_os("windows") then
	add_defines("_WIN32_WINNT=0x0601")
	add_links("ws2_32", "Bcrypt", "wsock32")
end

add_files("src/**.cpp")
add_defines("ASIO_STANDALONE")
add_defines("ASIO_SEPARATE_COMPILATION")

target("server-tcp")
	set_kind("binary")
	add_files("example/server_tcp.cpp")
target_end()

target("client-tcp")
	add_files("example/client_tcp.cpp")
target_end()

target("server-udp")
	set_kind("binary")
	add_files("example/server_udp.cpp")
target_end()

target("client-udp")
	add_files("example/client_udp.cpp")
target_end()
