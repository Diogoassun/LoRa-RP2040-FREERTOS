PWD := /home/yuno/UFC/embarcados/freertos-pico

SRC_DIR := $(PWD)/LoRa
BUILD_DIR := $(SRC_DIR)/build


export PICO_SDK_PATH := $(PWD)/pico-sdk
export FREERTOS_KERNEL_PATH := $(PWD)/FreeRTOS-Kernel

all: build

build:
	@echo "Configurando o projeto com cmake..."
	@rm -rf $(BUILD_DIR)/* 
	@cmake -S $(SRC_DIR) -B $(BUILD_DIR)
	@make -C $(BUILD_DIR)

clean:
	@echo "Limpando o diretório de build..."
	@rm -rf $(BUILD_DIR)/*  # Limpa os arquivos de build

.PHONY: all build clean
