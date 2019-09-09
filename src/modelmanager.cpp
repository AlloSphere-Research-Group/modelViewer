#include "modelmanager.hpp"

ModelManager::ModelManager() {

  mModelFile.registerChangeCallback(
      [&](std::string model) { loadModel(model); });
  mModelTexture.registerChangeCallback(
      [&](std::string texture) { loadTexture(texture); });
}

void ModelManager::loadModel(std::string fileName) {
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

void ModelManager::loadTexture(std::string fileName) {
  loadLock.lock();

  fileName = File::conformPathToOS(fileName);
  auto imageData = Image(fileName);
  if (imageData.array().size() != 0) {
    std::cout << "loaded image size: " << imageData.width() << ", "
              << imageData.height() << std::endl;
    if (tex.created()) {
      tex.destroy();
    }
    tex.create2D(imageData.width(), imageData.height());
    tex.submit(imageData.array().data(), GL_RGBA, GL_UNSIGNED_BYTE);
  } else {
    std::cout << "failed to load image " << fileName << std::endl;
  }
  loadLock.unlock();
}

void ModelManager::drawModel(Graphics &g) {
  loadLock.lock();

  g.depthTesting(true);
  g.lighting(true);
  // g.light().dir(1.f, 1.f, 2.f);

  g.pushMatrix();
  g.translate(0, 0, -4.0);

  // rotate it around the y axis
  g.rotate(rotAngle, 0.f, 1.f, 0.f);

  // scale the whole asset to fit into our view frustum
  float tmp = scene_max[0] - scene_min[0];
  tmp = std::max(scene_max[1] - scene_min[1], tmp);
  tmp = std::max(scene_max[2] - scene_min[2], tmp);
  tmp = 2.f / tmp;
  g.scale(tmp);

  // center the model
  g.translate(-scene_center);

  bool useTexture = mUseTexture == 1.0f;

  if (useTexture) {
    tex.bind(0);
    g.texture(); // use texture to color the mesh
  } else {
    g.color(mColor);
  }
  // draw all the meshes in the scene
  for (auto &m : meshes) {
    g.draw(m);
  }
  if (useTexture) {
    tex.unbind(0);
  }

  g.popMatrix();
  loadLock.unlock();
}
