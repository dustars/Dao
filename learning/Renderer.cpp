#include "Renderer.h"

Renderer::Renderer(Window& parent) : RenderBase(parent), framesPerSecond(0), oneSecond(0), startingTime(0){

	object = new RenderObject();
	if (!object->SetShader("shader/LearningVshader.glsl", "shader/BasicColorFShader.glsl")) {
		cout << "Shader set up failed!" << endl;
	}
	if (!object->GetTexture()->SetTexture("../assets/Textures/container.jpg")) {
		cout << "Texture set up failed!" << endl;
	}

	object->GetTransform()->CreateRotatingCube(startingTime);
	object->GetMesh()->CreateCube();

	camera = new Camera();

	Matrix4 projMatrix = Matrix4::Perspective(0.1f, 1000.0f, (float)width / (float)height, 50.0f);

	particleMaster = new ParticleMaster();
	particleMaster->AddSystem(new ParticleSystemBase(camera, projMatrix, 30, { 0,0,-50 }, 10.f));

	init = true;
}

Renderer::~Renderer(void) {
	delete object;
	delete camera;
	delete particleMaster;
}

void Renderer::Update(float dt) {
	camera->UpdateCamera(dt);
	//Vector3 color = {sin(dt), cos(dt), sin(dt)-cos(dt)};

	particleMaster->Update(dt);
	Render();

	// Utility
	FPSCalculation(dt);
}

void Renderer::Render() {
	const GLfloat backgroundColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	glClearBufferfv(GL_COLOR, 0, backgroundColor);

	//renderObject();
	particleMaster->Render();
	::SwapBuffers(deviceContext);
}

void Renderer::renderObject()
{
	glUseProgram(object->GetShader()->GetProgram());
	glBindVertexArray(object->GetMesh()->GetVAO());
	glEnableVertexAttribArray(0);

	// Update modelMatrix
	object->GetTransform()->CreateRotatingCube(startingTime);
	glUniformMatrix4fv(glGetUniformLocation(object->GetShader()->GetProgram(), "modelViewMatrix"), 1, GL_FALSE,
		(float*)&object->GetTransform()->GetModelMatrix());
	Matrix4 projMatrix = Matrix4::Perspective(0.1f, 1000.0f, (float)width / (float)height, 50.0f);
	glUniformMatrix4fv(glGetUniformLocation(object->GetShader()->GetProgram(), "projMatrix"), 1, GL_FALSE, (float*)&projMatrix);
	glUniformMatrix4fv(glGetUniformLocation(object->GetShader()->GetProgram(), "viewMatrix"), 1, GL_FALSE, (float*)&camera->BuildViewMatrix());

	glDrawArrays(GL_TRIANGLES, 0, object->GetMesh()->GetNumOfVertices());

	glDisableVertexAttribArray(0);
	glBindVertexArray(0);
	glUseProgram(0);
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