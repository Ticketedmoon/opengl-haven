# opengl-haven
A project based around learning OpenGL. This repo will contain files about different rendering techniques and scenarios in OpenGL.

---

#### The graphic Pipeline (fig 1.0)
![Graphics Pipeline Image from LearnOpenGL](https://learnopengl.com/img/getting-started/pipeline.png)

---

#### Normalized Device Coordinates (NDC) (fig 1.1)
![Normalized Device Coorindinates Graph Image](https://learnopengl.com/img/getting-started/ndc.png)

**Note:** The NDC coordinates will then be transformed to screen-space coordinates via the viewport transform using the data you provided with glViewport. The resulting screen-space coordinates are then transformed to fragments as inputs to your fragment shader.

---

#### Format of vertex buffer data:
![Vertex Buffer Data format](https://learnopengl.com/img/getting-started/vertex_attribute_pointer.png);

**Note:** The following diagram shows the size of each vertex attribute inside a contigious block of memory on the GPU.  
The example above uses `float` as the type for each vertex attribute and hence are 4 bytes in size.

---

#### Progress Report - Showing what's been built (Screenshots/Videos):

**The "simple" triangle** - 350 lines of openGL code + comments  

![Mon 06 Feb 2023 20:48:04 GMT](https://user-images.githubusercontent.com/21260839/217082227-c4430511-97d0-4559-9d19-53f47c54fa9e.png)


---

#### What will be demonstrated [✅ / ❌]:
**Stage #1**
- Making a Window [✅]
- Storing vertices [✅]
- Compiling shaders [✅]
- Drawing a triangle [✅]
- Loading Textures [❌]
- Applying Transformations [❌]
- Loading Object models [❌]

**Stage #2**
- Blinn-Phong Lighting [❌]
- Billboards [❌]
- Multiple Shaders [❌]
- Custom Framebuffers [❌]
- Text []
- Advanced Materials (e.g., Normal Mapping) [❌]

**Stage #3**
- Bloom [❌]
- Shadows [❌]
- Stencil buffer Usage [❌]
- Instanced rendering [❌]
- Deferred shading [❌]
- Screen-Space ambient occlusion [❌]

**Note:**   
After `stage #1` and `stage #2` you will have a good body of knowledge and be comfortable in OpenGL.  
Use `stage #3` for refinement. At this stage you will pick items and slot them into your current existing knowledge.  
We can start working on projects after `stage #1` and `stage #2` are understood, or perhaps a little into `stage #3`. 

---

#### Prerequisites
- cmake  
- glfw  
- libxrandr (package)
- libxrandr-dev (package)
- libxinerama-dev (package)
- libxcursor-dev (package)
- libxi-dev (package)

**Note:** The above dependencies annotated with `(package)` might be required by your package manager when running cmake.

##### Alternative
You can execute the setup script: `setup.sh` in the base directory to configure glfw for you with cmake.  
**Note:** Currently it is configured using Ubuntu's package manager: `apt-get`. Please adapt the script if you are using a different package manager.

---

#### Additional Notes
If `GLAD`, `KHR` and `glad.c` files are missing, they can be recreated using:
https://glad.dav1d.de/#language=c&specification=gl&api=gl%3D3.3&api=gles1%3Dnone&api=gles2%3Dnone&api=glsc2%3Dnone&profile=core&loader=on

Ensure you are using openGL Core and `generate a loader` is selected.
