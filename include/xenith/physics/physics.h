#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "Jolt/Jolt.h"
#include "Jolt/Core/Core.h"
#include "Jolt/Core/JobSystemThreadPool.h"

#include "Jolt/Math/Quat.h"
#include "Jolt/Math/Real.h"
#include "Jolt/Math/Vec3.h"

#include "Jolt/Physics/Body/AllowedDOFs.h"
#include "Jolt/Physics/Body/BodyInterface.h"
#include "Jolt/Physics/Body/MotionType.h"
#include "Jolt/Physics/Collision/CastResult.h"
#include "Jolt/Physics/Collision/RayCast.h"
#include "Jolt/Physics/EActivation.h"
#include "Jolt/Physics/PhysicsSettings.h"
#include "Jolt/Physics/PhysicsSystem.h"

#include "Jolt/Physics/Body/Body.h"
#include "Jolt/Physics/Body/BodyID.h"

#include "Jolt/Physics/Collision/ObjectLayer.h"
#include "Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/RegisterTypes.h"

#include "xenith/ecs_xn/ecs_xn.h"
#include "xenith/math/math.h"


namespace Xenith::Physics
{
  using PhysicsSystem  = JPH::PhysicsSystem;
  using BodyInterface  = JPH::BodyInterface;

  using ObjectLayer    = JPH::ObjectLayer;


  [[nodiscard]] inline Xenith::Math::Vec3 JoltVec3ToGLMVec3(const JPH::Vec3 &jolt_vector) noexcept
  {
    return Xenith::Math::Vec3{
      static_cast<float>(jolt_vector.GetX()),
      static_cast<float>(jolt_vector.GetY()),
      static_cast<float>(jolt_vector.GetZ())
    };
  }
  [[nodiscard]] inline Xenith::Math::Vec3 JoltRVec3ToGLMVec3(const JPH::RVec3 &jolt_vector) noexcept
  {
    return Xenith::Math::Vec3{
      static_cast<float>(jolt_vector.GetX()),
      static_cast<float>(jolt_vector.GetY()),
      static_cast<float>(jolt_vector.GetZ())
    };
  }

  // [[nodiscard]] inline Xenith::Math::Quaternion JoltQuatToGLMQuaternion(const JPH::Quat &jolt_quaternion) noexcept
  // {
  // }

  [[nodiscard]] inline JPH::Vec3 GLMVec3ToJoltVec3(const Xenith::Math::Vec3 &glm_vector) noexcept
  {
    return JPH::Vec3(glm_vector.x, glm_vector.y, glm_vector.z);
  }
  [[nodiscard]] inline JPH::RVec3 GLMVec3ToJoltRVec3(const Xenith::Math::Vec3 &glm_vector) noexcept
  {
    return JPH::RVec3(glm_vector.x, glm_vector.y, glm_vector.z);
  }

  [[nodiscard]] inline JPH::Quat GLMQuaternionToJoltQuat(const Xenith::Math::Quaternion &glm_quaterneion)
  {
    return JPH::Quat(glm_quaterneion.x, glm_quaterneion.y, glm_quaterneion.z, glm_quaterneion.w);
  }


  struct BoxShapeSettings
  {
    Xenith::Math::Vec3 extent;
    float convex_radius = JPH::cDefaultConvexRadius;
  };

  using ShapeReference = JPH::Ref<JPH::Shape>;

  using MotionType = JPH::EMotionType;
  namespace MotionTypes
  {
    constexpr MotionType STATIC     = JPH::EMotionType::Static;
    constexpr MotionType KINEMATIC  = JPH::EMotionType::Kinematic;
    constexpr MotionType DYNAMIC    = JPH::EMotionType::Dynamic;
  }

  using ActivationMode = JPH::EActivation;
  namespace ActivationModes
  {
    constexpr ActivationMode ACTIVATE       = JPH::EActivation::Activate;
    constexpr ActivationMode DONT_ACTIVATE  = JPH::EActivation::DontActivate;
  }

