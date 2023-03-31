### General Lighting
We chose to do the lighting calculations in world space, but most people tend to prefer doing lighting in view space. 
An advantage of view space is that the viewer's position is always at (0,0,0) so you already got the position of the viewer for free. 
However, I find calculating lighting in world space more intuitive for learning purposes.  
If you still want to calculate lighting in view space you want to transform all the relevant vectors with the view matrix as well (don't forget to change the normal matrix too).

### Fragment Positions 
With regards to the actual fragment's position, we're going to do all the lighting calculations in world space so 
we want a vertex position that is in world space first. 
We can accomplish this by multiplying the vertex position attribute with the model matrix only (not the view and projection matrix) 
to transform it to world space coordinates.

### Unit Vectors
When calculating lighting we usually do not care about the magnitude of a vector or their position; we only care about their direction. 
Because we only care about their direction almost all the calculations are done with unit vectors since it simplifies 
most calculations (like the dot product). So when doing lighting calculations, make sure you always normalize the relevant vectors to 
ensure they're actual unit vectors. Forgetting to normalize a vector is a popular mistake.

### Normal calculation
If the angle between both vectors is greater than 90 degrees then the result of the dot product will actually become negative and we end 
up with a negative diffuse component. For that reason we use the max function that returns the highest of both its parameters to make sure 
the diffuse component (and thus the colors) never become negative.

### Matrix Operations on the GPU (Avoid!)
Inversing matrices is a costly operation for shaders, so wherever possible try to avoid doing inverse operations since 
they have to be done on each vertex of your scene. For learning purposes this is fine, but for an efficient application you'll likely 
want to calculate the normal matrix on the CPU and send it to the shaders via a uniform before drawing (just like the model matrix).

### Phong vs Gouraud shading
Phong lighting is a combination of ambient, diffuse and specular lighting, where the light source is in world or view space.
When the Phong lighting model is implemented in the vertex shader it is called Gouraud shading instead of Phong shading.  
**Note:** that due to the interpolation the lighting looks somewhat off. The Phong shading gives much smoother lighting results.  

![Phong vs Gouraud](https://learnopengl.com/img/lighting/basic_lighting_gouruad.png)

### Blinn-Phong lighting
Phong lighting is a great and very efficient approximation of lighting, but its specular reflections break down in certain conditions, 
specifically when the shininess property is low resulting in a large (rough) specular area.  
The image below shows what happens when we use a specular shininess exponent of 1.0 on a flat textured plane:  
![Phong Lighting Specular Lighting vs Blinn-Phong](https://learnopengl.com/img/advanced-lighting/advanced_lighting_comparrison.png)  

You can see at the edges that the specular area is immediately cut off. The reason this happens is because the angle between the view and 
reflection vector doesn't go over 90 degrees. If the angle is larger than 90 degrees, the resulting dot product becomes negative and this 
results in a specular exponent of 0.0.  
You're probably thinking this won't be a problem since we shouldn't get any light with angles higher than 90 degrees anyways, right?  

Wrong, this only applies to the diffuse component where an angle higher than 90 degrees between the normal and light source means the 
light source is below the lighted surface and thus the light's diffuse contribution should equal 0.0. However, with specular lighting 
we're not measuring the angle between the light source and the normal, but between the view and reflection vector.  

![Phong vs Blinn-Phong visualisation](https://learnopengl.com/img/advanced-lighting/advanced_lighting_over_90.png)  

Here the issue should become apparent.  
The left image shows Phong reflections as familiar, with θ being less than 90 degrees. 
In the right image we can see that the angle θ between the view and reflection vector is larger than 90 degrees which as a result 
nullifies the specular contribution. This generally isn't a problem since the view direction is far from the reflection direction, 
but if we use a low specular exponent the specular radius is large enough to have a contribution under these conditions.  

Since we're nullifying this contribution at angles larger than 90 degrees we get the artifact as seen in the first image.

