#pragma once

#include "ParticleSystemBase.h"
#include <vector>

constexpr auto MAX_PSYSTEM = 10000;

class ParticleMaster
{

public:

	ParticleMaster();
	~ParticleMaster();

	void AddSystem(ParticleSystemBase* p);
	void RemoveSystem();

	void Update(float dt);
	void Render();

	void DeleteParticleMaster();
	bool SystemExist(); // need?
	
protected:
	std::vector<ParticleSystemBase*> particleSystemArray[MAX_PSYSTEM];
};

