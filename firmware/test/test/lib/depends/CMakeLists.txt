## TestDepends
add_executable(TestDepends TestDepends.c)
# Test harness
target_sources(TestDepends PRIVATE ${THIRD_PARTY_DIR}/Unity/src/unity.c)
target_sources(TestDepends PRIVATE ${THIRD_PARTY_DIR}/Unity/extras/fixture/src/unity_fixture.c)
