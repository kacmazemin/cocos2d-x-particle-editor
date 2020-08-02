#pragma once

#include <array>
#include <string>
#include <vector>

#include "base/ccTypes.h"
#include "math/Vec2.h"

namespace cocos2d
{
class Node;
class ParticleSystem;
class Image;
}

class ParticleEditor
{
public:
	explicit ParticleEditor(cocos2d::Node* parent);

	void draw();

private:
	struct ParticleSystemData {
		cocos2d::ParticleSystem* system = nullptr;
		cocos2d::Image* textureImage = nullptr;
		
		// emitter props
		float emitDuration = 0.f;
		std::array<int, 2> emitPositionVariance; // fixme emitPosition?
		int maxParticles = 0;
		float emitAngle = 0.f;
		float emitAngleVar = 0.f;
		int currentTypeIdx = 0;

		// emitter props - gravity
		float speed = 0.f;
		float speedVar = 0.f;
		cocos2d::Vec2 gravity;
		float radialAcceleration = 0.f;
		float radialAccelerationVar = 0.f;
		float tangentialAcceleration = 0.f;
		float tangentialAccelerationVar = 0.f;
		bool rotationIsDirection = false;

		// emitter props - radial
		float startRadius = 0.f;
		float startRadiusVar = 0.f;
		float endRadius = 0.f;
		float endRadiusVar = 0.f;
		float rotationPerSecond = 0.f;
		float rotationPerSecondVar = 0.f;

		// particle props
		float lifeTime = 0.f;
		float lifeTimeVar = 0.f;
		float startSize = 0.f;
		float startSizeVar = 0.f;
		float endSize = 0.f;
		float endSizeVar = 0.f;
		float startSpin = 0.f;
		float startSpinVar = 0.f;
		float endSpin = 0.f;
		float endSpinVar = 0.f;

		// color props
		cocos2d::Color4F startColor;
		cocos2d::Color4F startColorVar;
		cocos2d::Color4F endColor;
		cocos2d::Color4F endColorVar;

		// todo texture props
	};
	
    static constexpr std::array<const char*, 2> typeNames{"Gravity", "Radial"};

	std::vector<ParticleSystemData> systemData;
	size_t currentIdx = 0;

	void addParticleSystem(const std::string& path);
	void addParticleSystem(cocos2d::ParticleSystem* ps);
	static void drawParticleSystemData(ParticleSystemData& data);

	static void serialize(const ParticleSystemData& data, const std::string& path);
	static void updatePropertiesFromSystem(ParticleSystemData& data);
	
	template<typename T, typename M>
	static T* castContainer(M& m) { return reinterpret_cast<T*>(&m); }
};
