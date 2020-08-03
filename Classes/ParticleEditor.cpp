#include "ParticleEditor.h"

#include <array>
#include <string>

#include <imgui/imgui.h>
#include <zlib/include/zlib.h>

#include "2d/CCParticleSystem.h"
#include "2d/CCParticleExamples.h"
#include "base/base64.h"
#include "platform/CCFileUtils.h"
#include "platform/CCImage.h"
#include "renderer/CCTextureCache.h"

std::string compressToGzip(unsigned char* input, const size_t inputSize)
{
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if(deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS | 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        return "";
    }

    zs.avail_in = static_cast<uInt>(inputSize);
    zs.next_in = input;
    
    int ret;
    std::array<char, 32768> outbuffer;
    std::string outstring;

    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer.data());
        zs.avail_out = outbuffer.size();

        ret = deflate(&zs, Z_FINISH);

        if (outstring.size() < zs.total_out) {
            outstring.append(outbuffer.data(), zs.total_out - outstring.size());
        }
    } while(ret == Z_OK);

    deflate(&zs, Z_FINISH);
    deflateEnd(&zs);

    return outstring;
}

ParticleEditor::ParticleEditor(cocos2d::Node* parent)
    : parent{parent}
{
    CC_ASSERT(parent);
    addParticleSystem("Galaxy.plist");
}

void ParticleEditor::draw()
{
    if(ImGui::Begin("Editor")) {
        // todo implement particle system list
        // - add
        // - remove
        // - visible
        // - stop
        // - pause/resume
        // - reset
    }

    if(ImGui::Begin("SystemData")) {
        drawParticleSystemData(systemData[currentIdx]);
        ImGui::End();
    }
}

void ParticleEditor::addParticleSystem(const std::string& path)
{
    addParticleSystem(cocos2d::ParticleSystemQuad::create(path));
}

void ParticleEditor::addParticleSystem(cocos2d::ParticleSystem* ps)
{
    CC_ASSERT(ps);
    if(!ps->getParent()) { parent->addChild(ps); }
    ps->setPosition(cocos2d::Vec2::ZERO);
    systemData.push_back({ps, ps->getImage()});
    updatePropertiesFromSystem(systemData.back());
}

