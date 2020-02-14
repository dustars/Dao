#pragma once

#include "glew.h"
#include <list>
#include "../basic/math/Vector3.h"

constexpr auto GRAVITY = -50.f;

class Particle
{
public:
	// Contain all particles
	static std::list<Particle> ParticleList;

	// Iterate and update all particles in the listf
	static void UpdateParticleList(float dt) {
		dt *= 0.001f;
		for (auto i = ParticleList.begin(); i != ParticleList.end(); ++i) {
			bool ifAlived = i->Update(dt);
			if (!ifAlived) {
				i = ParticleList.erase(i);
				if (i == ParticleList.end()) {
					break;
				}
			}
		}
	}

	inline static void AddParticle(Vector3 position, Vector3 velocity, float rotation, float scale, float weight, float life) {
		Particle newP(position, velocity, rotation, scale, weight, life);
		ParticleList.push_back(newP);
	}


	Particle();
	Particle(Vector3 position, Vector3 velocity, float rotation, float scale, float weight, float life);
	~Particle();
	// Update a single particle
	bool Update(float dt);

private:
	Vector3 position;
	Vector3 velocity;
	float rotation;
	float scale;
	float weight;
	float life; // Remaining life of the particle. if < 0 : dead and unused.

	float elapsedTime;
};
