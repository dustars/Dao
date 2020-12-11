/*
	1. Do we need a base class? Is it necessary to use polymorphism?
	2. Now the Getters are just resources leak, do we really need them?
	3. A better method to create a cube and plane for testing purpose
	4. Normal Generation doesn't work accurately
*/

#pragma once
#include <string>
#include <vector>
#include "math/Math.h"
#include <GL/glew.h>

class Mesh {
protected:
	enum BufferName {
		POSITION,
		COLOR,
		TEXTURE,
		NORMAL,
		INDEX,
		MAXBUFFER
	};

public:
	Mesh();
	explicit Mesh(const std::string& filename); //using Assimp as model loader
	virtual ~Mesh();

	void CreatePlane();
	void CreateCube();
	void CreateQuad();

	int						GetNumOfVertices()		const { return numOfVertices; }
	GLuint					GetVAO()				const { return vao; }
	GLuint					GetType()				const { return renderType; }
	std::vector<Vector3>&		GetPosition()			{ return position; }
	std::vector<Vector3>&		GetColor()				{ return color; }
	std::vector<Vector2>&		GetTexCoord()			{ return texCoord; }
	std::vector<Vector3>&		GetNormal()				{ return normal; }
	std::vector<unsigned>&		GetIndex()				{ return index; }

	virtual void Draw();
	virtual void Update(float dt);
	
protected:
	GLuint vao;
	GLuint vbo[MAXBUFFER];
	GLuint renderType;
	unsigned numOfVertices; //?
	unsigned numOfIndex; //?

	std::vector<Vector3> position;
	std::vector<Vector3> color;
	std::vector<Vector2> texCoord;
	std::vector<Vector3> normal;
	std::vector<unsigned> index;

	void EnableAttribs();
	void DisableAttribs();

	void GenerateNormals();

	void BufferDataToGPU();
	void UpdateDataToGPU();
};



