### Mesh Notes

In OpenGL, a `mesh` represents a single drawable entity.

Structs have a great property in C++ that their memory layout is sequential.  
That is, if we were to represent a struct as an array of data, it would only contain the struct's variables in sequential order.

Another great use of structs is a preprocessor directive called `offsetof(s, m)` that takes as its first argument a struct and as 
its second argument a variable name of the struct. The macro returns the byte offset of that variable from the start of the struct.  
This is perfect for defining the offset parameter of the glVertexAttribPointer function:

### Texture Types for Meshes
- aiTextureType_NONE
Dummy value.
No texture, but the value to be used as 'texture semantic' (#aiMaterialProperty::mSemantic) for all material properties not related to textures.

- aiTextureType_DIFFUSE
The texture is combined with the result of the diffuse lighting equation.

- aiTextureType_SPECULAR
The texture is combined with the result of the specular lighting equation.

- aiTextureType_AMBIENT
The texture is combined with the result of the ambient lighting equation.

- aiTextureType_EMISSIVE
The texture is added to the result of the lighting calculation.
It isn't influenced by incoming light.

- aiTextureType_HEIGHT
The texture is a height map.
By convention, higher gray-scale values stand for higher elevations from the base height.

- aiTextureType_NORMALS
The texture is a (tangent space) normal-map.
Again, there are several conventions for tangent-space normal maps. Assimp does (intentionally) not distinguish here.

- aiTextureType_SHININESS
The texture defines the glossiness of the material.
The glossiness is in fact the exponent of the specular (phong) lighting equation. Usually there is a conversion function defined to map the linear color values in the texture to a suitable exponent. Have fun.

- aiTextureType_OPACITY
The texture defines per-pixel opacity.
Usually 'white' means opaque and 'black' means 'transparency'. Or quite the opposite. Have fun.

- aiTextureType_DISPLACEMENT
Displacement texture.
The exact purpose and format is application-dependent.
Higher color values stand for higher vertex displacements.

- aiTextureType_LIGHTMAP
Lightmap texture (aka Ambient Occlusion)
Both 'Lightmaps' and dedicated 'ambient occlusion maps' are covered by this material property. The texture contains a scaling value for the final color value of a pixel. Its intensity is not affected by incoming light.

- aiTextureType_REFLECTION
Reflection texture.
Contains the color of a perfect mirror reflection. Rarely used, almost never for real-time applications.

- aiTextureType_UNKNOWN
Unknown texture.
A texture reference that does not match any of the definitions above is considered to be 'unknown'. It is still imported, but is excluded from any further postprocessing.
