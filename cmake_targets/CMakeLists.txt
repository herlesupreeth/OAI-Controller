cmake_minimum_required (VERSION 2.6)

project (Controller)

set (CONTROLLER_DIR     $ENV{CONTROLLER_DIR})
set (CONTROLLER_CMAKE   ${CONTROLLER_DIR}/cmake_targets)
set (CONTROLLER_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY})

set(protoc_call "${CONTROLLER_CMAKE}/tools/generate_protobuf")

if (NOT EXISTS "${CONTROLLER_BIN_DIR}/MESSAGES")
	file(MAKE_DIRECTORY "${CONTROLLER_BIN_DIR}/MESSAGES")
endif (NOT EXISTS "${CONTROLLER_BIN_DIR}/MESSAGES")

set(protobuf_generated_dir ${CONTROLLER_BIN_DIR}/MESSAGES)
  
set(PRPT_MSG_DIR ${CONTROLLER_DIR}/MESSAGES)
set(PRPT_MSG_FILES
${PRPT_MSG_DIR}/header.proto
${PRPT_MSG_DIR}/progran.proto
${PRPT_MSG_DIR}/stats_common.proto
${PRPT_MSG_DIR}/stats_messages.proto
${PRPT_MSG_DIR}/time_common.proto
${PRPT_MSG_DIR}/controller_commands.proto
${PRPT_MSG_DIR}/mac_primitives.proto
${PRPT_MSG_DIR}/config_messages.proto
${PRPT_MSG_DIR}/config_common.proto
${PRPT_MSG_DIR}/control_delegation.proto
)

set(PRPT_C_DIR ${protobuf_generated_dir})
#message("calling protoc_call=${protoc_call} PRPT_C_DIR=${PRPT_C_DIR} PRPT_MSG_FILES=${PRPT_MSG_FILES}")
execute_process(COMMAND ${protoc_call} ${PRPT_C_DIR} ${PRPT_MSG_DIR} ${PRPT_MSG_FILES})
file(GLOB PRPT_source ${PRPT_C_DIR}/*.c)
set(PRPT_OAI_generated
${PRPT_C_DIR}/header.pb-c.c
${PRPT_C_DIR}/progran.pb-c.c
${PRPT_C_DIR}/stats_common.pb-c.c
${PRPT_C_DIR}/stats_messages.pb-c.c
${PRPT_C_DIR}/time_common.pb-c.c
${PRPT_C_DIR}/controller_commands.pb-c.c
${PRPT_C_DIR}/mac_primitives.pb-c.c
${PRPT_C_DIR}/config_messages.pb-c.c
${PRPT_C_DIR}/config_common.pb-c.c
${PRPT_C_DIR}/control_delegation.pb-c.c
)

file(GLOB prpt_h ${PRPT_C_DIR}/*.h)
set(prpt_h ${prpt_h} )

add_library(PRPT_MSG
${PRPT_OAI_generated}
${PRPT_source}
)
set(PRPT_MSG_LIB PRPT_MSG)
message("prpt c dir is : ${PRPT_C_DIR}")
include_directories (${PRPT_C_DIR})

add_library(ASYNC_IF
${CONTROLLER_DIR}/ASYNC_IF/socket_link.c
${CONTROLLER_DIR}/ASYNC_IF/link_manager.c
${CONTROLLER_DIR}/ASYNC_IF/message_queue.c
)
set(ASYNC_IF_LIB ASYNC_IF)
include_directories(${CONTROLLER_DIR}/ASYNC_IF)

add_library(CONTROLLER_AGENT
${CONTROLLER_DIR}/controller_agent_handler.c
${CONTROLLER_DIR}/controller_agent_common.c
${CONTROLLER_DIR}/controller_agent_mac.c
${CONTROLLER_DIR}/controller_agent.c
${CONTROLLER_DIR}/controller_agent_task_manager.c
${CONTROLLER_DIR}/controller_agent_net_comm.c
${CONTROLLER_DIR}/controller_agent_async.c
${CONTROLLER_DIR}/controller_agent_mac_internal.c
)

set(CONTROLLER_AGENT_LIB CONTROLLER_AGENT)
#include_directories(${CONTROLLER_DIR})

set (CONTROLLER_UTIL   ${CONTROLLER_DIR}/UTIL)

add_library(LOGGING
${CONTROLLER_UTIL}/LOG/log.c
${CONTROLLER_UTIL}/LOG/backtrace.c
)
##${CONTROLLER_UTIL}/LOG/vcd_signal_dumper.c

include_directories("${CONTROLLER_UTIL}/LOG")
include_directories("${CONTROLLER_UTIL}")
include_directories("${CONTROLLER_UTIL}/COLLECTION")

set(PROTOBUF_LIB "protobuf-c")

#set(PROTOBUF_LIB "protobuf") #for Cpp

#################################
# add executables for operation
#################################

# controller implementation
###################################################

add_executable(controller
  ${CONTROLLER_DIR}/controller-main.c
  )

target_link_libraries (controller
  -Wl,--start-group
   LOGGING ${PRPT_MSG_LIB} ${ASYNC_IF_LIB} ${CONTROLLER_AGENT_LIB} 
  -Wl,--end-group )

target_link_libraries (controller ${LIBXML2_LIBRARIES})
target_link_libraries (controller pthread protobuf-c m ${CONFIG_LIBRARIES} ${PROTOBUF_LIB})
target_link_libraries (controller ${LIBBOOST_LIBRARIES})
target_link_libraries (controller ${LIB_LMS_LIBRARIES})
