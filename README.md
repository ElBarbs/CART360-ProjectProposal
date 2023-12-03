# Anxious Machine

## Louis Barbier & Tania Gaboriault

### Materials

- 5m LED Strip Lights (3): 25$ each -> 75$
- MLX90640 Thermal Camera Breakout - Wide Angle (1) -> 85$
- Raspberry Pi Pico WH (2): 10$ each -> 20$
- Arduino UNO R3 (1) -> 30$
- AA Battery Holder (1) -> 5$
- AA Batteries (3) -> 7.50$
- 9V Battery (1) -> 5$
- Wooden Platform (1) [1.5m x 1.5m]
- Foil Mirror Paper (1)
- Subwoofer (1)
- Laptop (1)

### Data Workflow

1.  The first Pico WH is connected to the thermal camera and strategically positioned on the ceiling. Every 5 seconds, it captures an overhead view of the room and transmits the image data to the laptop over Wi-Fi using UDP.
2.  A dedicated Python script runs on the laptop to receive, process, and analyze the thermal imaging data. Subsequently, the script calculates an anxiety level ranging from 0 to 7. The laptop then communicates this anxiety level to the second Pico WH via OSC. This value is also transmitted to a Sonic Pi loop playing on the subwoofer, that imitates a heartbeat. The intensity of the heartbeat sound correlates with the anxiety level, with higher levels resulting in a faster heartbeat.
3.  The second Pico WH is connected to the Arduino UNO R3, which is connected to the LED strips. The Pico WH receives the anxiety level from the laptop and sends it to the UNO R3 via UART. The UNO R3 receives the data and lights up the LED strips accordingly.
