# Reflex Firmware
Andorinha drone Mini Gimbal Firmware

## Target Hardware: PlumaN6 Gimbal
Reflex is currently being developed for a custom **PlumaN6 Gimbal** FOC BLDC controller board. This board features:
- **Core:** STM32C092 (Cortex-M0 @ 48MHz)
- **BLDC Driver:** The TMC6460, an all new fully integrated FOC controller from Analog Devices
- **Axes:** Full 3-axis control: Pitch, Yaw, and Roll
- **Feedback:** Either trough IMU or encoder on each axis
- **Connectivity:** Dual I2C (one for control, one to IMU) and UART (on debug connector)

Full specifications and details can be found on the board project TBD
