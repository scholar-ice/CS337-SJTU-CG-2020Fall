#pragma once

#include "OctTree.h"
#include "Ogre.h"
#include "OgreApplicationContext.h"
#include "OgreInput.h"
#include "OgreRTShaderSystem.h"
#include <iostream>
#include <string>
#include <time.h>

using namespace Ogre;
using namespace OgreBites;

class Render
    : public ApplicationContext
    , public InputListener
{
public:
    Render();
    virtual ~Render() {}

    void setup();
    bool keyPressed(const KeyboardEvent& evt);
    void frameRendered(const FrameEvent& evt);

    void set_viewport(std::vector<Real> v,SceneNode* s_node);

private:
    //每500帧统计一次
    int frame_num;
    clock_t frame_time;

    OctTree* object;
    ManualObject* show_obj;
    MaterialPtr material;
    std::vector<triangle> toRender;
    SceneNode** modelNode;

    SceneManager* scnMgr;
    Light* light;
    SceneNode* lightNode;
    Camera* cam;
    SceneNode* camNode;
    Vector4 origin_fru;
    Vector4 show_fru;
    frustum frus;

};