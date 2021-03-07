#include "Render.h"

const char* modelDir = "Pasha_guard_head.obj";//obj资源路径
const char* texDir = "Pasha_guard_head_1.png";//材质路径
const int modelNum = 11;//实际数量是modelNum^2
const Real model_inter_scale = 1.5;//模型之间的距离比例

Render::Render()
    : ApplicationContext("Ogre")
{
}

void Render::setup()
{
    ApplicationContext::setup();
    addInputListener(this);

    Root* root = getRoot();
    scnMgr = root->createSceneManager();

    RTShader::ShaderGenerator* shadergen = RTShader::ShaderGenerator::getSingletonPtr();
    shadergen->addSceneManager(scnMgr);

    //Light
    scnMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
    light = scnMgr->createLight("MainLight");
    lightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
    lightNode->attachObject(light);
    lightNode->setPosition(20, 80, 100);

    //Camera
    camNode = scnMgr->getRootSceneNode()->createChildSceneNode();
    cam = scnMgr->createCamera("myCam");
    cam->setNearClipDistance(5);
    cam->setAutoAspectRatio(true);
    camNode->attachObject(cam);
    camNode->setPosition(0, 0, 1000);
    camNode->lookAt(Vector3(0, 0, 0),Ogre::Node::TransformSpace::TS_WORLD);
    getRenderWindow()->addViewport(cam);

    //excport model and initialize the OctTree
    object = new OctTree(modelDir);
    material = MaterialManager::getSingletonPtr()->create("modelMat", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    material->getTechnique(0)->getPass(0)->createTextureUnitState(texDir);
    material->getTechnique(0)->getPass(0)->setLightingEnabled(true);
    object->split();

    Vector3 min_pos = object->get_min();
    Vector3 max_pos = object->get_max();
    Real x_distance = max_pos.x - min_pos.x;
    Real y_distance = max_pos.y - min_pos.y;

    //input information(frustum)
    std::vector<Real> v;
    v.resize(4);
    std::cout << "Input the real number range of frustum(left,right,top,bottom):" << std::endl << "Tips: [0,1] refers to the origin frustum of this camera." << std::endl;
    std::cin >> v[0] >> v[1] >> v[2] >> v[3];

    modelNode = new SceneNode* [modelNum * modelNum];
    
    Vector3 model_pos,lookat_y;
    model_pos.z = 0;
    lookat_y = Vector3(0, -1, 0);
    std::cout << "Model searching starts." << std::endl;
    clock_t c1 = clock();
    for (int i = 0; i < modelNum; i++) {
        for (int j = 0; j < modelNum; j++) {
            modelNode[modelNum * i + j] = scnMgr->getRootSceneNode()->createChildSceneNode();
            model_pos.x = x_distance * model_inter_scale * (i - (modelNum / 2));
            model_pos.y = y_distance * model_inter_scale * (j - (modelNum / 2));
            modelNode[modelNum * i + j]->setPosition(model_pos);
            set_viewport(v, modelNode[modelNum * i + j]);

            double** coef;
            coef = new double* [6];
            for (int i = 0; i < 6; i++)
                coef[i] = new double[5];
            toRender = object->search(&frus,coef);
            for (int i = 0; i < 6; i++)
                delete[] coef[i];
            delete[] coef;

            show_obj = scnMgr->createManualObject();
            show_obj->clear();
            show_obj->begin("modelMat", RenderOperation::OT_TRIANGLE_LIST);
            for (int i = 0; i < toRender.size(); i++) {
                show_obj->position(toRender[i].A);
                show_obj->textureCoord(toRender[i].tA);
                show_obj->position(toRender[i].B);
                show_obj->textureCoord(toRender[i].tB);
                show_obj->position(toRender[i].C);
                show_obj->textureCoord(toRender[i].tC);
            }
            show_obj->end();

            modelNode[modelNum * i + j]->attachObject(show_obj);
        }
    }
    clock_t c2 = clock();
    std::cout << "Model searching ends. Using time is: " << double((c2 - c1)) / CLOCKS_PER_SEC << std::endl;

    frame_num = 0;
    frame_time = clock();
}

void Render::set_viewport(std::vector<Real> v,SceneNode* s_node) {
    Real origin_fovy = cam->getFOVy().valueRadians();
    Real height = cam->getNearClipDistance() * tan(origin_fovy / 2);
    origin_fru[2] = height;
    origin_fru[3] = -height;
    Real origin_aspect = cam->getAspectRatio();
    origin_fru[0] = -height * origin_aspect;
    origin_fru[1] = height * origin_aspect;
    Vector3 origin_lookat = Vector3(0, 0, 0);

    show_fru = Vector4(origin_fru[0] + (origin_fru[1] - origin_fru[0]) * v[0], origin_fru[0] + (origin_fru[1] - origin_fru[0]) * v[1],
        origin_fru[3] + (origin_fru[2] - origin_fru[3]) * v[2], origin_fru[3] + (origin_fru[2] - origin_fru[3]) * v[3]);
    cam->setFrustumExtents(show_fru[0], show_fru[1], show_fru[2], show_fru[3]);

    Real fovy_upper = atan(show_fru[2] / cam->getNearClipDistance());
    Real fovy_lower = atan(show_fru[3] / cam->getNearClipDistance());
    Real fovy_h_middle = (fovy_lower + fovy_upper) / 2;
    Real show_lookat_y = cam->getNearClipDistance() * tan(fovy_h_middle);
    Real fovy_lefter = atan(show_fru[0] / cam->getNearClipDistance());
    Real fovy_righter = atan(show_fru[1] / cam->getNearClipDistance());
    Real fovy_w_middle = (fovy_lefter + fovy_righter) / 2;
    Real show_lookat_x = cam->getNearClipDistance() * tan(fovy_w_middle);
    Vector3 show_lookat = Vector3(show_lookat_x, show_lookat_y, camNode->getPosition().z - cam->getNearClipDistance());
    camNode->lookAt(show_lookat, Ogre::Node::TransformSpace::TS_WORLD);

    //修正到以模型为原点
    frus.camPos = camNode->getPosition() - s_node->getPosition();
    frus.camUp = cam->getRealUp();
    frus.lookAt = show_lookat - s_node->getPosition();
    frus.zNear = cam->getNearClipDistance();
    frus.zFar = cam->getFarClipDistance();
    frus.aspect = cam->getAspectRatio();
    frus.fovy = fovy_upper - fovy_lower;

    cam->setFrustumExtents(origin_fru[0], origin_fru[1], origin_fru[2], origin_fru[3]);
    camNode->lookAt(origin_lookat, Ogre::Node::TransformSpace::TS_WORLD);
}

bool Render::keyPressed(const KeyboardEvent& evt)
{
    if (evt.keysym.sym == SDLK_ESCAPE)
    {
        getRoot()->queueEndRendering();
    }
    if (evt.keysym.sym == SDLK_RIGHT)
    {
        camNode->rotate(Vector3(0, 1, 0), -Radian(3.1415926F / 180));
    }
    if (evt.keysym.sym == SDLK_LEFT)
    {
        camNode->rotate(Vector3(0, 1, 0), Radian(3.1415926F / 180));
    }
    if (evt.keysym.sym == SDLK_UP)
    {
        camNode->rotate(Vector3(1, 0, 0), Radian(3.1415926F / 180));
    }
    if (evt.keysym.sym == SDLK_DOWN)
    {
        camNode->rotate(Vector3(1, 0, 0), -Radian(3.1415926F / 180));
    }
    if (evt.keysym.sym == SDLK_F1)
    {
        camNode->translate(Vector3(0, 0, -3));
    }
    if (evt.keysym.sym == SDLK_F2)
    {
        camNode->translate(Vector3(0, 0, 3));
    }
    if (evt.keysym.sym == SDLK_F3)
    {
        camNode->translate(Vector3(0, 3, 0));
    }
    if (evt.keysym.sym == SDLK_F4)
    {
        camNode->translate(Vector3(0, -3, 0));
    }
    if (evt.keysym.sym == SDLK_F5)
    {
        camNode->translate(Vector3(-3, 0, 0));
    }
    if (evt.keysym.sym == SDLK_F6)
    {
        camNode->translate(Vector3(3, 0, 0));
    }
    return true;
}

void Render::frameRendered(const FrameEvent& evt) {
    frame_num++;
    if (frame_num >= 500) {
        clock_t now_time = clock();
        double time = double((now_time - frame_time)) / CLOCKS_PER_SEC;
        double fps = frame_num / time;
        std::cout << "Now FPS: " << fps << std::endl;

        frame_num = 0;
        frame_time = now_time;
    }
}

int main(int argc, char **argv)
{
    try
    {
    	Render app;
        app.initApp();
        app.getRoot()->startRendering();
        app.closeApp();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error occurred during execution: " << e.what() << '/n';
        return 1;
    }

    return 0;
}

