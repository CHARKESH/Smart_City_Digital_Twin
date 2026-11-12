🏙️ Smart City Digital Twin Project

<img width="720" height="1600" alt="image" src="https://github.com/user-attachments/assets/cf7aef28-cd6e-49c6-a1a0-0586a23548a8" />


📱 Unity 3D City Visualization

<img width="1600" height="899" alt="image" src="https://github.com/user-attachments/assets/ede075a1-74cd-4d3c-a7c6-38ddca7a0ae3" />


🔌 Circuit Connection

<img width="720" height="1600" alt="image" src="https://github.com/user-attachments/assets/0b2bbf88-46fc-4020-964f-60ac53a0d90b" />

📋 Project Overview

Smart City Digital Twin using ESP32, sensors, ThingSpeak cloud, and Unity 3D visualization.

🔧 Components Used

- ESP32 Development Board
- DHT22 Temperature/Humidity Sensor
- HC-SR04 Ultrasonic Sensor
- IR Proximity Sensor
- LDR Light Sensor
- SG90 Servo Motor
- OLED Display (128x64)
- LED
- Breadboard and Jumper Wires

🔌 Circuit Connections

| ESP32 Pin | Component | Connection |
|-----------|-----------|------------|
| GPIO15 | DHT22 Data | Temperature/Humidity |
| GPIO5 | Ultrasonic Trig | Traffic Distance |
| GPIO18 | Ultrasonic Echo | Traffic Distance |
| GPIO4 | IR Sensor | Parking Detection |
| GPIO27 | LDR Digital OUT | Light Level |
| GPIO21 | OLED SDA | Display Data |
| GPIO22 | OLED SCL | Display Clock |
| GPIO13 | Servo Signal | Gate Control |
| GPIO25 | LED Anode | Street Light |
| 3V3 | DHT22 VCC, IR VCC, LDR VCC, OLED VCC | Power |
| GND | All Component GND | Common Ground |
| VIN (5V) | Ultrasonic VCC, Servo VCC | Power |

📊 ThingSpeak Fields

| Field | Data Type | Description |
|-------|-----------|-------------|
| Field 1 | Traffic Distance | Ultrasonic sensor reading (cm) |
| Field 2 | Parking Status | IR sensor (0=Free, 1=Full) |
| Field 3 | Temperature | DHT22 reading (°C) |
| Field 4 | Light Level | LDR reading (%) |
| Field 5 | Message Length | Morse code message length |

🚀 Features

- Real-time traffic monitoring
- Smart parking detection
- Weather monitoring
- Automatic street lighting
- Mobile web control
- Morse code messaging
- Unity 3D visualization
- ThingSpeak cloud integration

📱 Mobile Control

Access web control panel: http://[ESP32_IP_ADDRESS]

- Turn street lights ON/OFF
- Send messages to OLED display
- View live sensor data
- Morse code via phone flashlight

🎮 Unity Integration

Unity 3D city updates in real-time:
- Street lights dim/brighten with LDR
- Traffic gates open/close with ultrasonic
- Parking spots change color with IR
- Live sensor data on dashboard

👨‍💻 Author

CHARKESH S

📄 License

MIT License
