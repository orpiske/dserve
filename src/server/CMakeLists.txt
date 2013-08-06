add_definitions(-DDSSERVER)

add_library(
	dscommon STATIC
	../common/options.c
	../common/messenger.c
	../common/node.c
	../common/list.c
	../common/transaction.c
	../common/util.c
	../common/network/connection.c
)

link_libraries(
	dsdownstream
	curl
)

add_library(
	dsdownstream STATIC
	downstream/controller.c
	downstream/downloadmanager.c
	downstream/queue.c
)

add_library(
	dsupstream STATIC
	upstream/server.c
)

add_executable(
	dsserver
	dsserver.c
)

target_link_libraries(
	dsserver
	dscommon
	dsdownstream
	dsupstream
)



