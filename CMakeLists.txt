# cmake_minimum_required(VERSION 3.13)

# # Importa o SDK da Raspberry Pi Pico
# include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# # Importa o FreeRTOS
# include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

# # Nome do projeto
# project(app C CXX ASM)
# set(CMAKE_C_STANDARD 11)
# set(CMAKE_CXX_STANDARD 17)

# # Inicializa o Pico SDK
# pico_sdk_init()

# # Adiciona o executável principal
# add_executable(main
#     src/main.cpp     
# )

# # Adiciona os diretórios de include
# target_include_directories(main PRIVATE
#     ${CMAKE_CURRENT_LIST_DIR}
#     ${CMAKE_CURRENT_LIST_DIR}/includes  # Diretório dos cabeçalhos
#     ${CMAKE_CURRENT_LIST_DIR}/drivers  # Diretório com implementações dos drivers
#     ${CMAKE_CURRENT_LIST_DIR}/handlers # Diretório com implementações dos handlers
# )

# add_library(freertos_config INTERFACE)

# # Vincula as bibliotecas necessárias
# target_link_libraries(main
#     pico_stdlib               # Biblioteca padrão do Pico SDK
#     hardware_i2c              # Biblioteca de hardware I2C do Pico SDK
#     hardware_pwm
#     hardware_adc
#     FreeRTOS-Kernel  
#     freertos_config         # Núcleo do FreeRTOS
#     FreeRTOS-Kernel-Heap4     # Gerenciador de heap do FreeRTOS
# )

# pico_enable_stdio_usb(main 1)
# pico_enable_stdio_uart(main 1)

# # Gera arquivos extras (UF2, bin, map, etc.)
# pico_add_extra_outputs(main)

# # add_custom_command(TARGET main POST_BUILD
# #         COMMAND ${CMAKE_COMMAND} -E copy
# #         main.uf2
# #         /media/$ENV{USER}/RPI-RP2/
# #         COMMENT "Copiando motorProject.uf2 para a Raspberry Pi Pico"
# # )

cmake_minimum_required(VERSION 3.13)

# Importa o SDK da Raspberry Pi Pico
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Importa o FreeRTOS
include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(app C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Inicializa o Pico SDK
pico_sdk_init()

# Adiciona bibliotecas
add_library(freertos_config INTERFACE)

add_library(LoRa_pico_lib src/LoRa-RP2040.cpp)
add_library(LoRa_print src/Print.cpp)
add_library(Pluviometro src/pluviometro.cpp)

# Inclui diretórios de include para as bibliotecas
target_include_directories(LoRa_pico_lib PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

target_include_directories(LoRa_print PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

target_include_directories(Pluviometro PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

# Inclui o diretório raiz para freertos_config para que o compilador encontre FreeRTOSConfig.h
target_include_directories(freertos_config INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Linka bibliotecas necessárias nas libs
target_link_libraries(LoRa_pico_lib
    pico_stdlib
    hardware_spi
    hardware_i2c
    hardware_pwm
    hardware_adc
)

target_link_libraries(LoRa_print
    pico_stdlib
)

target_link_libraries(Pluviometro
    pico_stdlib
    hardware_pwm
    hardware_gpio
)

# Adiciona executável principal
add_executable(main src/main.cpp)

# Inclui diretório src para o main (onde estão os headers)
target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}  
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/includes  # Diretório dos cabeçalhos
    ${CMAKE_CURRENT_LIST_DIR}/drivers  # Diretório com implementações dos drivers
    ${CMAKE_CURRENT_LIST_DIR}/handlers # Diretório com implementações dos handlers         
)

# Linka as libs e SDK ao executável
target_link_libraries(main
    LoRa_pico_lib
    LoRa_print
    Pluviometro
    pico_stdlib
    hardware_spi
    hardware_interp
    hardware_i2c
    hardware_pwm
    hardware_adc
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
    freertos_config          # importante para encontrar FreeRTOSConfig.h
)

# Habilita stdio via USB e UART
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 1)

# Gera arquivos extras
pico_add_extra_outputs(main)