void ParticleEditor::drawParticleSystemData(ParticleSystemData& data)
{
    auto* ps = data.system;

    if(ImGui::BeginTabBar("#tab"))
    {
        if(ImGui::BeginTabItem("Emitter Settings"))
        {
            if(ImGui::InputFloat("Duration", &data.emitDuration, 0.01f, 1.0f))
            {
                ps->setDuration(data.emitDuration);
                ps->resetSystem();
            }

            if(ImGui::SliderInt2("Source Position Variance", data.emitPositionVariance.data(), 0, 2048))
            {
                ps->setPosVar(cocos2d::Vec2(data.emitPositionVariance[0], data.emitPositionVariance[1]));
            }

            if(ImGui::InputInt("Maximum Particles", &data.maxParticles, 0, 2000))
            {
                ps->setTotalParticles(data.maxParticles);
            }

            if(ImGui::SliderFloat("Emit Angle", &data.emitAngle, 0.f, 360.f))
            {
                ps->setAngle(data.emitAngle);
            }

            if(ImGui::SliderFloat("Emit Angle Variance", &data.emitAngleVar, 0.f, 360.f))
            {
                ps->setAngleVar(data.emitAngleVar);
            }

            ImGui::Spacing();
            ImGui::Spacing();

            if(ImGui::Combo("Type", &data.typeIdx, typeNames.data(),2))
            {
                ps->setEmitterMode(static_cast<cocos2d::ParticleSystem::Mode>(data.typeIdx));
                updatePropertiesFromSystem(data);
            }

            ImGui::Spacing();
            ImGui::Spacing();

            if(ps->getEmitterMode() == cocos2d::ParticleSystem::Mode::GRAVITY)
            {
                if(ImGui::InputFloat("Speed", &data.speed, 0.f, 2000.0f))
                {
                    ps->setSpeed(data.speed);
                }

                if(ImGui::InputFloat("Speed Variance", &data.speedVar, 0.f, 2000.0f))
                {
                    ps->setSpeedVar(data.speedVar);
                }

                if(ImGui::SliderFloat2("Gravity", castContainer<float>(data.gravity), -2000.f, 2000.f))
                {
                    ps->setGravity(data.gravity);
                }

                if(ImGui::SliderFloat("Radial Acceleration", &data.radialAcceleration, -2000.f, 2000.0f))
                {
                    ps->setRadialAccel(data.radialAcceleration);
                }

                if(ImGui::SliderFloat("Radial Acceleration Variance", &data.radialAccelerationVar, -2000.f, 2000.0f))
                {
                    ps->setRadialAccelVar(data.radialAccelerationVar);
                }

                if(ImGui::SliderFloat("Tangential Acceleration", &data.tangentialAcceleration, -2000.f, 2000.0f))
                {
                    ps->setTangentialAccel(data.tangentialAcceleration);
                }

                if(ImGui::SliderFloat("Tangential Acceleration Variance", &data.tangentialAccelerationVar, -2000.f, 2000.0f))
                {
                    ps->setTangentialAccelVar(data.tangentialAccelerationVar);
                }

                if(ImGui::Checkbox("Rotation is Direction", &data.rotationIsDirection))
                {
                    ps->setRotationIsDir(data.rotationIsDirection);
                }
            }
            else
            {
                if(ImGui::SliderFloat("Start Radius", &data.startRadius, 0.f, 1000.0f))
                {
                    ps->setStartRadius(data.startRadius);
                }
                
                if(ImGui::SliderFloat("Start Radius Variance", &data.startRadiusVar, 0.f, 1000.0f))
                {
                    ps->setStartRadiusVar(data.startRadiusVar);
                }
                
                if(ImGui::SliderFloat("End Radius", &data.endRadius, 0.f, 1000.0f))
                {
                    ps->setEndRadius(data.endRadius);
                }
                
                if(ImGui::SliderFloat("End Radius Variance", &data.endRadiusVar, 0.f, 1000.0f))
                {
                    ps->setEndRadiusVar(data.endRadiusVar);
                }

                if(ImGui::SliderFloat("Rotate Per Second", &data.rotationPerSecond, -1000.f, 1000.0f))
                {
                    ps->setRotatePerSecond(data.rotationPerSecond);
                }

                if(ImGui::SliderFloat("Rotate Per Second Variance", &data.rotationPerSecondVar, -1000.f, 1000.0f))
                {
                    ps->setRotatePerSecondVar(data.rotationPerSecondVar);
                }
            }

            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("Particle Settings"))
        {
            if(ImGui::SliderFloat("Lifetime", &data.lifeTime, 0.f, 10.0f))
            {
                ps->setLife(data.lifeTime);
            }

            if(ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("Fethiye Test");
                ImGui::EndTooltip();
            }

            if(ImGui::SliderFloat("Lifetime Variance", &data.lifeTimeVar, 0.f, 10.0f))
            {
                ps->setLifeVar(data.lifeTimeVar);
            }

            if(ImGui::SliderFloat("Start Size", &data.startSize, 0.f, 512.0f))
            {
                ps->setStartSize(data.startSize);
            }

            if(ImGui::SliderFloat("Start Size Variance", &data.startSizeVar, 0.f, 512.0f))
            {
                ps->setStartSizeVar(data.startSizeVar);
            }

            if(ImGui::SliderFloat("End Size", &data.endSize, 0.f, 512.0f))
            {
                ps->setEndSize(data.endSize);
            }

            if(ImGui::SliderFloat("End Size Variance", &data.endSizeVar, 0.f, 512.0f))
            {
                ps->setEndSizeVar(data.endSizeVar);
            }

            if(ImGui::SliderFloat("Start Spin", &data.startSpin, 0.f, 360.0f))
            {
                ps->setStartSpin(data.startSpin);
            }

            if(ImGui::SliderFloat("Start Spin Variance", &data.startSpinVar, 0.f, 360.0f))
            {
                ps->setStartSpinVar(data.startSpinVar);
            }

            if(ImGui::SliderFloat("End Spin", &data.endSpin, 0.f, 360.0f))
            {
                ps->setEndSpin(data.endSpin);
            }

            if(ImGui::SliderFloat("End Spin Variance", &data.endSpinVar, 0.f, 360.0f))
            {
                ps->setEndSpinVar(data.endSpinVar);
            }

            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("Color Settings"))
        {
            if(ImGui::ColorEdit4("Start Color", castContainer<float>(data.startColor))) 
            {
                ps->setStartColor(data.startColor);
            }

            if(ImGui::ColorEdit4("Start Color Variance", castContainer<float>(data.startColorVar))) 
            {
                ps->setStartColorVar(data.startColorVar);
            }

            if(ImGui::ColorEdit4("End Color", castContainer<float>(data.endColor))) 
            {
                ps->setEndColor(data.endColor);
            }

            if(ImGui::ColorEdit4("End Color Variance", castContainer<float>(data.endColorVar))) 
            {
                ps->setEndColorVar(data.endColorVar);
            }

            if(ImGui::Combo("Blend Source", &data.blendSrcIdx, blendFuncNames.data(), blendFuncNames.size()))
            {
                ps->setBlendFunc(cocos2d::BlendFunc{blendIndexToGLenum(data.blendSrcIdx), ps->getBlendFunc().dst});
            }

            if(ImGui::Combo("Blend Destination", &data.blendDstIdx, blendFuncNames.data(), blendFuncNames.size()))
            {
                ps->setBlendFunc(cocos2d::BlendFunc{ps->getBlendFunc().src, blendIndexToGLenum(data.blendDstIdx)});
            }

            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("Texture Settings"))
        {
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

void ParticleEditor::changeTexture(ParticleSystemData& data, const std::string& texturePath)
{
    const auto it = imageCache.find(texturePath); 
    if(it == imageCache.end()) {
        auto* img = new cocos2d::Image(); // its fine, not going to be freed until the end anyway
        if(img->initWithImageFile(texturePath)) {
            imageCache.emplace(texturePath, img);
            auto* tex = cocos2d::Director::getInstance()->getTextureCache()->addImage(img, texturePath);
            data.system->setTexture(tex);
            data.textureImage = img;
            updatePropertiesFromSystem(data);
        }
        else {
            // todo issue a warning
        }
    }
    else {
        auto* tex = cocos2d::Director::getInstance()->getTextureCache()->getTextureForKey(texturePath);
        data.system->setTexture(tex);
        data.textureImage = it->second;
        updatePropertiesFromSystem(data);
    }
}

void ParticleEditor::serialize(const ParticleSystemData& data, const std::string& path)
{
    cocos2d::ValueMap dict;

    dict.emplace("duration", cocos2d::Value{data.emitDuration});
    dict.emplace("maxParticles", cocos2d::Value{data.maxParticles});
    dict.emplace("angle", cocos2d::Value{data.emitAngle});
    dict.emplace("angleVariance", cocos2d::Value{data.emitAngleVar});
    dict.emplace("emitterType", cocos2d::Value{data.typeIdx});
    
    dict.emplace("sourcePositionx", cocos2d::Value{data.system->getPositionX()});
    dict.emplace("sourcePositionx", cocos2d::Value{data.system->getPositionY()});
    dict.emplace("sourcePositionVariancex", cocos2d::Value{data.emitPositionVariance[0]});
    dict.emplace("sourcePositionVariancey", cocos2d::Value{data.emitPositionVariance[1]});

    if(data.system->getEmitterMode() == cocos2d::ParticleSystem::Mode::GRAVITY) 
    {
        dict.emplace("speed", cocos2d::Value{data.speed});
        dict.emplace("speedVariance", cocos2d::Value{data.speedVar});
        dict.emplace("gravityx", cocos2d::Value{data.gravity.x});
        dict.emplace("gravityy", cocos2d::Value{data.gravity.y});
        dict.emplace("radialAcceleration", cocos2d::Value{data.radialAcceleration});
        dict.emplace("radialAccelVariance", cocos2d::Value{data.radialAccelerationVar});
        dict.emplace("tangentialAcceleration", cocos2d::Value{data.tangentialAcceleration});
        dict.emplace("tangentialAccelVariance", cocos2d::Value{data.tangentialAccelerationVar});
        dict.emplace("rotationIsDir", cocos2d::Value{data.rotationIsDirection});
    }
    else 
    {
        dict.emplace("maxRadius", cocos2d::Value{data.startRadius});
        dict.emplace("maxRadiusVariance", cocos2d::Value{data.startRadiusVar});
        dict.emplace("minRadius", cocos2d::Value{data.endRadius});
        dict.emplace("minRadiusVariance", cocos2d::Value{data.endRadiusVar});
        dict.emplace("rotatePerSecond", cocos2d::Value{data.rotationPerSecond});
        dict.emplace("rotatePerSecondVariance", cocos2d::Value{data.rotationPerSecondVar});
    }

    dict.emplace("particleLifespan", cocos2d::Value{data.lifeTime});
    dict.emplace("particleLifespanVariance", cocos2d::Value{data.lifeTimeVar});

    dict.emplace("startParticleSize", cocos2d::Value{data.startSize});
    dict.emplace("startParticleSizeVariance", cocos2d::Value{data.startSizeVar});
    dict.emplace("finishParticleSize", cocos2d::Value{data.endSize});
    dict.emplace("finishParticleSizeVariance", cocos2d::Value{data.endSizeVar});

    dict.emplace("rotationStart", cocos2d::Value{data.startSpin});
    dict.emplace("rotationStartVariance", cocos2d::Value{data.startSpinVar});
    dict.emplace("rotationEnd", cocos2d::Value{data.endSpin});
    dict.emplace("rotationEndVariance", cocos2d::Value{data.endSpinVar});
    
    dict.emplace("startColorRed", cocos2d::Value{data.startColor.r});
    dict.emplace("startColorGreen", cocos2d::Value{data.startColor.g});
    dict.emplace("startColorBlue", cocos2d::Value{data.startColor.b});
    dict.emplace("startColorAlpha", cocos2d::Value{data.startColor.a});
    
    dict.emplace("startColorVarianceRed", cocos2d::Value{data.startColorVar.r});
    dict.emplace("startColorVarianceGreen", cocos2d::Value{data.startColorVar.g});
    dict.emplace("startColorVarianceBlue", cocos2d::Value{data.startColorVar.b});
    dict.emplace("startColorVarianceAlpha", cocos2d::Value{data.startColorVar.a});
    
    dict.emplace("finishColorRed", cocos2d::Value{data.endColor.r});
    dict.emplace("finishColorGreen", cocos2d::Value{data.endColor.g});
    dict.emplace("finishColorBlue", cocos2d::Value{data.endColor.b});
    dict.emplace("finishColorAlpha", cocos2d::Value{data.endColor.a});
    
    dict.emplace("finishColorVarianceRed", cocos2d::Value{data.endColorVar.r});
    dict.emplace("finishColorVarianceGreen", cocos2d::Value{data.endColorVar.g});
    dict.emplace("finishColorVarianceBlue", cocos2d::Value{data.endColorVar.b});
    dict.emplace("finishColorVarianceAlpha", cocos2d::Value{data.endColorVar.a});

    dict.emplace("blendFuncSource", cocos2d::Value{blendIndexToGLenum(data.blendSrcIdx)});
    dict.emplace("blendFuncDestination", cocos2d::Value{blendIndexToGLenum(data.blendDstIdx)});

    const auto compressed = compressToGzip(data.textureImage->getData(), data.textureImage->getDataLen());

    char* encoded = nullptr;
    const auto encodedLen = cocos2d::base64Encode(reinterpret_cast<const unsigned char*>(compressed.data()), compressed.length(), &encoded);

    dict.emplace("textureImageData", cocos2d::Value{std::string{encoded, encodedLen}});
    std::free(encoded);

    if(!cocos2d::FileUtils::getInstance()->writeToFile(dict, path)) {
        // todo error writing
    }
}

void ParticleEditor::updatePropertiesFromSystem(ParticleSystemData& data)
{
    auto* ps = data.system;

    // emitter props
	data.emitDuration = ps->getDuration();
    data.emitPositionVariance = {ps->getPosVar().x, ps->getPosVar().y};
	data.maxParticles = ps->getTotalParticles();
	data.emitAngle = ps->getAngle();
	data.emitAngleVar = ps->getAngleVar();
	data.typeIdx = static_cast<int>(ps->getEmitterMode());

    if(ps->getEmitterMode() == cocos2d::ParticleSystem::Mode::GRAVITY) {
	    data.speed = ps->getSpeed();
	    data.speedVar = ps->getSpeedVar();
	    data.gravity = ps->getGravity();
	    data.radialAcceleration = ps->getRadialAccel();
	    data.radialAccelerationVar = ps->getRadialAccelVar();
	    data.tangentialAcceleration = ps->getTangentialAccel();
	    data.tangentialAccelerationVar = ps->getTangentialAccelVar();
	    data.rotationIsDirection = ps->getRotationIsDir();
    }
    else {
	    data.startRadius = ps->getStartRadius();
	    data.startRadiusVar = ps->getStartRadiusVar();
	    data.endRadius = ps->getEndRadius();
	    data.endRadiusVar = ps->getEndRadiusVar();
	    data.rotationPerSecond = ps->getRotatePerSecond();
	    data.rotationPerSecondVar = ps->getRotatePerSecondVar();
    }

	// particle props
	data.lifeTime = ps->getLife();
	data.lifeTimeVar = ps->getLifeVar();
	data.startSize = ps->getStartSize();
	data.startSizeVar = ps->getStartSizeVar();
	data.endSize = ps->getEndSize();
	data.endSizeVar = ps->getEndSizeVar();
	data.startSpin = ps->getStartSpin();
	data.startSpinVar = ps->getStartSpinVar();
	data.endSpin = ps->getEndSpin();
	data.endSpinVar = ps->getEndSpinVar();

    // color props
    data.startColor = ps->getStartColor();
    data.startColorVar = ps->getStartColorVar();
    data.endColor = ps->getEndColor();
    data.endColorVar = ps->getEndColorVar();
    data.blendSrcIdx = blendGLenumToIndex(ps->getBlendFunc().src);
    data.blendDstIdx = blendGLenumToIndex(ps->getBlendFunc().dst);
}

GLenum ParticleEditor::blendIndexToGLenum(const int idx)
{
    switch(idx) {
        default:
        case 0:
            return GL_ZERO;
        case 1:
            return GL_ONE;
        case 2:
            return GL_DST_COLOR;
        case 3:
            return GL_ONE_MINUS_DST_COLOR;
        case 4:
            return GL_SRC_ALPHA;
        case 5:
            return GL_ONE_MINUS_SRC_ALPHA;
        case 6:
            return GL_DST_ALPHA;
        case 7:
            return GL_ONE_MINUS_DST_ALPHA;
        case 8:
            return GL_SRC_ALPHA_SATURATE;
    }
}

int ParticleEditor::blendGLenumToIndex(const GLenum e)
{
	switch(e) {
        case GL_ZERO:
            return 0;
        case GL_ONE:
            return 1;
        case GL_DST_COLOR:
            return 2;
        case GL_ONE_MINUS_DST_COLOR:
            return 3;
        case GL_SRC_ALPHA:
            return 4;
        case GL_ONE_MINUS_SRC_ALPHA:
            return 5;
        case GL_DST_ALPHA:
            return 6;
        case GL_ONE_MINUS_DST_ALPHA:
            return 7;
        case GL_SRC_ALPHA_SATURATE:
            return 8;
        default:
            CCASSERT(false, "unimplemented blendfunc");
            return 0;
    }
}
