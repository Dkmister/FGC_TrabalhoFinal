#include "stdafx.h"
#include "HeightScene.h"

#include "Matrices.h"

HeightScene::HeightScene() {
   // start the sound engine with default parameters
   _engine = irrklang::createIrrKlangDevice();
   _engine->setDefault3DSoundMinDistance(10.f);

   if (!_engine) {
      printf("Could not startup sound engine\n");
   }

   irrklang::vec3df pos3d(0.0f, 100.0f, 100.0f);
   _music = _engine->play3D("./resources/media/ophelia.mp3", pos3d, true, false, true);

   if (!_music) { 
      printf("Could not load sound\n");
   }

   //////////////////////////////////////////////////////////////////////////////////////

   _heightmap.reset(new HeightMap("./resources/heightmap/texture.bmp"));
   _heightmap->load();

   //////////////////////////////////////////////////////////////////////////////////////

   GLuint vertexLocation = 0, normalLocation = 1, texcoordLocation = 2;

   _shader.reset(new glsl::ShaderProgram("phong",
      glsl::VertexShader("./resources/phong/phong.vert"), 
      glsl::FragmentShader("./resources/phong/phong.frag")));

   _shader->bindAttribLocation(vertexLocation, "in_vertex");
   _shader->bindAttribLocation(normalLocation, "in_normal");
   _shader->bindAttribLocation(texcoordLocation, "in_texcoord");
   
   //////////////////////////////////////////////////////////////////////////////////////

   _lights.reset(new LightManager(1));
   _lights->getLight(0)->position = glm::vec4(0.0f, 30.0f, 100.0f, 1.0);
   _lights->getLight(0)->spot_direction = glm::vec4(0.0f, 30.0f, 99.0f, 0.0);
   _lights->getLight(0)->ambient  = glm::vec4(0.5,  0.5,  0.5, 1.0);
   _lights->getLight(0)->diffuse  = glm::vec4(1.0,  1.0,  1.0, 1.0);
   _lights->getLight(0)->specular = glm::vec4(1.0,  1.0,  1.0, 1.0);
   _lights->getLight(0)->spotCutoff   = 180.0;
   _lights->getLight(0)->spotExponent =  0.0;
   _lights->getLight(0)->constantAttenuation  =  1.0f;
   _lights->getLight(0)->linearAttenuation    =  0.0f;
   _lights->getLight(0)->quadraticAttenuation =  0.0f;
   
   
   //////////////////////////////////////////////////////////////////////////////////////
   
   _mesh.reset(new SkinnedMesh());
   _mesh->LoadMesh("./resources/scenes/bob/boblampclean.md5mesh");
   
   //////////////////////////////////////////////////////////////////////////////////////

   _camera.reset(new FirstPersonCamera(glm::vec3(0.0f, 100.0f, 100.0f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.0f, 1.0f, 0.0f)));
   //_camera.reset(new ThirdtPersonCamera(glm::vec3(0.0f, 100.0f, 100.0f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.0f, 1.0f, 0.0f)));
}

HeightScene::~HeightScene() {
   _music->drop(); // release music stream.
   _engine->drop(); // delete engine
}

void HeightScene::render(GLFWwindow* window) {
   _camera->computeFromInputs(window);

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glCullFace(GL_BACK);

   int width, height;
   glfwGetWindowSize(window, &width, &height);

   glm::mat4 Projection = glm::perspective(60.f, width / (float)height, 0.1f, 1000.0f);
   glm::mat4 View = _camera->getMatrix();
   glm::mat4 Model = glm::mat4(1.0);

   //////////////////////////////////////////////////////////////////////////////////////   

   _heightmap->render(*_camera, Projection, true);

   //////////////////////////////////////////////////////////////////////////////////////   
   
   _shader->use();
   
   _lights->updateData(*_shader, _camera->getMatrix());

   _music->setPosition(irrklang::vec3df(
      _heightmap->getHeightFromVertexData(100,100).x, 
      _heightmap->getHeightFromVertexData(100,100).y, 
      _heightmap->getHeightFromVertexData(100,100).z)
   ); 
   
   Model = glm::translate(Model, _heightmap->getHeightFromVertexData(100,100));         
   Model = glm::scale(Model, glm::vec3(50.f));

   Model = glm::translate(Model, glm::vec3(0.f, 0.4f, 0.f));
   Model = glm::rotate(Model, 270.f, glm::vec3(1.f, 0.f, 0.f));
   Model = _mesh->translate_to_origin_and_scale(Model);

   util::updateMVP(*_shader, Model, View, Projection);

   _mesh->boneTransform(*_shader, glfwGetTime());
   _mesh->render(*_shader);

   _shader->unuse();   

   //////////////////////////////////////////////////////////////////////////////////////

   _engine->setListenerPosition(
      // Position of the camera or listener
      irrklang::vec3df(_camera->getEye().x, _camera->getEye().y, _camera->getEye().z),
      // Direction vector where the camera or listener is looking into
      irrklang::vec3df(_camera->getCenter().x, _camera->getCenter().y, _camera->getCenter().z),
      // The velocity per second describes the speed of the listener and is only needed for Doppler effects
      irrklang::vec3df(0,0,0),
      // Vector pointing 'up', so the engine can decide where is left and right
      irrklang::vec3df(_camera->getUp().x, _camera->getUp().y, _camera->getUp().z)
      );

   if (glfwGetKey(window, 'B') == GLFW_PRESS) {
      _engine->play2D("./resources/media/bell.wav");
   }   
}
