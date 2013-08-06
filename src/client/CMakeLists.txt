add_definitions(-DDSCLIENT)

add_library(
	dscli SHARED
	../common/network/connection.c
	../common/transaction.c
	../common/messenger.c
	../common/options.c
	../common/util.c
)

add_executable(
	dsclient
	dsclient.c
)

target_link_libraries(
	dsclient
	dscli
) 

