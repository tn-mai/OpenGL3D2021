/**
* @file PlayerActor.cpp
*/
#include "PlayerActor.h"
#include "../GameEngine.h"
#include "../Audio.h"
#ifdef USE_EASY_AUDIO
#include "../EasyAudioSettings.h"
#else
#include "../Audio/MainWorkUnit/SE.h"
#endif // USE_EASY_AUDIO
#include <glm/gtc/matrix_transform.hpp>

/**
* �R���X�g���N�^
*/
PlayerActor::PlayerActor(
  const glm::vec3& position,
  const glm::vec3& scale,
  float rotation) :
  Actor(
    "Tiger-I",
    GameEngine::Get().LoadMesh("Res/tank/Tiger_I.obj"),
    position, scale, rotation, glm::vec3(0))
{
  health = 10;
  //collider = Box::Create(glm::vec3(-1.8f, 0, -1.8f), glm::vec3(1.8f, 2.8f, 1.8f));
  collider = Cylinder::Create(glm::vec3(0), 1.8f, 2.5f);
  mass = 57'000;
  //cor = 0.1f;
  //friction = 1.0f;

  MeshRenderer& meshRenderer = static_cast<MeshRenderer&>(*renderer);
  MeshPtr mesh = meshRenderer.GetMesh();
  if (mesh) {
    // �e�q�֌W��ݒ�
    Mesh::Group& gun = mesh->groups[gunGroup];
    gun.parent = turretGroup;

    // �t�o�C���h�|�[�Y�s���ݒ�
    gun.matInverseBindPose =
      glm::translate(glm::mat4(1), glm::vec3(0, -2.2f, -1.1f));

    // �t�o�C���h�|�[�Y�s�񂩂�o�C���h�|�[�Y�s����v�Z
    gun.matBindPose = glm::inverse(gun.matInverseBindPose);
  }
}

