project(eval)

add_executable(ps-benchmark main.cpp)
#add_definitions ("-ansi -Wall -std=c++0x")
add_definitions ("-std=c++14")

target_link_libraries(ps-benchmark
        peval
        penum
        ${Boost_LIBRARIES}
)
