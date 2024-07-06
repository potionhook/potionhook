# Using combined sources in order to support macOS / iOS universal builds
# It makes it super easy to extend to new platforms too
set(context_srcs
  ${CMAKE_CURRENT_LIST_DIR}/context/jump_combined.S
  ${CMAKE_CURRENT_LIST_DIR}/context/make_combined.S
  ${CMAKE_CURRENT_LIST_DIR}/context/ontop_combined.S
  ${BOOST_SOURCE}/libs/context/src/posix/stack_traits.cpp
)
enable_language(ASM)

_add_boost_lib(
  NAME context
  SOURCES
    ${context_srcs}
  DEFINE_PRIVATE
    BOOST_CONTEXT_SOURCE=1
    BOOST_CONTEXT_EXPORT
  LINK
    Boost::thread
)

_add_boost_test(
  NAME context_test
  LINK
    Boost::context
    Boost::unit_test_framework
  TESTS
    RUN ${BOOST_SOURCE}/libs/context/test/test_invoke.cpp
    RUN ${BOOST_SOURCE}/libs/context/test/test_apply.cpp
    RUN ${BOOST_SOURCE}/libs/context/test/test_fcontext.cpp
    RUN ${BOOST_SOURCE}/libs/context/test/test_fiber.cpp
    RUN ${BOOST_SOURCE}/libs/context/test/test_callcc.cpp
    RUN ${BOOST_SOURCE}/libs/context/test/test_execution_context_v2.cpp
)
