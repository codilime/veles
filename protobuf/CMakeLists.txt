PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS network.proto)
PROTOBUF_GENERATE_PYTHON(PROTO_PY network.proto)

add_library(veles_network
    ${INCLUDE_DIR}/network/server.h
    ${PROTO_SRCS}
    ${SRC_DIR}/network/server.cc
    ${PROTO_HDRS}
# python file will only be build if something depends on it
    ${PROTO_PY}
)

add_dependencies(veles_network protobuf)
qt5_use_modules(veles_network Core Network Widgets)

target_link_libraries(veles_network veles_base)
# Windows is too dumb to select correct version on its own
if(WIN32)
  target_link_libraries(veles_network $<$<CONFIG:Debug>:${PROTOBUF_LIBRARY_DEBUG}> $<$<NOT:$<CONFIG:Debug>>:${PROTOBUF_LIBRARY}>)
else(WIN32)
  target_link_libraries(veles_network ${PROTOBUF_LIBRARIES})
endif(WIN32)
