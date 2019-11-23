#include "modelmanager.hpp"

ModelManager::ModelManager() {

  mModelFile.registerChangeCallback(
      [&](std::string model) {
    std::cout << "Loading model: " << model <<std::endl;
    loadModel(model);
  });
  mModelTexture.registerChangeCallback(
      [&](std::string texture) { loadTexture(texture); });

      parentPickable.containChildren = true;

}

void ModelManager::loadModel(std::string fileName) {
  loadLock.lock();
  mModelToLoad = fileName;
  loadLock.unlock();
}

void ModelManager::loadTexture(std::string fileName) {
  loadLock.lock();
  mTextureToLoad = fileName;
  loadLock.unlock();
}

void ModelManager::drawModel(Graphics &g) {
  loadLock.lock();
  std::string fileName = mModelToLoad;
  std::string textureFileName = mTextureToLoad;
  loadLock.unlock();

  if (fileName.size() > 0) {
    loadLock.lock();
    mModelToLoad = "";
    loadLock.unlock();
    // load in a "scene"
    ascene = Scene::import(fileName);
    if (ascene == nullptr) {
      printf("error reading %s\n", fileName.c_str());
      
    } else {
      ascene->getBounds(scene_min, scene_max);
      scene_center = (scene_min + scene_max) / 2.f;
      ascene->print();
      // Remove child pickables from parent pickable
      for (auto *p : parentPickable.children) {
        delete p;
      }
      parentPickable.children.clear();
      // extract meshes from scene
      meshes.clear();
      meshes.resize(ascene->meshes());
      for (int i = 0; i < ascene->meshes(); i += 1) {
        ascene->mesh(i, meshes[i]);
        PickableBB *child = new PickableBB;
        child->set(meshes[i]);
        parentPickable.addChild(child);
      }
    }

  }

  if (textureFileName.size() > 0) {
    loadLock.lock();
    mTextureToLoad = "";
    loadLock.unlock();

    textureFileName = File::conformPathToOS(textureFileName);
    auto imageData = Image(textureFileName);
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
  }

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
