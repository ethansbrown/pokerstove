project(sim)

add_executable(ps-sim main.cpp)
#add_definitions ("-ansi -Wall -std=c++0x")
add_definitions ("-std=c++0x")

target_link_libraries(ps-sim
        peval
        penum
        ${Boost_LIBRARIES}
)
