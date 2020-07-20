  cmake_minimum_required(VERSION 3.5.2)
project(code)

set(CMAKE_CXX_STANDARD 14)

add_executable(code main.cpp Instruction.hpp exceptions.hpp Add_Reservation.hpp Load_Buffer.hpp Re-order_Buffer.hpp Prediction.hpp RISC-V_Tomasulo.hpp)