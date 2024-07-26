import ast
import numpy as np
import pyvista as pv



# Open the file in read mode
with open('example.txt', 'r') as file:
    # Read the file content
    content = file.read()

# Use ast.literal_eval to convert the string to a 2D list
points = np.array(ast.literal_eval(content))

# Create a PolyData object
cloud = pv.PolyData(points)

# Plot the point cloud
cloud.plot()

# Create a 3D Delaunay triangulation of the point cloud
volume = cloud.delaunay_3d(alpha=0.6)

# Extract the outer surface of the volume
shell = volume.extract_geometry()

# Plot the resulting mesh
shell.plot()
