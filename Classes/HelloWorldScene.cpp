/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include <cocos/base/firePngData.h>
#include <array>
#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "CCImGuiLayer.h"
#include "CCIMGUI.h"
#include "cocos/2d/CCParticleSystemQuad.h"

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

static Texture2D* getDefaultTexture()
{
    Texture2D* texture = nullptr;
    Image* image = nullptr;
    do
    {
        const std::string key = "/__firePngData";
        texture = Director::getInstance()->getTextureCache()->getTextureForKey(key);
        CC_BREAK_IF(texture != nullptr);

        image = new (std::nothrow) Image();
        CC_BREAK_IF(nullptr == image);
        bool ret = image->initWithImageData(__firePngData, sizeof(__firePngData));
        CC_BREAK_IF(!ret);

        texture = Director::getInstance()->getTextureCache()->addImage(image, key);
    } while (0);

    CC_SAFE_RELEASE(image);

    return texture;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

    if (closeItem == nullptr ||
        closeItem->getContentSize().width <= 0 ||
        closeItem->getContentSize().height <= 0)
    {
        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
    }
    else
    {
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width/2;
        float y = origin.y + closeItem->getContentSize().height/2;
        closeItem->setPosition(Vec2(x,y));
    }

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label

    auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);
    if (label == nullptr)
    {
        problemLoading("'fonts/Marker Felt.ttf'");
    }
    else
    {
        // position the label on the center of the screen
        label->setPosition(Vec2(origin.x + visibleSize.width/2,
                                origin.y + visibleSize.height - label->getContentSize().height));

        // add the label as a child to this layer
        this->addChild(label, 1);
    }

    // add "HelloWorld" splash screen"
    auto sprite = Sprite::create("HelloWorld.png");
    if (sprite == nullptr)
    {
        problemLoading("'HelloWorld.png'");
    }
    else
    {
        // position the sprite on the center of the screen
        sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

        // add the sprite as a child to this layer
        this->addChild(sprite, 0);
    }

    std::string layerName = "ImGUILayer";
    auto order = INT_MAX;
    auto layer = ImGuiLayer::create();
    addChild(layer, order, layerName);


// add ui callbacks

    auto ps = ParticleFireworks::create();

    addChild(ps, 10);

    CCIMGUI::getInstance()->addCallback([=](){
        ImGui::Text("Texture path: %s", ps->getTexture()->getPath().c_str());
        ImGui::Text("isFull %d", ps->isFull());

        if(ImGui::BeginTabBar("#tab"))
        {
            if(ImGui::BeginTabItem("Emitter Settings"))
            {
                static float s_duration = ps->getDuration();
                if(ImGui::InputFloat("Duration", &s_duration, 0.01f, 1.0f))
                {
                    ps->setDuration(s_duration);
                    ps->resetSystem();
                }

                static std::array<int, 2> sourcePosition{(int)ps->getPosVar().x, (int)ps->getPosVar().y};
                if(ImGui::SliderInt2("Source Position Variance", sourcePosition.data(), 0, 2048))
                {
                    ps->setPosVar(Vec2(sourcePosition[0],sourcePosition[1]));
                }

                static int s_maximum_particles = ps->getTotalParticles();
                if(ImGui::InputInt("Maximum Particles", &s_maximum_particles, 0, 2000))
                {
                    ps->setTotalParticles(s_maximum_particles);
                }

                static float s_emit_angle = ps->getAngle();
                if(ImGui::SliderFloat("Emit Angle", &s_emit_angle, 0.f, 360.f))
                {
                    ps->setAngle(s_emit_angle);
                }

                static float s_emit_angle_variance = ps->getAngleVar();
                if(ImGui::SliderFloat("Emit Angle Variance", &s_emit_angle_variance, 0.f, 360.f))
                {
                    ps->setAngleVar(s_emit_angle_variance);
                }

                static int current_type = (int)ps->getEmitterMode();
                static constexpr std::array<const char*,2> types{"Gravity", "Radial"};
                if(ImGui::Combo("Type", &current_type, types.data(),2))
                {
                    ps->setEmitterMode(static_cast<ParticleSystem::Mode >(current_type));
                }

                static std::array<float, 2> s_gravity{ps->getGravity().x, ps->getGravity().y};
                if(ImGui::SliderFloat2("gravity", s_gravity.data(), -1000.f, 1000.f))
                {
                    ps->setGravity(Vec2{s_gravity.data()});
                }


                //ImGui::Text("speed %f", ps->getSpeed());
//                ImGui::Text("speed var %f", ps->getSpeedVar());
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("Particle Settings"))
            {
                ImGui::EndTabItem();
            }

            if(ImGui::BeginTabItem("Color Settings"))
            {
                ImGui::EndTabItem();
            }

            if(ImGui::BeginTabItem("Texture Settings"))
            {
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

    }, "hello");

    return true;
}



void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);


}
