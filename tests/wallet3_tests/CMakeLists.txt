

add_executable(wallet3_tests
  db_schema.cpp
  scan_received.cpp
  sign.cpp
  verify.cpp
  decoy.cpp
  main.cpp
)

target_link_libraries(wallet3_tests
  PRIVATE
  Catch2::Catch2
  SQLiteCpp
  ringct
  wallet3)


add_test(
  NAME    wallet3_tests
  COMMAND wallet3_tests)
