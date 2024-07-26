import serial
import json
from math import cos, sin, pi
import matplotlib.pyplot as plt
import numpy as np
from mpl_toolkits.mplot3d import Axes3D

from datetime import datetime

# datetime object containing current date and time
now = datetime.now()
 
print("now =", now)

# dd/mm/YY H:M:S
dt_string = now.strftime("%d/%m/%Y %H:%M:%S")

printed = False

def plot_lines(coordinates):
    x_values, y_values = zip(*coordinates)
    plt.plot(x_values, y_values, color='blue', linestyle='-', linewidth=2, label='Line')

    plt.title('Graph with Connected Line (No Dots)')
    plt.xlabel('X-axis')
    plt.ylabel('Y-axis')

    # Set axis limits to always be 0 to 300
    #plt.xlim(0, 300)
    #plt.ylim(0, 300)

    # Set the aspect ratio to be equal
    plt.gca().set_aspect('equal', adjustable='box')

    plt.grid(True)
    plt.legend()
    plt.show()

def plot_dots(coordinates):
    x_values, y_values = zip(*coordinates)
    plt.scatter(x_values, y_values, color='blue', marker='o')
    plt.title('Graph with Dots')
    plt.xlabel('X-axis')
    plt.ylabel('Y-axis')

    #plt.xlim(0, 300)
    #plt.ylim(0, 300)

    plt.gca().set_aspect('equal', adjustable='box')

    plt.grid(True)
    plt.show()


def plot_3d_surface(coordinates):
    x_coords = [coordinate[0] for coordinate in coordinates]
    y_coords = [coordinate[1] for coordinate in coordinates]
    z_coords = [coordinate[2] for coordinate in coordinates]

    # Convert coordinates to numpy arrays
    x = np.array(x_coords)
    y = np.array(y_coords)
    z = np.array(z_coords)

    # Create a 3D plot
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    # Plot the scatter plot
    ax.scatter(x, y, z, c='b', marker='o', s = 1)

    # Connect the points with lines
    for i in range(1, len(x)):
        ax.plot([x[i-1], x[i]], [y[i-1], y[i]], [z[i-1], z[i]], c='b')

    ax.set_xlabel('x')
    ax.set_ylabel('y')
    ax.set_zlabel('z')

    plt.show()

def plot_3d_wireframe(coordinates):
    x_coords = [coordinate[0] for coordinate in coordinates]
    y_coords = [coordinate[1] for coordinate in coordinates]
    z_coords = [coordinate[2] for coordinate in coordinates]

    # Convert coordinates to numpy arrays
    x = np.array(x_coords)
    y = np.array(y_coords)
    z = np.array(z_coords)

    # Create a 3D plot
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    # Plot the wireframe
    ax.plot_trisurf(x, y, z, linewidth=0, antialiased=False)

    ax.set_xlabel('x')
    ax.set_ylabel('y')
    ax.set_zlabel('z')

    plt.show()


def str2list(input_string):
    input_string = input_string.replace('\r', "")
    input_string = input_string.replace('\n', "")
    
    temp_buffer = []
    temp_text = ''
    selected_dots = []

    for stringData in input_string.split('\n'):
        temp_buffer.append(temp_text + stringData.split('\n')[0])
        temp_text = ''

        if temp_buffer[-1][:3] == "aaa":
            temp_text = ''
            build_temp = temp_buffer[-1][3:].rstrip('\n\r,')
            
            # Handle the case when the string ends with a trailing comma
            if build_temp[-2]:
                build_temp = build_temp[:-2]+build_temp[-1]

            try:
                nested_list = [list(map(float, innerList)) for innerList in json.loads(build_temp)]
            except json.decoder.JSONDecodeError as e:
                print("Error decoding JSON:", e)
                print("Problematic data:", build_temp)
                return False
            
            for nested_item in nested_list:
                x_temp = ((nested_item[2] * cos(nested_item[0] * (pi / 180)) * cos(nested_item[1] * (pi / 180))) + 2000) /10
                y_temp = ((nested_item[2] * cos(nested_item[0] * (pi / 180)) * sin(nested_item[1] * (pi / 180))) + 2000) /10
                z_temp = ((nested_item[2] * sin(nested_item[0] * (pi / 180))) + 2000) /10
                
                if x_temp < 1000 and y_temp < 1000:
                    selected_dots.append([x_temp, y_temp,z_temp])

            #print("Selected dots:", selected_dots)
            return selected_dots

    return False


class ReadLine:
    def __init__(self, s):
        self.buf = bytearray()
        self.s = s

    def readline(self):
        i = self.buf.find(b"\n")
        if i >= 0:
            r = self.buf[:i+1]
            self.buf = self.buf[i+1:]
            return r.decode("utf-8")
        
        while True:
            i = max(1, min(2048, self.s.in_waiting))
            data = self.s.read(i)
            i = data.find(b"\n")
            if i >= 0:
                r = self.buf + data[:i+1]
                self.buf[0:] = data[i+1:]
                return r.decode("utf-8")
            else:
                self.buf.extend(data)

ser = serial.Serial('COM3', 9600)
rl = ReadLine(ser)

can_update = False
list_for_3d = []

while not printed:
    line = rl.readline()
    #print("Received:", line)
    print_temp = str2list(line)
    if print_temp != False:
        #print(print_temp)
        if (int(print_temp[0][1]) == 0 and int(print_temp[0][0]) == 0):
            can_update = True
        if True:
            for i in print_temp:
                list_for_3d.append(i)
            print(len(list_for_3d)/50)
            if len(list_for_3d) >= 200*50:
                #plot_dots(print_temp)
                #plot_lines(print_temp)
                #print(len(list_for_3d))
                # Specify the file path
                file_path = "example" + dt_string + ".txt"

                # Open the file in write mode
                with open(file_path, 'w') as file:
                    # Write the text to the file
                    file.write(str(list_for_3d))

                print(f"Text saved to {file_path}")
                #print(list_for_3d)
                plot_3d_surface(list_for_3d)
                printed = True

        
