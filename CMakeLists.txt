cmake_minimum_required(VERSION 3.10)
project(MySQLXExample)

# C++ standardını belirtin
set(CMAKE_CXX_STANDARD 17)

# MySQL Connector ve OpenSSL için include ve link yolları
include_directories(
        /opt/homebrew/opt/mysql-connector-c++/include
        /opt/homebrew/opt/openssl@3/include
)

link_directories(
        /opt/homebrew/opt/mysql-connector-c++/lib
        /opt/homebrew/opt/openssl@3/lib
)

# Çıktı dosyasını oluştur
add_executable(MySQLXExample main.cpp main.h)

# Kütüphaneleri bağlayın
target_link_libraries(MySQLXExample
        mysqlcppconnx
        ssl
        crypto
)