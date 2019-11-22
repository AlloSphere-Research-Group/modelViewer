

#include "al/app/al_DistributedApp.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/ui/al_FileSelector.hpp"
#include "al/ui/al_ParameterGUI.hpp"

#include "al_ext/openvr/al_OpenVRDomain.hpp"

#include "Gamma/Oscillator.h"

#include "modelmanager.hpp"

#include <iostream>
#include <memory>
#include <vector>

using namespace al;

struct State {
  float a = 0.f; // current rotation angle
};

class MyApp : public DistributedAppWithState<State> {
public:
  ParameterBool mVr{"VR", "", 0.0};
  ParameterGUI gui;
  FileSelector fileSelectorModel;
  FileSelector fileSelectorTexture;

  SearchPaths searchpaths;

  std::shared_ptr<OpenVRDomain> vrDomain;

  ModelManager modelManager;

  MyApp() : DistributedAppWithState<State>() {}

  void onInit() override {
    oscDomain()->parameterServer()
        << modelManager.mModelFile << modelManager.mModelTexture
        << modelManager.mColor << modelManager.mUseTexture
        << modelManager.autoRotate << modelManager.rotAngle;
  }

  void onCreate() override {
    mVr.registerChangeCallback([&](float value) {
      if (value != 0.0f) {
        vrDomain = OpenVRDomain::enableVR(this);
        vrDomain->setDrawFunction(
            std::bind(&MyApp::drawVr, this, std::placeholders::_1));
      } else {
        OpenVRDomain::disableVR(this, vrDomain);
      }
    });
    if (rank == 0) {
      modelManager.mModelFile.set("data/ducky.obj");
      modelManager.mModelTexture.set("data/hubble.jpg");
    }

    imguiInit();
  }

  void onAnimate(double /*dt*/) override {
    if (isPrimary() && (modelManager.autoRotate != 0.0f)) {
      float newAngle = modelManager.rotAngle + 0.5f;
      if (newAngle > 360) {
        newAngle -= 360;
      }
      modelManager.rotAngle = newAngle;
    }
  }

  //  void onMessage(osc::Message &m) override { m.print(); }

  void onExit() override { imguiShutdown(); }

  void drawVr(Graphics &g) {
    // HMD scene will have blueish background
    g.clear(0.0, 0.3f, 0);
    modelManager.drawModel(g);
  }

  void drawGui() {

    imguiBeginFrame();
    ParameterGUI::beginPanel("OBJ loader");
#ifdef AL_EXT_OPENVR
    // Only show VR button if there is support for OpenVR
    ParameterGUI::draw(&mVr);
#endif
    ParameterGUI::draw(&modelManager.mUseTexture);
    ParameterGUI::draw(&modelManager.autoRotate);
    ParameterGUI::draw(&modelManager.rotAngle);
    if (modelManager.mUseTexture == 0.0f) {
      ParameterGUI::draw(&modelManager.mColor);
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
        modelManager.mModelFile.set(selection[0].filepath());
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
        modelManager.mModelTexture.set(selection[0].filepath());
      }
    }

    ParameterGUI::endPanel();
    imguiEndFrame();
  }

  void onDraw(Graphics &g) override {
    if (!hasCapability(Capability::CAP_OMNIRENDERING)) {
      navControl().active(!ParameterGUI::usingInput());
    }

    // Dekstop scene will have reddish background and gui.
    g.clear(0.3f, 0, 0);
    modelManager.drawModel(g);
    if (hasCapability(Capability::CAP_RENDERING)) {
      drawGui();
      imguiDraw();
    }
  }
};

int main() {

  MyApp app;
  app.start();

  return 0;
}
