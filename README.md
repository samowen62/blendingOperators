# Blending Operators
We utilize Hermite radial basis functions (HRBFs) to specify the surface associated with each bone of a skeleton implicitly as a level set. These functions are particularly useful when creating realistic blends/unions between intersecting joints. This repository features implementations of some of these blending operators using techniques detailed in Vaillant et. al. and Gourmel et. al. 



![Interpolation Wire](/images/Iterpolation.png =450x)
Original mesh

![Interpolation Wire](/images/Iterpolation-wire.png =450x)
Wire Frame after deformation

![Interpolation Faces](/images/Iterpolation-faces.png =450x)
Result of deformation


![Clean-Union Wire](/images/Clean-Union.png =450x)
Original mesh

![Clean-Union Wire](/images/Clean-Union-wire.png =450x)
Wire Frame after deformation

![Clean-Union Faces](/images/Clean-Union-faces.png =450x)
Result of deformation


This program's dependencies include OpenGL, GLU, GLEW, SDL2, and the Eigen and libigl c++ libraries and is written in the C++11 standard.

run '''make all''' to compile the program.

A necessary first step that must be perfomed for each mesh before any kind of manipulation or viewing is to calculate the coefficents of the HRBF function which comes down to solving a system of equations 4 times the size of the number of verticies.

'''./out hrbf objs/&lt;file&gt;.obj '''

This process outputs a .hrbf file that is read during the viewing process. For the existing objects in this repository they are already calculated.

Next we can run the viewer with 

'''./out viewer &lt;alpha&gt; &lt;lambda&gt; &lt;face_visible&gt;'''

The parameters '''alpha''' and '''lambda''' (both &lt; 1) are floats, the former describing the degree to which we try to conform to the iso surface during our tangental relaxation process. I've found that low decimals around 0.01 to work best. The latter parameter controlls the degree to which smooth the surface during tangental relaxation. '''face_visible''' determines wether we display the wire frame (0) or actual surfaces with faces (1). 

##TODO:
* Add support for multiple joints and incorporate parent/child relationships among the nodes.
* Put verticies in normals inside a VBO instead of calling the gl functions.
* Add more images of different rotations
