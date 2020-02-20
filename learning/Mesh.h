#pragma once

#include <string>

#include "Mesh.h"
#include "MeshSource.h"
#include "glew.h"

class Mesh {
	enum BufferName {
		POSITION,
		COLOR,
		TEXTURE,
		MAXBUFFER
	};

public:
	Mesh();
	Mesh(std::string filename);
	~Mesh();

	void CreateTriangle();
	void CreateCube();

	inline int GetNumOfVertices()	{ return numOfVertices; }
	inline GLuint GetVAO()			{ return vao; }
	inline GLuint GetType()			{ return renderType; }

protected:
	GLuint vao;
	GLuint vbo[MAXBUFFER];
	GLuint renderType;

	unsigned int numOfVertices;
};



