# Phytium FreeRTOS SDK 2022-10-21 ChangeLog

Change Log sinc 2022-10-15

## example

- add e2000q example
## third-party

- Adapt fatfs to e2000 demo board for sata
- Add the lwip configuration kconfig
- Add udp multicast function

# Phytium FreeRTOS SDK 2022-10-10 ChangeLog

Change Log sinc 2022-09-23

## example

- add cherryusb host example

## third-party

- add cherryusb 0.5.2 and port to xhci

# Phytium FreeRTOS SDK 2022-09-23 ChangeLog

Change Log sinc 2022-09-15

## driver 

- add can os driver

## example

- add can test test example

# Phytium FreeRTOS SDK 2022-09-07 ChangeLog

Change Log sinc 2022-08-30

## driver 

- optimize qspi os driver adapt to sfud and spiffs

## example

- add sata controller fatfs test example

## third-party

- modify qspi sfud use qspi os driver interface functions
- add sata controller fatfs port diskio

# Phytium FreeRTOS SDK 2022-08-29 ChangeLog

Change Log sinc 2022-08-24

## driver 

- add adc driver

## example

- add adc test example

# Phytium FreeRTOS SDK 2022-08-10 0.2.2 ChangeLog

Change Log sinc 0.2.1 

## driver

- add gpio freertos driver
- add spi freertos driver
- add ddma freertos driver
- add gdma freertos driver
- add sdio freertos driver

## example

- add gpio, gdma, ddma example
- add spi nor flash, tf emmc example

## third-party

- add tlsf to implement memory pool
- add sfud to support nor flash
- add sdmmc to support tf/emmc

# Phytium FreeRTOS SDK 2022-08-18 ChangeLog

Change Log sinc 2022-08-16

## driver 

- add pwm driver

## example

- add pwm test example

# Phytium FreeRTOS SDK 2022-08-10 0.2.1 ChangeLog

Change Log sinc 0.2.0 

## README

- add E2000D/S description

# Phytium FreeRTOS SDK 2022-08-09 0.2.0 ChangeLog

Change Log sinc 2022-07-29

## driver 

- modify wdt driver

## example

- add e2000d support
- delete startup and helloworld example

## third-party

- fix qspi spiffs and sata fatfs driver
- fix shell cmd

# Phytium FreeRTOS SDK 2022-08-08 ChangeLog

Change Log sinc 2022-07-14

# driver 

- add fxmac driver

## example

- add xmac_lwip_test example

## third-party

- add fxmac ports

# Phytium FreeRTOS SDK 2022-07-26 ChangeLog

Change Log sinc 2022-07-18

# driver 

- modify wdt freertos driver

## third-party

- support qspi spiffs 
- support sata fatfs 

## example

- add usage example for qspi spiffs and sata fatfs


# Phytium FreeRTOS SDK 2022-07-14 ChangeLog

Change Log sinc 2022-07-06

# driver 

- add qspi read and write freertos driver
- add wdt freertos driver

## example

- add usage example for qspi and wdt freertos driver


# Phytium FreeRTOS SDK 2022-07-05 ChangeLog

Change Log sinc 2022-07-01

## example

- add usage example for freertos function, include task, interrupt, queue, resource,and so on.

# Phytium FreeRTOS SDK 2022-06-18 0.1.0 ChangeLog

Change Log sinc 2022-05-30

## example

- adapt to the new standalone sdk v0.2.0

- Restruct lwip_test example

## third-party

- Restruct lwip gmac adapter, add gmac port

# Phytium FreeRTOS SDK 0.0.7 ChangeLog

Change Log sinc 2022-03-21,2022-04-20

## example

- adapt to the new standalone sdk v0.1.17

- add Linux OpenAMP example for freertos

## make

- Modified some variable positions and added some configuration related variables


# Phytium FreeRTOS SDK 2022-03-21 ChangeLog

Change Log sinc 0.0.6, 2022.03.21

## example

- adapt to the new standalone sdk v0.1.16

- add OpenAMP for FreeRTOS

## third-party

- add OpenAMP for freertos application
- add Letter_shell for freertos

## LICENSE

- replace LICENSE with Phytium Public License 1.0 (PPL-1.0)
- update file COPYRIGHT declaration with PPL-1.0



# Phytium FreeRTOS SDK v0.0.6 ChangeLog

Change Log sinc 0.0.5, 2021.12.23

## example

- adapt to the new standalone sdk v0.1.15
- reconstruct the aarch framework for freertos

# Phytium FreeRTOS SDK v0.0.5 ChangeLog

Change Log sinc 0.0.4, 2021.11.2

## example

- add freertos function test examples
- change Compile environment and installation script

# Phytium FreeRTOS SDK v0.0.4 ChangeLog

Change Log sinc 0.0.3, 2021.9.24

## example

- add aarch32 example
- add aarch64 example
- add lwip_test example 

## third-party

- add lwip for freertos application
- import freertos v0.0.4 source code
- add ft20004/d2000 lwip code

# Phytium FreeRTOS SDK v0.0.1 ChangeLog

Change Log sinc init

## drivers

- port PL110 uart driver to FreeRTOS

## example

- add aarch32 example
- add aarch64 example
- add qemu-aarch32 example 

## make

- import makefile scripts
## scripts

- add arm-linux cc install script for qemu


## third-party

- add simple bootloader for qemu application
- import freertos v10.0.1 source code
- add ft20004/e2000/qemu port code
