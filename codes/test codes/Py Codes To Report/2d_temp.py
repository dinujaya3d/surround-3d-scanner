import serial
import json
from math import cos, sin, pi
import matplotlib.pyplot as plt

printed = False

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
                x_temp = ((nested_item[1] * cos(nested_item[0] * (pi / 180))) ) /10
                y_temp = ((nested_item[1] * sin(nested_item[0] * (pi / 180))) ) /10
                
                if x_temp**2 + y_temp**2 < 900:
                    selected_dots.append([x_temp, y_temp])

            print("Selected dots:", selected_dots)
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

while not printed:
    line = rl.readline()
    print("Received:", line)
    print_temp = str2list(line)
    if print_temp != False:
        print(print_temp)
        plot_dots(print_temp)
        printed = True