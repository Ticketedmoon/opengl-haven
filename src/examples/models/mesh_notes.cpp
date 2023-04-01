### Mesh Notes

In OpenGL, a `mesh` represents a single drawable entity.

Structs have a great property in C++ that their memory layout is sequential.  
That is, if we were to represent a struct as an array of data, it would only contain the struct's variables in sequential order.

Another great use of structs is a preprocessor directive called `offsetof(s, m)` that takes as its first argument a struct and as 
its second argument a variable name of the struct. The macro returns the byte offset of that variable from the start of the struct.  
This is perfect for defining the offset parameter of the glVertexAttribPointer function:
