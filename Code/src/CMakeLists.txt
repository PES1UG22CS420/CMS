cmake_minimum_required(VERSION 3.16)
project(CrisisManagementSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find POCO package
find_package(Poco REQUIRED Foundation Net Util JSON Data DataSQLite)

# Include directories
include_directories(${Poco_INCLUDE_DIRS})

# Add all source files
file(GLOB SOURCES 
    "*.cpp"
    "models/*.cpp"
    "controllers/*.cpp"
    "api/*.cpp"
    "database/*.cpp"
)

# Create the executable
add_executable(CrisisManagementSystem ${SOURCES})

# Link against POCO libraries
target_link_libraries(CrisisManagementSystem 
    Poco::Foundation
    Poco::Net
    Poco::Util
    Poco::JSON
    Poco::Data
    Poco::DataSQLite
)