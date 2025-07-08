cmake_minimum_required(VERSION 3.15)
project(InfernoBot VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add libraries
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)

# Add tgbot-cpp from source if not available as package
include(FetchContent)
FetchContent_Declare(
  tgbot
  GIT_REPOSITORY https://github.com/reo7sp/tgbot-cpp.git
  GIT_TAG v1.2.1
)
FetchContent_MakeAvailable(tgbot)

# Include nlohmann/json as header-only library
FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
file(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Create executable
add_executable(inferno_bot ${SRC_FILES})

# Link libraries
target_link_libraries(inferno_bot
    PRIVATE
    TgBot
    nlohmann_json::nlohmann_json
    Threads::Threads
    ${CURL_LIBRARIES}
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)

target_include_directories(inferno_bot
    PRIVATE
    ${CURL_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
) 