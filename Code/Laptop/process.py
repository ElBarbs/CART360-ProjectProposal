import math
import numpy as np
import socket
from pythonosc import udp_client

UDP_IP_IN = "0.0.0.0"  # Listen on all available network interfaces.
UDP_PORT_IN = 51689   # Port number to listen on.
IP_PICO = "192.168.130.180"
PORT_PICO = 51688  # Port number to send to.
IP_SONICPI = "192.168.56.1"
PORT_SONICPI = 4560

THRESHOLD_MULTIPLIER = 1.5

# Create an empty list to store the threshold values.
tresholds = []

# Create a UDP socket.
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP_IN, UDP_PORT_IN))

udp_client_pico = udp_client.SimpleUDPClient(IP_PICO, PORT_PICO)
udp_client_sonicpi = udp_client.SimpleUDPClient(IP_SONICPI, PORT_SONICPI)

try:
    while sock.fileno() != -1:
        data, addr = sock.recvfrom(10000)  # Adjust the buffer size as needed

        # 'data' contains the received UDP message.
        # 192 readings from the thermal camera.
        data_str = data.decode('utf-8')

        # Convert the string of readings into a list of floating point values.
        thermal_readings_array = [float(reading)
                                  for reading in data_str.split(',')]

        # Reshape the list of readings into a 12x16 grid.
        thermal_readings_grid = np.array(thermal_readings_array).reshape(
            16, 12).astype(np.uint8)

        # Calculate the mean and standard deviation of the temperature readings
        mean_temperature = np.mean(thermal_readings_array)
        std_temperature = np.std(thermal_readings_array)

        # Set the threshold based on the mean and a multiplier (you can adjust this multiplier)
        threshold_temperature = mean_temperature

        if len(tresholds) < 5:
            tresholds.append(threshold_temperature)
        else:
            threshold_temperature = np.mean(
                tresholds) + THRESHOLD_MULTIPLIER * np.std(tresholds)

        # Find the positions where the temperature exceeds the threshold
        higher_temperature_positions = np.where(
            thermal_readings_grid > threshold_temperature)

        # Calculate the percentage of positions with higher temperature
        # Map the percentage to a range of 0-7
        anxiety_index = math.ceil(
            (len(higher_temperature_positions[0]) / 192) * 7)

        # Send the anxiety index via OSC.
        # udp_client_pico.send_message("/anxiety", anxiety_index)
        udp_client_sonicpi.send_message("/anxiety", anxiety_index)

        # Print the grid with 'x' marking cells with higher temperature
        for row in range(16):
            for col in range(12):
                if thermal_readings_grid[row, col] > threshold_temperature:
                    print('x', end=' ')
                else:
                    print('.', end=' ')
            print()  # Move to the next line for the next row
        print(len(tresholds))


except KeyboardInterrupt:
    print("Server stopped by the user.")

finally:
    sock.close()
