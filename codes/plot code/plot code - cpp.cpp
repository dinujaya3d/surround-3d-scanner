#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <ctime>
#include <json/json.h> // JSON library
#include <serial/serial.h> // Serial library
#include <gnuplot-iostream.h> // GNUplot library for plotting

#define PI 3.14159265358979323846

using namespace std;

// Structure to hold the 3D coordinates
struct Coordinate {
    float x;
    float y;
    float z;
};

// Function prototype for plotting 3D surface
void plot3DSurface(const vector<Coordinate>& coordinates);

int main() {
    // Open the serial port
    serial::Serial my_serial("COM8", 9600, serial::Timeout::simpleTimeout(1000));

    // Check if the serial port is open
    if (!my_serial.isOpen()) {
        cerr << "Failed to open serial port!" << endl;
        return 1;
    }

    vector<Coordinate> coordinates; // Vector to store the coordinates
    bool printed = false;
    int printed_lines = 0;
    bool can_update = false;

    // Loop to read data from the serial port
    while (!printed) {
        string line = my_serial.readline(); // Read a line from the serial port
        vector<Coordinate> temp_coords = str2list(line); // Convert the line to a list of coordinates

        // Check if the list is not empty
        if (!temp_coords.empty()) {
            // Check if the first coordinates are zero
            if (static_cast<int>(temp_coords[0].y) == 0 && static_cast<int>(temp_coords[0].x) == 0) {
                can_update = true;
            }

            // Update the list of coordinates if allowed
            if (can_update) {
                coordinates.insert(coordinates.end(), temp_coords.begin(), temp_coords.end());
                printed_lines++;

                // Once enough lines have been read, save the coordinates to a file and plot the surface
                if (printed_lines >= 67) {
                    ofstream outfile("example.txt");
                    for (const auto& coord : coordinates) {
                        outfile << coord.x << " " << coord.y << " " << coord.z << "\n";
                    }
                    outfile.close();
                    plot3DSurface(coordinates);
                    printed = true;
                }
            }
        }
    }

    return 0;
}

// Function to convert a string to a list of coordinates
vector<Coordinate> str2list(const string& input_string) {
    string cleaned_string = input_string;
    cleaned_string.erase(remove(cleaned_string.begin(), cleaned_string.end(), '\r'), cleaned_string.end());
    cleaned_string.erase(remove(cleaned_string.begin(), cleaned_string.end(), '\n'), cleaned_string.end());

    vector<Coordinate> selected_dots;
    size_t pos = cleaned_string.find("aaa");

    // Check if the string contains the prefix "aaa"
    if (pos != string::npos) {
        string json_string = cleaned_string.substr(pos + 3);

        // Parse the JSON string
        Json::Value root;
        Json::CharReaderBuilder builder;
        builder["collectComments"] = false;
        JSONCPP_STRING errs;
        istringstream s(json_string);
        if (!parseFromStream(builder, s, &root, &errs)) {
            cerr << "Error decoding JSON: " << errs << endl;
            return {};
        }

        // Convert JSON data to a list of coordinates
        for (const auto& item : root) {
            float angle1 = item[0].asFloat() * (PI / 180);
            float angle2 = item[1].asFloat() * (PI / 180);
            float distance = item[2].asFloat();

            float x_temp = ((distance * cos(angle1) * cos(angle2)) + 2000) / 10;
            float y_temp = ((distance * cos(angle1) * sin(angle2)) + 2000) / 10;
            float z_temp = ((distance * sin(angle1)) + 2000) / 10;

            if (x_temp < 500 && y_temp < 500) {
                selected_dots.push_back({x_temp, y_temp, z_temp});
            }
        }
    }

    return selected_dots;
}

// Function to plot the 3D surface using GNUplot
void plot3DSurface(const vector<Coordinate>& coordinates) {
    Gnuplot gp;
    gp << "set title '3D Surface Plot'\n";
    gp << "set xlabel 'X-axis'\n";
    gp << "set ylabel 'Y-axis'\n";
    gp << "set zlabel 'Z-axis'\n";
    gp << "splot '-' with points pointtype 7 pointsize 1 notitle\n";

    // Send coordinates to GNUplot
    gp.send1d(boost::make_tuple(coordinates.begin(), coordinates.end(), 
               [](const Coordinate& coord) { return make_tuple(coord.x, coord.y, coord.z); }));
    gp << "e\n";
}
