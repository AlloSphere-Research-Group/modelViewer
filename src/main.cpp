#include <memory>
#include <iostream>
#include <vector>

#include "al/util/ui/al_ParameterGUI.hpp"
#include "al/util/ui/al_FileSelector.hpp"
#include "al/core/graphics/al_Shapes.hpp"

#include "al_ext/distributed/al_App.hpp"
#include "al_ext/assets3d/al_Asset.hpp"
#include "module/img/loadImage.hpp"

#include "Gamma/Oscillator.h"

using namespace al;


class MyApp: public BaseCompositeApp {
public:
  ParameterBool mVr {"VR", "", 0.0};
  ParameterBool mUseTexture {"UseTexture", "", 1.0};
  ParameterColor mColor {"Color"};
  ParameterGUI gui;
  FileSelector fileSelectorModel;
  FileSelector fileSelectorTexture;

  // Shared state parameters
  ParameterString mModelFile {"ModelFile"};
  ParameterString mModelTexture {"ModelTexture"};

  SearchPaths searchpaths;
  std::mutex loadLock;
  Scene* ascene = nullptr;
  Vec3f scene_min, scene_max, scene_center;
  Texture tex;
  std::vector<Mesh> meshes;
  float a = 0.f;  // current rotation angle

  void onInit() {
    oscDomain()->parameterServer() << mModelFile << mModelTexture;
  }

  void onCreate() {
    mVr.registerChangeCallback([&](float value) {
      if (value != 0.0) {
        enableVR();
        setOpenVRDrawFunction(std::bind(&MyApp::drawVr, this, std::placeholders::_1));
      } else {
        disableVR();
      }
    });

    loadModel("data/ducky.obj");
    loadTexture("data/hubble.jpg");

    ParameterGUI::initialize();
  }

  void drawScene(Graphics &g) {
    loadLock.lock();
    // This function is used by both the VR and the desktop draw calls.

//    g.clear(0.1);

    g.depthTesting(true);
    g.lighting(true);
    //g.light().dir(1.f, 1.f, 2.f);

    g.pushMatrix();
    g.translate(0, 0, -4.0);

    // rotate it around the y axis
    g.rotate(a, 0.f, 1.f, 0.f);
    a += 0.5;

    // scale the whole asset to fit into our view frustum
    float tmp = scene_max[0] - scene_min[0];
    tmp = std::max(scene_max[1] - scene_min[1], tmp);
    tmp = std::max(scene_max[2] - scene_min[2], tmp);
    tmp = 2.f / tmp;
    g.scale(tmp);

    // center the model
    g.translate(-scene_center);

    bool useTexture = mUseTexture == 1.0f;

    if (useTexture == 1.0f) {
      tex.bind(0);
      g.texture(); // use texture to color the mesh
    } else {
      g.color(mColor);
    }
    // draw all the meshes in the scene
    for (auto& m : meshes) {
        g.draw(m);
    }
    if (useTexture == 1.0f) {
      tex.unbind(0);
    }

    g.popMatrix();
    loadLock.unlock();
  }

  void drawVr(Graphics &g) {
    // HMD scene will have blueish background
    g.clear(0.0, 0.3, 0);
    drawScene(g);
  }

  void drawGui() {

    ParameterGUI::beginDraw();
    ParameterGUI::beginPanel("OBJ loader");
#ifdef AL_EXT_OPENVR
    // Only show VR button if there is support for OpenVR
    ParameterGUI::draw(&mVr);
#endif
    ParameterGUI::draw(&mUseTexture);
    if (mUseTexture == 0.0f) {
      ParameterGUI::draw(&mColor);
    }

    if (ImGui::Button("Select Model")) {
      if (fileSelectorModel.isActive()) {
        fileSelectorModel.cancel();
      } else {
        fileSelectorModel.start(".");
      }
    }
    if (fileSelectorModel.drawFileSelector()) {
      auto selection = fileSelectorModel.getSelection();
      if (selection.count() > 0) {
        loadModel(selection[0].filepath());

      }
    }
    if (ImGui::Button("Select Texture")) {
      if (fileSelectorTexture.isActive()) {
        fileSelectorTexture.cancel();
      } else {
        fileSelectorTexture.start(".");
      }
    }
    if (fileSelectorTexture.drawFileSelector()) {
      auto selection = fileSelectorTexture.getSelection();
      if (selection.count() > 0) {
        loadTexture(selection[0].filepath());
      }
    }

    ParameterGUI::endPanel();
    ParameterGUI::endDraw();
  }

  void onDraw(Graphics &g) override {
    graphicsDomain()->navControl().active(!ParameterGUI::usingInput());

    // Dekstop scene will have reddish background and gui.
    g.clear(0.3, 0, 0);
    drawScene(g);

    drawGui();
  }

  void onExit() override {

    ParameterGUI::cleanup();
  }

  void loadModel(std::string fileName) {
    loadLock.lock();
    // load in a "scene"
    ascene = Scene::import(fileName);
    if (ascene == nullptr) {
      printf("error reading %s\n", fileName.c_str());
      loadLock.unlock();
      return;
    } else {
      ascene->getBounds(scene_min, scene_max);
      scene_center = (scene_min + scene_max) / 2.f;
      ascene->print();
    }
    // extract meshes from scene
    meshes.clear();
    meshes.resize(ascene->meshes());
    for (int i = 0; i < ascene->meshes(); i += 1) {
        ascene->mesh(i, meshes[i]);
    }
    loadLock.unlock();
  }

  void loadTexture(std::string fileName) {
    loadLock.lock();

    auto imageData = imgModule::loadImage(File::conformPathToOS(fileName));
    if (imageData.data.size() != 0) {
      std::cout << "loaded image size: " << imageData.width << ", " << imageData.height << std::endl;
      if (tex.created()) { tex.destroy(); }
      tex.create2D(imageData.width, imageData.height);
      tex.submit(imageData.data.data(), GL_RGBA, GL_UNSIGNED_BYTE);
    } else {
      std::cout << "failed to load image " << fileName << std:: endl;
    }
    loadLock.unlock();
  }

};

int main(int argc, char *argv[])
{
  MyApp app;
  app.start();


  return 0;
}