/**
* �A�N�^�[�̏�Ԃ��X�V����
*
* @param deltaTime �O��̍X�V����̌o�ߎ���(�b)
*/
void PlayerActor::OnUpdate(float deltaTime)
{
  GameEngine& engine = GameEngine::Get();

  // �e�̃C���X�^���V���O�p�����_�����쐬
  if (!bulletRenderer) {
    bulletRenderer = std::make_shared<InstancedMeshRenderer>(100);
    bulletRenderer->SetMesh(engine.LoadMesh("Res/Bullet.obj"));
    bulletRenderer->SetMaterial(0, { "bullet", glm::vec4(1), engine.LoadTexture("Res/Bullet.tga") });
    std::shared_ptr<Actor> p = std::make_shared<Actor>("BulletInstancedActor");
    p->renderer = bulletRenderer;
    p->shader = Shader::InstancedMesh;
    engine.AddActor(p);
  }

  // �C���X�^���X�̃��f���s����X�V
  bulletRenderer->UpdateInstanceTransforms();

  // ���[�U�[������󂯕t���Ȃ��Ƃ��͉������Ȃ�
  if (!isControlable) {
    oldShotButton = 0;
#ifdef USE_EASY_AUDIO
    Audio::Stop(1);
#else
    Audio::Get().Stop(2);
#endif
    return;
  }

    // �^�[���b�g(�ƖC�g)���}�E�X�J�[�\���̕����Ɍ�����
  MeshRenderer& meshRenderer = static_cast<MeshRenderer&>(*renderer);
  MeshPtr mesh = meshRenderer.GetMesh();
  if (mesh) {
    // �}�E�X�J�[�\�����W�ƌ������镽�ʂ̍��W�Ɩ@��
    const float gunY = mesh->groups[gunGroup].matBindPose[3][1];
    const glm::vec3 gunPlanePos = position + glm::vec3(0, gunY, 0); // ���ʂ̍��W
    const glm::vec3 gunPlaneNormal = glm::vec3(0, 1, 0); // ���ʂ̖@��

    // �C�����}�E�X�J�[�\���̕����Ɍ�����
    Camera& camera = engine.GetCamera();
    const glm::mat4 matVP = camera.GetProjectionMatrix() * camera.GetViewMatrix();
    glm::vec3 start, end, p;
    ScreenPosToLine(engine.GetMousePosition(), matVP, start, end);
    if (Intersect(start, end, gunPlanePos, gunPlaneNormal, p)) {
      // �A�N�^�[����}�E�X�J�[�\���֌����������x�N�g�����v�Z
      const glm::vec3 d = p - position;

      // �A�[�N�^���W�F���g�֐��Ō����x�N�g�����p�x�ɕϊ�
      // 0�x�̂Ƃ��̖C���̌����͉������ŁA���w�I��0�x(�E����)����-90�x�̈ʒu�ɂ���
      // �v�Z�œ���ꂽ�p�x��90�x�𑫂��΁A��]�p�x�ƃ��f���̌�������v����͂�
      rotTurret = std::atan2(-d.z, d.x) + glm::radians(90.0f);

      // �A�N�^�[�̉�]��ł�����
      rotTurret -= rotation;
    }

    // �C����Y����]������s����쐬
    const glm::mat4 matRot =
      glm::rotate(glm::mat4(1), rotTurret, glm::vec3(0, 1, 0));

    // ��]�s���C���̃O���[�v�s��ɐݒ�
    meshRenderer.SetGroupMatrix(turretGroup, matRot);
  }

  bool playTankTruck = false;

  if (isOnActor) {
    if (engine.GetKey(GLFW_KEY_A)) {
      rotation += glm::radians(90.0f) * deltaTime;
      playTankTruck = true;
    } else if (engine.GetKey(GLFW_KEY_D)) {
      rotation -= glm::radians(90.0f) * deltaTime;
      playTankTruck = true;
    }
  }

  // rotation��0�̂Ƃ��̐�Ԃ̌����x�N�g��
  glm::vec3 tankFront(0, 0, 1);
  // rotation���W�A��������]�������]�s������
  const glm::mat4 matRot = glm::rotate(glm::mat4(1), rotation, glm::vec3(0, 1, 0));
  // �����x�N�g����tank.rotation������]������
  tankFront = matRot * glm::vec4(tankFront, 1);

  if (isOnActor) {
    float speed2 = glm::dot(velocity, velocity);
    //if (speed2 < 10.0f * 10.0f) {
    float tankAccel = 0.2f; // ��Ԃ̉����x
    if (engine.GetKey(GLFW_KEY_W)) {
      velocity += tankFront * tankAccel;
      playTankTruck = true;
    } else if (engine.GetKey(GLFW_KEY_S)) {
      velocity -= tankFront * tankAccel;
      playTankTruck = true;
    } else {
      float v = glm::dot(tankFront, velocity);
      velocity -= tankFront * glm::clamp(v, -0.1f, 0.1f);
    }
    //glm::vec3 tankRight = glm::normalize(glm::cross(tankFront, glm::vec3(0, 1, 0)));
    //float rightSpeed = glm::dot(tankRight, velocity);
    //velocity -= tankRight * glm::clamp(rightSpeed, -0.2f, 0.2f);
  //}
    if (engine.GetKey(GLFW_KEY_SPACE)) {
      velocity.y = 12;
    }
  }

#ifdef USE_EASY_AUDIO
  if (playTankTruck) {
    Audio::Play(1, SE_TANK_MOVE, 0.1f, true);
  } else {
    Audio::Stop(1);
  }
#else
  if (playTankTruck) {
    Audio::Get().Play(2, CRI_SE_TANK_MOVE, 0.1f);
  } else {
    Audio::Get().Stop(2);
  }
#endif

  // �}�E�X���{�^���̏�Ԃ��擾����
  int shotButton = engine.GetMouseButton(GLFW_MOUSE_BUTTON_LEFT);

  // �}�E�X���{�^���������ꂽ�u�Ԃɒe�A�N�^�[�𔭎˂���
  static int shotInterval = 5;
  bool isShot = false;
  if (shotButton != 0) {
    if (oldShotButton == 0 || --shotInterval <= 0) {
      isShot = true;
      shotInterval = 5;
    }
  }
  if (isShot) {
    // ���˕������v�Z
    const float rot = rotTurret - glm::radians(90.0f) + rotation;
    const glm::vec3 direction(std::cos(rot), 0, -std::sin(rot));

    // ���ˈʒu��C�̐�[�ɐݒ�
    glm::vec3 position = this->position + direction * 6.0f;
    position.y += 2.0f;

    std::shared_ptr<Actor> bullet = std::make_shared<Actor>(
      "Bullet", false,
      position, glm::vec3(0.25f), rotation, glm::vec3(0));

    // 1.5�b��ɒe������
    bullet->lifespan = 1.5f;

    // ��Ԃ̌����Ă�������ɁA30m/s�̑��x�ňړ�������
    bullet->velocity = direction * 30.0f;

    // �e�ɏՓ˔����t����
    //bullet->collider = Box::Create(glm::vec3(-0.25f), glm::vec3(0.25f));
    //bullet->collider = Sphere::Create(glm::vec3(0), 1.0f);
    bullet->collider = Cylinder::Create(glm::vec3(0, -0.3f, 0), 0.3f, 0.6f);
    bullet->mass = 6.8f;
    bullet->friction = 1.0f;

    bulletRenderer->AddInstance(bullet);
    engine.AddActor(bullet);

#ifdef USE_EASY_AUDIO
    Audio::PlayOneShot(SE_PLAYER_SHOT);
#else
    Audio::Get().Play(1, CRI_SE_SHOT);
#endif // USE_EASY_AUDIO
  }

  // �u�O��̃V���b�g�{�^���̏�ԁv���X�V����
  oldShotButton = shotButton;
}

/**
* �Փ˂���������
*
* @param contact �Փˏ��
*/
void PlayerActor::OnCollision(const struct Contact& contact)
{
  if (contact.b->name == "EnemyBullet") {
    // ���[�U�[������󂯕t���Ȃ��Ƃ��̓_���[�W���󂯂Ȃ�
    if (isControlable) {
      --health;
    }
    if (health <= 0) {
#ifdef USE_EASY_AUDIO
      Audio::PlayOneShot(SE_EXPLOSION);
#else
      Audio::Get().Play(1, CRI_SE_EXPLOSION);
#endif // USE_EASY_AUDIO
      isDead = true;
    }
    contact.b->isDead = true;
  }
}

