///////////////////////////////////////////////////////////////////////////////
///
///	@File  : ParticleSystem.h
/// @Brief : This file defines the ParticleSystem class, responsible for 
///          managing and simulating particle effects within the engine. 
///          The system handles particle emission, updates, and rendering, 
///          supporting various emission shapes and behaviors. It allows for 
///          efficient particle reuse and dynamic customization of particle 
///          properties such as velocity, lifetime, and texture.
///
///	@Main Author : Edwin Leow (100%)
///	@Secondary Author : NIL
/// @Copyright 2025, Digipen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef _ParticleSystem_H_
#define _ParticleSystem_H_

#include "pch.h"
#include "System.h"
#include "Vector3D.h"
#include "Vector2D.h"
#include "InputHandler.h"
#include "ComponentList.h"
#include "Graphics.h"
#include "EngineState.h"

namespace Framework
{
	class ParticleSystem : public ISystem
	{
	public:
		static ParticleSystem* GetInstance()
		{
			static ParticleSystem instance;
			return &instance;
		}

		Graphics::Model* particleMesh;
		bool abilityTest = false;

		ParticleSystem();
		~ParticleSystem() = default;

		void Initialize();
		void Update(float deltaTime) override;
		std::string GetName() override;

		std::vector<ParticleComponent> particles;		// Dynamic Array of Particles
		unsigned int maxParticles = 10000;				// Maximum Number of Particles
		glm::vec2 emitterPosition = { 0,0 };			// Position of the Particle Emitter

		void emit(Entity entity, float deltaTime);
		void emitDamageNumber(Entity entity, int damage);
		glm::vec2 randomVelocity(EmissionShape shape);
		void resetParticles(Entity entity, std::string textureName);	// Reset a particle to its initial state
		void SetEmit(bool value) { shouldEmit = value; }				// Set emission flag
		InputHandler* InputHandlerInstance;

	private:
		ParticleComponent* getInactiveParticle();		// Find an inactive particle to reuse
		glm::vec2 randomVelocity();						// Generate some randomness in particle velocity
		bool shouldEmit = false;						// Controls continuous emission
	};
	extern ParticleSystem GlobalParticleSystem;
}
#endif // !_ParticleSystem_H_