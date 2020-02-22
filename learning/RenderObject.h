#pragma once
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "Transform.h"

class RenderObject
{
public:
	RenderObject();
	~RenderObject();

	inline Shader*		GetShader()		{ return shader; }
	inline Mesh*		GetMesh()		{ return mesh; }
	inline Texture*		GetTexture()	{ return texture; }
	inline Transform*	GetTransform()	{ return transform; }

	void SetMesh(Mesh* m);

	bool SetShader(string vs, string fs, string gs = "", string cs = "", string es = "");

	void Draw();

protected:
	Shader*			shader;
	Mesh*			mesh;
	Texture*		texture;
	Transform*		transform;
};

