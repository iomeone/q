#ifndef SCENARIO_H
#define SCENARIO_H

#include "support.h"
#include "gl_helpers.h"

struct IScenario {
public:
  virtual ~IScenario() {}
  virtual void startLoading();
  virtual bool loadDependencies();
  virtual bool finishLoading();
  virtual void think(glm::vec2 dir, double pitch, double yaw);
  virtual void render();
};

struct TestScenario : IScenario {
public:
  void startLoading() override;
  bool loadDependencies() override;
  bool finishLoading() override;
  void think(glm::vec2 dir, double pitch, double yaw) override {}
  void render() override;

private:
  static const int SHADER_ID = 1;
  static const int TEXTURE_ID = 2;

  static constexpr float vertices[] = {
    // Position   Texcoords
    -0.5f,  0.5f, 0.0f, 0.0f, // Top-left
    0.5f,  0.5f, 1.0f, 0.0f, // Top-right
    0.5f, -0.5f, 1.0f, 1.0f, // Bottom-right
    -0.5f, -0.5f, 0.0f, 1.0f  // Bottom-left
  };

  static constexpr GLuint elements[] = {
    0, 1, 2,
    2, 3, 0
  };
};

struct Camera {
public:
  glm::vec3 location;
  double pitch;
  double yaw;

  glm::vec3 forward();
  glm::vec3 right();
  glm::vec3 upApproximate();
};

struct RenderableBSP;

struct BSPScenario : IScenario {
public:
  void startLoading() override;
  bool loadDependencies() override;
  bool finishLoading() override;
  void think(glm::vec2 dir, double pitch, double yaw) override;
  void render() override;

private:
  static const int BSP_ID = 0;
  static const int SCENE_SHADER_ID = 1;
  static const int BLEND_SHADER_ID = 2;

  shared_ptr<RenderableBSP> _renderableMap = nullptr;

  unordered_map<int, GLuint> _lightmapTextures;
  GLuint _fallbackLightmapTexture;

  // For rendering with the scene shader
  vector<VBO> _verticesPerFace;
  vector<VBO> _colorsPerFace;
  vector<EBO> _elementsPerFace;

  // For rendering the blend shader

  // For non-transparent parts of the scene.
  GLuint _sceneFBO;
  GLuint _sceneTexture;

  // For transparent parts of the scene.
  GLuint _effectsFBO;
  GLuint _effectsTexture;

  Camera _camera;
};

#endif