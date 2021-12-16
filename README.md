# CS123 Final Project

Jinwoo Choi,
Allison Yeh,
Miranda Mo,
Brandon Lee

## Project Proposal

**Logins:** ayeh2, blee50, jchoi38, mmo4

**Group Name:** 
JAMB

**Description/Ideas (Generally, what you expect your final project to look like. Feel free to include multiple ideas):**

Snowy mountain - terrain with lights and snow or fireworks (particles) with some fancy (but not too fancy, and probably just one or two) stylized shaders/filters.

Flower field - terrain, flower/grass (geometry shader), petals (particles)

**Technical features:**

Terrain (a mountain or field)
Particles (snow, fireworks, petals)
Stylized shader OR filter (toon and/or outlines)
River
Bloom

**Github repo link:** https://github.com/allisonkyeh/CS123JAMB 

## Project Plan

**General description of how each feature will be implemented.**

Magma particles:
- Starting off from particle generation lab, modifying physics/math as necessary
- Implement billboarding

Mountain construction:
- The volcanoes were generated with bicubic interpolation. We are planning on modifying the terrain random value generations in terrain.cpp so that we can create an intentional mountain shape mimicking a cinder cone or composite volcano. 

**Resources**

Billboarding:
- https://gamedev.stackexchange.com/questions/113147/rotate-billboard-towards-camerahttps://www.geeks3d.com/20140807/billboarding-vertex-shader-glsl/

Texture:
- https://doc.instantreality.org/tools/color_calculator/
- https://learnopengl.com/Getting-started/Textures

Skybox:
- https://amin-ahmadi.com/2019/07/28/creating-a-skybox-using-c-qt-and-opengl/

**High-level Overview**

Magma particles will be in the upper half of the screen while the volcanic terrain is the bottom half to display a scene of volcanic eruption! 

**Division of Labor**

Magma particles:
- Port particles shader into project with terrain shader, transform from camera space into world space, apply billboarding to make it appear 3D, and style it to represent magma spitting out from the volcano
  - Allison
  - Brandon

Volcanic terrain:
- Calculate volcanic terrain with bicubic interpolation and doubling waves at offset, add textures
  - Jinwoo
  - Miranda

## How to run
Clone the repository and open volcano.pro with QT Creator. Build and run the project to see the volcanos!

## Known bugs
No known bugs.