  using AllowedDegreesOfFreedom = JPH::EAllowedDOFs;
  namespace AllowedDegreesOfFreedoms
  {
    constexpr AllowedDegreesOfFreedom ALL   = JPH::EAllowedDOFs::All;
    constexpr AllowedDegreesOfFreedom NONE  = JPH::EAllowedDOFs::None;

    constexpr AllowedDegreesOfFreedom PLANE_2D  = JPH::EAllowedDOFs::Plane2D;

    constexpr AllowedDegreesOfFreedom TRANSLATION_X  = JPH::EAllowedDOFs::TranslationX;
    constexpr AllowedDegreesOfFreedom TRANSLATION_Y  = JPH::EAllowedDOFs::TranslationY;
    constexpr AllowedDegreesOfFreedom TRANSLATION_Z  = JPH::EAllowedDOFs::TranslationZ;

    constexpr AllowedDegreesOfFreedom ROTATION_X  = JPH::EAllowedDOFs::RotationX;
    constexpr AllowedDegreesOfFreedom ROTATION_Y  = JPH::EAllowedDOFs::RotationY;
    constexpr AllowedDegreesOfFreedom ROTATION_Z  = JPH::EAllowedDOFs::RotationZ;

    constexpr AllowedDegreesOfFreedom TRANSLATION_XYZ  = TRANSLATION_X |
                                                         TRANSLATION_Y |
                                                         TRANSLATION_Z;

    constexpr AllowedDegreesOfFreedom ROTATION_XYZ  = ROTATION_X |
                                                      ROTATION_Y |
                                                      ROTATION_Z;
  }
  

  using BodyID = JPH::BodyID;

  struct RigidbodySettings
  {
    Xenith::Physics::ShapeReference shape_reference = nullptr;
    Xenith::Math::Vec3 position;
    Xenith::Math::Quaternion rotation_quaternion = Xenith::Math::QuaternionIdentity();

    Xenith::Physics::MotionType motion_type;
    Xenith::Physics::ObjectLayer object_layer;

    float mass;

    /// Limit how body is going to move and rotate in world space. 
    /// e.g. TRANSLATION_X for body to only move on X axis, without rotating,
    /// e.g. ROTATION_Z, to only rotate on Z axis without moving(translating).
    /// Default value is ALL, since bodies usually move and rotate in all directions
    Xenith::Physics::AllowedDegreesOfFreedom allowed_degrees_of_freedom_flags = Xenith::Physics::AllowedDegreesOfFreedoms::ALL;

    float friction = 0.2f;

  };

  struct Rigidbody
  {
    Xenith::Physics::BodyID body_id;
  };


  constexpr uint16_t MAX_LAYERS = 64;
  constexpr uint8_t MAX_BROADPHASE_LAYERS = 8;
  
  struct LayerMatrix 
  {
    std::array<uint8_t, MAX_LAYERS> object_to_broadphase{};
    std::array<uint8_t, MAX_BROADPHASE_LAYERS> broadphase_masks{};
    std::array<uint64_t, MAX_LAYERS> collision_masks{};
  };

  namespace BroadPhase 
  {
    constexpr uint8_t STATIC   = 0;
    constexpr uint8_t DYNAMIC  = 1;
  }

  // Filter 1 - Object vs Broad Phase
  class JoltObjectVsBPFilter final : public JPH::ObjectVsBroadPhaseLayerFilter
  {
    private:
      const LayerMatrix &layer_matrix_private_member;

    public:
      explicit JoltObjectVsBPFilter(const LayerMatrix& layer_matrix) : layer_matrix_private_member(layer_matrix) {}

      bool ShouldCollide(JPH::ObjectLayer object_layer_1, JPH::BroadPhaseLayer broad_phase_layer2) const override
      {
        uint8_t broad_phase_layer1 = layer_matrix_private_member.object_to_broadphase[object_layer_1];
        uint8_t broad_phase_layer2_value = static_cast<uint8_t>(broad_phase_layer2.GetValue());

        return (layer_matrix_private_member.broadphase_masks[broad_phase_layer1] & (1 << broad_phase_layer2_value)) != 0;
      }
  };

  // Filter 2 - Object vs Object
  class JoltObjectLayerFilter final : public JPH::ObjectLayerPairFilter
  {
    private:
      const LayerMatrix &layer_matrix_private_member;

