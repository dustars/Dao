#pragma once
#include "RenderBase.h"
#include "RenderObject.h"
#include "Camera.h"

#include "ParticleMaster.h"

class Renderer : public RenderBase {
public:
	Renderer(Window& parent);
	virtual ~Renderer(void);

	virtual void Update(float dt) override;
	virtual void Render() override;

	void renderObject();

	float startingTime;
	float framesPerSecond;
	float oneSecond;

protected:
	
	RenderObject*		object;
	Camera*				camera;

	ParticleMaster* particleMaster;

	void FPSCalculation(float dt);
};


