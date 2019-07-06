include_directories(BEFORE .)
file(GLOB_RECURSE fancontrol2_SOURCES "*.cpp")
add_executable(fancontrol2 ${fancontrol2_SOURCES})
target_link_libraries(fancontrol2 sensors boost_filesystem boost_system yaml-cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
-DBOOST_DISABLE_THREADS -DBOOST_SP_DISABLE_THREADS \
-pipe -std=c++11 -Wall -Wextra -Wconversion -Wstrict-overflow=3 \
-Wno-missing-field-initializers -fmessage-length=0 -fmax-errors=10 \
-Wno-unused-local-typedefs")
set(CMAKE_CXX_FLAGS_DEBUG
	"${CMAKE_CXX_FLAGS_DEBUG} -O0 -DFANCONTROL_PIDFILE_ROOTONLY=0")

set(C_OPTIM_FLAGS_RELEASE "-flto -ffast-math")
set(CMAKE_CXX_FLAGS_RELEASE
	"${CMAKE_CXX_FLAGS_RELEASE} ${C_OPTIM_FLAGS_RELEASE} -frepo")
set(CMAKE_LINK_FLAGS_RELEASE
	"${CMAKE_CXX_FLAGS_RELEASE} ${C_OPTIM_FLAGS_RELEASE}")

install(TARGETS fancontrol2 RUNTIME DESTINATION sbin)