    public:
      explicit JoltObjectLayerFilter(const LayerMatrix &layer_matrix) : layer_matrix_private_member(layer_matrix) {}

      bool ShouldCollide(JPH::ObjectLayer object_layer_1, JPH::ObjectLayer object_layer_2) const override
      { 
        return (layer_matrix_private_member.collision_masks[object_layer_1] & (1ULL << object_layer_2)) != 0;
      }
  };

  class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
  {
    private:
      const LayerMatrix &layer_matrix_private_member;

    public:
      explicit BPLayerInterfaceImpl(const LayerMatrix &layer_matrix) : layer_matrix_private_member(layer_matrix) {}

      uint32_t GetNumBroadPhaseLayers() const override 
      {
        return MAX_BROADPHASE_LAYERS;
      }

      JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
      {
        uint8_t broad_phase_layer_index = layer_matrix_private_member.object_to_broadphase[inLayer];
        return JPH::BroadPhaseLayer(broad_phase_layer_index);
      }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
      const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
      {
        switch (static_cast<uint8_t>(inLayer.GetValue()))
          {
            case BroadPhase::STATIC:  return "STATIC";
            case BroadPhase::DYNAMIC: return "DYNAMIC";
            default:                  return "UNKNOWN";
          }
      }
#endif
  };


  struct PhysicsContext
  {
    Xenith::Physics::LayerMatrix layer_matrix;

    std::unique_ptr<JPH::TempAllocatorImpl> temp_allocator;
    std::unique_ptr<JPH::JobSystemThreadPool> job_system;

    Xenith::Physics::BPLayerInterfaceImpl broad_phase_interface;
    Xenith::Physics::JoltObjectVsBPFilter object_broad_phase_filter;
    Xenith::Physics::JoltObjectLayerFilter object_filter;

    Xenith::Physics::PhysicsSystem physics_system;

    explicit PhysicsContext(const Xenith::Physics::LayerMatrix& in_matrix) : layer_matrix(in_matrix),
                                                                              broad_phase_interface(layer_matrix),
                                                                              object_broad_phase_filter(layer_matrix),
                                                                              object_filter(layer_matrix)
                                                                              {}
  };

  struct PhysicsSystemInitializationConfig
  {
    int number_of_worker_threads;
    size_t size_of_allocated_memory_for_temporary_allocator;

    uint32_t max_bodies;
    uint32_t number_of_body_mutexes = 0;
    uint32_t max_body_pairs;
    uint32_t max_contant_constraints;
  };


  Xenith::Physics::PhysicsSystemInitializationConfig LoadEngineDefaultPhysicsSystemInitializationConfig();


  void InitializePhysicsSystem(Xenith::Physics::PhysicsContext &physics_context,
                               const Xenith::Physics::PhysicsSystemInitializationConfig &physics_system_initialization_config);


  inline Xenith::Physics::BodyInterface& GetBodyInterfaceFromPhysicsSystem(Xenith::Physics::PhysicsSystem &physics_system)
  {
    return physics_system.GetBodyInterface();
  }

  inline Xenith::Math::Vec3 GetRigidbodyPositionFromBodyInterface(Xenith::Physics::BodyInterface &body_interface,
                                                                  const Xenith::Physics::BodyID &body_id)
  {
    JPH::RVec3 jolt_rigidbody_position = body_interface.GetPosition(body_id);

    return Xenith::Physics::JoltRVec3ToGLMVec3(jolt_rigidbody_position);
  }
  inline void SetRigidbodyPositionInBodyInterface(Xenith::Physics::BodyInterface &body_interface,
                                                  const Xenith::Physics::BodyID &body_id,
                                                  const Xenith::Math::Vec3 &new_position_vector,
                                                  const Xenith::Physics::ActivationMode activation_mode)
  {
    JPH::RVec3 jolt_new_position_vector = Xenith::Physics::GLMVec3ToJoltRVec3(new_position_vector);

    body_interface.SetPosition(body_id, jolt_new_position_vector, activation_mode);
  }

