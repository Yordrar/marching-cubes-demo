# marching-cubes-demo

This is a simple demo showcasing the Marching Cubes algorithm.

It is a really neat algorithm for procedurally generating meshes, specially terrain. I plan to eventually make a simple terrain editor based on this algorithm generating the mesh via a compute shader.

The GUI was made with the Dear ImGui library, and it allows you to change the grid resolution and cube size, as well as toggling interpolation and changing the mesh color.

I learned the algorithm from the following resources:

[Polygonising a scalar field](http://paulbourke.net/geometry/polygonise/): Article by Paul Bourke.

[Coding Adventure: Marching Cubes](https://www.youtube.com/watch?v=M3iI2l0ltbE): Video by Sebastian Lague.

## Images

![sea](/images/sea.PNG)

![mountain](/images/mountain.PNG)

![valley](/images/valley.PNG)