#pragma once

#include <string>
#include <vector>

#include "../basic/math/Vector4.h"
#include "../basic/math/Vector2.h"

#include "glew.h"

using namespace std;

class Mesh {
	enum BufferName {
		POSITION,
		COLOR,
		TEXTURE,
		INDEX,
		MAXBUFFER
	};

public:
	Mesh();
	Mesh(string filename);
	~Mesh();

	void CreateTriangle();
	void CreateQuad();

	int						GetNumOfVertices()		{ return numOfVertices; }
	GLuint					GetVAO()				{ return vao; }
	GLuint					GetType()				{ return renderType; }
	vector<Vector3>&		GetPosition()			{ return position; }
	vector<Vector3>&		GetColor()				{ return color; }
	vector<Vector2>&		GetTexCoord()			{ return texCoord; }
	vector<unsigned int>&	GetIndex()				{ return index; }

	void Draw();
	
protected:
	GLuint vao;
	GLuint vbo[MAXBUFFER];
	void BufferDataToGPU();

	GLuint renderType;

	vector<Vector3> position;
	vector<Vector3> color;
	vector<Vector2> texCoord;
	vector<unsigned int> index;

	unsigned int numOfVertices;
	unsigned int numOfIndex;

	void EnableAttribs();
	void DisableAttribs();
};



