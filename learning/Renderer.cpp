#include "Renderer.h"

Renderer::Renderer(Window& parent) : RenderBase(parent), framesPerSecond(0), oneSecond(0), startingTime(0),
	object(nullptr), particleMaster(nullptr)
{
	// Initialize the basics ( later move to a function )
	camera = new Camera(0,0,Vector3(0,0,200.f));
	projMatrix = Matrix4::Perspective(0.1f, 1000.0f, (float)width / (float)height, 50.0f);
	modelMatrix = modelMatrix * Matrix4::Scale(Vector3(100,100,100));

	// That experimental quad
	object = new RenderObject();
	if (!object->SetShader("shader/TriangleVShader.glsl", "shader/TriangleFShader.glsl")) {
		cout << "Shader set up failed!" << endl;
	}

	if (!object->GetTexture()->SetTexture("../assets/Textures/container.jpg")) {
		cout << "Texture set up failed!" << endl;
	}

	object->GetMesh()->CreateTriangle();
	//object->SetMesh(new HeightMap(5, 5));

	//CreateParticle();

	init = true;
}

Renderer::~Renderer(void) {
	delete camera;
	if (object) {
		delete object;
	}
	if (particleMaster) {
		delete particleMaster;
	}
}

void Renderer::Update(float dt) {
	camera->UpdateCamera(dt*0.1f);
	if (particleMaster) {
		particleMaster->Update(dt);
	}
	////////////
	// Render //
	////////////
	Render();

	// Utility
	FPSCalculation(dt);
}

void Renderer::Render() {
	const GLfloat backgroundColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	glClearBufferfv(GL_COLOR, 0, backgroundColor);

	if (object) {
		renderObject();
	}

	if (particleMaster) {
		particleMaster->Render();
	}
	
	::SwapBuffers(deviceContext);
}

void Renderer::renderObject()
{
	glUseProgram(object->GetShader()->GetProgram());
	Matrix4 transform = projMatrix * camera->BuildViewMatrix() * modelMatrix;
	//glUniformMatrix4fv(glGetUniformLocation(object->GetShader()->GetProgram(), "ViewMatrix"), 1, GL_FALSE, (float*)&camera->BuildViewMatrix());
	//glUniformMatrix4fv(glGetUniformLocation(object->GetShader()->GetProgram(), "ProjMatrix"), 1, GL_FALSE, (float*)&projMatrix);
	//glUniformMatrix4fv(glGetUniformLocation(object->GetShader()->GetProgram(), "ModelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniformMatrix4fv(glGetUniformLocation(object->GetShader()->GetProgram(), "TransformMatrix"), 1, GL_FALSE, (float*)&transform);
	object->Draw();
	glUseProgram(0);
	if (glGetError()) {
		cout << "Error Happens! Code: " << glGetError() << endl;
	}
}

void Renderer::FPSCalculation(float dt) {
	oneSecond += dt * 0.001f;
	framesPerSecond = framesPerSecond + 1;
	if (oneSecond > 1.0f) {
		cout << "Current FPS: " << framesPerSecond << endl;
		oneSecond = 0;
		framesPerSecond = 0;
	}
}

void Renderer::CreateParticle()
{
	Shader* particleShader = new Shader("shader/ParticleBaseVShader.glsl", "shader/ParticleBaseFShader.glsl");
	if (!particleShader->LinkProgram()) {
		cout << "Shader set up failed!" << endl;
	}

	particleMaster = new ParticleMaster();
	particleMaster->AddSystem(new ParticleSystemBase(projMatrix, "../assets/Textures/star.jpg",
		camera, particleShader, 1, { 0,0,-50 }, 3.f, 2, 7));
}
