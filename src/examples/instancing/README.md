### What is Instancing?

Instancing is a technique where we draw many (equal mesh data) objects at once with a single render call, saving us all 
the CPU -> GPU communications each time we need to render an object. To render using instancing all we need to do is change the 
render calls `glDrawArrays` and `glDrawElements` to `glDrawArraysInstanced` and `glDrawElementsInstanced` respectively.  
These instanced versions of the classic rendering functions take an extra parameter called the instance count that sets the number of instances 
we want to render. We send all the required data to the GPU once, and then tell the GPU how it should draw all these instances with a single call. 
The GPU then renders all these instances without having to continually communicate with the CPU.

By itself this function is a bit useless. Rendering the same object a thousand times is of no use to us since each of the rendered objects is 
rendered exactly the same and thus also at the same location; we would only see one object! 
For this reason GLSL added another built-in variable in the vertex shader called `gl_InstanceID`.

When drawing with one of the instanced rendering calls, `gl_InstanceID` is incremented for each instance being rendered starting from 0. 
If we were to render the 43th instance for example, `gl_InstanceID` would have the value `42` in the vertex shader. 
Having a unique value per instance means we could now for example index into a large array of position values to position each instance at a different location in the world.
