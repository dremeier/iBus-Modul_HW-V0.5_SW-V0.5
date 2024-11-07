# SARA-R5 Project with Sparkfun and Custom iBusTrx Library

This project involves using the u-blox SARA-R5 module for an LTE connection, integrating GPS data, NeoPixel LED control, MQTT communication with SSL/TLS for secure data transfer to ioBroker, and interaction with the iBus system. Additionally, Bluetooth functionalities and multiple timers are included to handle various tasks.

## Overview
This code connects a BMW E38 car to a cloud-based MQTT server using an LTE module, providing real-time telemetry, GPS data, and interactive vehicle controls. The project uses:
- SparkFun u-blox SARA-R5 Library
- Custom-modified iBusTrx Library for iBus communication
- NeoPixel LEDs for status indication
- Custom timer management to handle multiple intervals without blocking
- EEPROM storage for preserving settings
- Bluetooth control integration

## Features
- **MQTT Connection:** Secure MQTT 3.11 connection with TLS to ioBroker.
- **GPS Data Processing:** Captures and publishes vehicle location, speed, and orientation.
- **NeoPixel LED Indicators:** Three LEDs provide real-time system status for network, MQTT, and GPS.
- **Custom iBus Commands:** Allows sending commands to the vehicle's iBus for various functionalities.
- **Bluetooth Interaction:** Controls and retrieves Bluetooth module data.
- **EEPROM Storage:** Saves GPS distance, angle, and other settings persistently.
- **Timers:** Multiple non-blocking timers manage polling intervals for different data sources.

## Hardware Version
- Version 0.5
- Author: AME
- Date: 01/11/2024

## Dependencies
- [SparkFun u-blox SARA-R5 Arduino Library](http://librarymanager/All#SparkFun_u-blox_SARA-R5_Arduino_Library)
- Custom-modified iBusTrx Library for enhanced iBus support

## Setup
1. **Neopixel LEDs:** Initializes three LEDs for system status indication.
2. **PIN Configuration:** Sets up power pins, UART pins, and power-saving pins for the SARA-R5 and iBus communication.
3. **Bluetooth Initialization:** Configures Bluetooth settings for remote control interaction.
4. **SARA-R5 LTE Connection:** Attempts to connect to a network provider and establish a secure MQTT connection with ioBroker.
5. **EEPROM Initialization:** Reads and writes essential data for system parameters.

## Usage
### Core Functionalities
- **MQTT Connection and Data Publishing:** Establishes a secure connection to an MQTT broker, sending vehicle telemetry data, GPS position, and status updates.
- **GPS Data Processing:** Polls GPS data at regular intervals, calculates distance and angle changes, and publishes the information.
- **iBus Messaging:** Controls various vehicle components via iBus messages, such as central locking, mirror folding, and ambient light control.
- **Bluetooth Control:** Interacts with a Bluetooth module, sending commands and receiving playback data.
- **Standheizung Timer:** Manages the car’s auxiliary heater based on time intervals or remote commands.

### Important Commands
- **Power Saving and Connection Settings:** AT commands manage power-saving modes and keep-alive settings to ensure stable LTE connectivity.
- **iBus Custom Commands:** Control functions like locking/unlocking, mirror folding, and ambient light activation.
- **MQTT Topic Subscriptions:** Dynamically subscribes to MQTT topics, handles data processing, and updates car settings based on received commands.

### Code Structure
- **`setup()`**: Initializes hardware components, connects to LTE, sets up MQTT, and subscribes to relevant topics.
- **`loop()`**: Regularly checks for data from sensors, processes iBus messages, and updates system states.

### Key Components and Files
- **`sara-lte.h`**: Manages LTE communication and MQTT connection.
- **`globals.h`**: Stores global variables and settings.
- **`mqtt_secrets.h`**: Keeps MQTT broker details securely.
- **`neopixel.h`**: Handles LED display status.
- **`iBusTrx`**: Custom library for enhanced iBus functionality.

## License
MIT License - Please refer to the LICENSE file for more details.
