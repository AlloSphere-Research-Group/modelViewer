#ifndef MODELMANAGER_HPP
#define MODELMANAGER_HPP

#include "al/graphics/al_Graphics.hpp"
#include "al/io/al_File.hpp"
#include "al/graphics/al_Image.hpp"
#include "al/ui/al_Parameter.hpp"

#include "al_ext/assets3d/al_Asset.hpp"

#include <string>
#include <mutex>

using namespace al;

class ModelManager
{
public:

    // Shared state parameters
    ParameterString mModelFile{"ModelFile"};
    ParameterString mModelTexture{"ModelTexture"};
    ParameterBool mUseTexture{"UseTexture", "", 1.0};
    ParameterColor mColor{"Color"};

    ParameterBool autoRotate{"AutoRotate"};
    Parameter rotAngle{"RotationAngle", "", 0.0, "", 0.0f, 360.0f};

    std::mutex loadLock;
    Scene *ascene = nullptr;
    Vec3f scene_min, scene_max, scene_center;
    Texture tex;
    std::vector<Mesh> meshes;

    ModelManager();

    void loadModel(std::string fileName);

    void loadTexture(std::string fileName);

    void drawModel(Graphics &g);

private:
    std::string mModelToLoad;
    std::string mTextureToLoad;
};

#endif // MODELMANAGER_HPP