  inline Xenith::Math::Vec3 GetRigidbodyLinearVelocityFromBodyInterface(Xenith::Physics::BodyInterface &body_interface,
                                                                        const Xenith::Physics::BodyID &body_id)
  {
    JPH::Vec3 jolt_rigidbody_velocity = body_interface.GetLinearVelocity(body_id);

    return Xenith::Physics::JoltRVec3ToGLMVec3(jolt_rigidbody_velocity);
  }
  inline void SetRigidbodyLinearVelocityInBodyInterface(Xenith::Physics::BodyInterface &body_interface,
                                                        const Xenith::Physics::BodyID &body_id,
                                                        const Xenith::Math::Vec3 &velocity_vector)
  {
    JPH::Vec3 jolt_velocity_vector = Xenith::Physics::GLMVec3ToJoltVec3(velocity_vector);

    body_interface.SetLinearVelocity(body_id, jolt_velocity_vector);
  }


  inline Xenith::Math::Vec3 GetGravityVectorFromPhysicsSystem(Xenith::Physics::PhysicsSystem &physics_system)
  {
    JPH::Vec3 jolt_gravity_vector = physics_system.GetGravity();

    return Xenith::Physics::JoltVec3ToGLMVec3(jolt_gravity_vector);
  }
  inline void SetGravityVectorInPhysicsSystem(Xenith::Physics::PhysicsSystem &physics_system,
                                              const Xenith::Math::Vec3 &new_gravity_vector)
  {
    JPH::Vec3 jolt_new_gravity_vector = Xenith::Physics::GLMVec3ToJoltVec3(new_gravity_vector);

    physics_system.SetGravity(jolt_new_gravity_vector);
  }


  void UpdatePhysicsSystemStep(Xenith::Physics::PhysicsContext &physics_context,
                               float delta_time,
                               int collision_steps = 1);

  
  Xenith::Physics::ShapeReference CreateBoxShapeReference(const BoxShapeSettings &box_shape_settings);


  Xenith::Physics::Rigidbody CreateRigidbody(Xenith::Physics::BodyInterface &body_interface,
                                             const RigidbodySettings &rigidbody_settings,
                                             const ecsxn::Entity entity_id,
                                             bool add_body_to_body_interface_automatically,
                                             const Xenith::Physics::ActivationMode activation_mode);

  struct RayCastHit
  {
    Xenith::Math::Vec3 position;
    Xenith::Math::Vec3 normal;

    float fraction = 0.0f;

    Xenith::Physics::BodyID body_id;
    bool has_hit = false;
  };

  inline bool CheckLayerMask(Xenith::Physics::ObjectLayer layer, uint32_t mask)
  {
    return (mask & (1 << layer)) != 0;
  }
  struct ObjectLayerBitmaskFilter : public JPH::ObjectLayerFilter
  {
    uint32_t mask = 0;
    explicit ObjectLayerBitmaskFilter(uint32_t mask) : mask(mask) {}
    bool ShouldCollide(JPH::ObjectLayer layer) const override { return (mask & (1 << layer)) != 0; }
  };

  inline bool CastRayNarrowPhaseQueryWithObjectLayerFilter(Xenith::Physics::PhysicsSystem &physics_system,
                                                           const Xenith::Math::Vec3 &origin,
                                                           const Xenith::Math::Vec3 &direction,
                                                           uint32_t layer_mask,
                                                           Xenith::Physics::RayCastHit* out_ray_cast_info = nullptr)
  {
    JPH::RRayCast ray_cast(GLMVec3ToJoltVec3(origin), GLMVec3ToJoltVec3(direction));
    JPH::RayCastResult ray_cast_result;

    bool hit = physics_system.GetNarrowPhaseQuery().CastRay(
      ray_cast,
      ray_cast_result,
      {},
      ObjectLayerBitmaskFilter(layer_mask)
    );

    if((out_ray_cast_info != nullptr) && hit)
    {
      out_ray_cast_info->has_hit = true;

      out_ray_cast_info->body_id = ray_cast_result.mBodyID;
      out_ray_cast_info->fraction = ray_cast_result.mFraction;

      out_ray_cast_info->position = origin + (direction * ray_cast_result.mFraction);
    }

    return hit;
  }
}
