add_executable(bench_parser bench_parser.cpp)
target_link_libraries(bench_parser PRIVATE collector_objects benchmark::benchmark)

add_executable(bench_orderbook bench_orderbook.cpp)
target_link_libraries(bench_orderbook PRIVATE collector_objects benchmark::benchmark)

add_executable(bench_queue bench_queue.cpp)
target_link_libraries(bench_queue PRIVATE collector_objects benchmark::benchmark)
