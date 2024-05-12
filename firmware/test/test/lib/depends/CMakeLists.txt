## TestDepends
add_executable(TestDepends TestDepends.c)
# Test harness
target_sources(TestDepends PRIVATE ${PROJECT_SOURCE_DIR}/Unity/src/unity.c)
target_sources(TestDepends PRIVATE ${PROJECT_SOURCE_DIR}/Unity/extras/fixture/src/unity_fixture.c)
