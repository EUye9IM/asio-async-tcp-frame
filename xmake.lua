set_warnings("all", "error")
set_languages("c11")

if is_os("windows") then
	add_cxflags("-fexec-charset=gb18030")
end

add_includedirs("include")
add_includedirs("asio")

if is_os("windows") then
	add_defines("_WIN32_WINNT=0x0601")
	add_links("ws2_32")
	add_links("Bcrypt")
	add_links("wsock32")
end

add_files("src/**.cpp")
add_defines("ASIO_STANDALONE")
add_defines("ASIO_SEPARATE_COMPILATION")

target("server")
	set_kind("binary")
	add_files("example/server.cpp")
target_end()

target("client")
	add_files("example/client.cpp")
target_end()
