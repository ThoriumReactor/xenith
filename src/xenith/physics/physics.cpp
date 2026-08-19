#include "xenith/physics/physics.h"

#include <algorithm>
#include <cstdint>
#include <thread>

#include "Jolt/Physics/Body/AllowedDOFs.h"
#include "Jolt/Physics/Body/BodyID.h"
#include "Jolt/Physics/Body/BodyCreationSettings.h"

#include "xenith/ecs_xn/ecs_xn.h"
#include "xenith/debug_xn/logging.h"


namespace Xenith::Physics
{
  Xenith::Physics::PhysicsSystemInitializationConfig LoadEngineDefaultPhysicsSystemInitializationConfig()
  {
    Xenith::Physics::PhysicsSystemInitializationConfig physics_system_initialization_config_to_return = {
      .number_of_worker_threads = -1,
      .size_of_allocated_memory_for_temporary_allocator = 5 * 1024 * 1024,

      .max_bodies = 1024,
      .number_of_body_mutexes = 0,
      .max_body_pairs = 1024,
      .max_contant_constraints = 1024
    };

    return physics_system_initialization_config_to_return;
  }


  void InitializePhysicsSystem(Xenith::Physics::PhysicsContext &physics_context,
                               const Xenith::Physics::PhysicsSystemInitializationConfig &physics_system_initialization_config)
  {
    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();

    uint32_t input_number_of_threads = 0;
    if(physics_system_initialization_config.number_of_worker_threads < 0)
      input_number_of_threads = std::max(1u, std::thread::hardware_concurrency() - 1);
    else
      input_number_of_threads = physics_system_initialization_config.number_of_worker_threads;

    physics_context.job_system = std::make_unique<JPH::JobSystemThreadPool>(
        JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers,
        input_number_of_threads
    );

    physics_context.temp_allocator = std::make_unique<JPH::TempAllocatorImpl>(
        physics_system_initialization_config.size_of_allocated_memory_for_temporary_allocator
    );

    physics_context.physics_system.Init(physics_system_initialization_config.max_bodies, 
                                        physics_system_initialization_config.number_of_body_mutexes,
                                        physics_system_initialization_config.max_body_pairs, 
                                        physics_system_initialization_config.max_contant_constraints,
                                        physics_context.broad_phase_interface,
                                        physics_context.object_broad_phase_filter,
                                        physics_context.object_filter);
  }


  void UpdatePhysicsSystemStep(Xenith::Physics::PhysicsContext &physics_context,
                               float delta_time,
                               int collision_steps)
  {
    physics_context.physics_system.Update(delta_time, collision_steps,
                                          physics_context.temp_allocator.get(),
                                          physics_context.job_system.get());
  }


  Xenith::Physics::ShapeReference CreateBoxShapeReference(const BoxShapeSettings &box_shape_settings)
  {
    JPH::BoxShapeSettings cube_shape_settings(
      JPH::Vec3(box_shape_settings.extent.x / 2.0f,
                              box_shape_settings.extent.y / 2.0f,
                              box_shape_settings.extent.z / 2.0f),
      box_shape_settings.convex_radius
    );
    JPH::ShapeSettings::ShapeResult cube_shape_result = cube_shape_settings.Create();
    if(cube_shape_result.HasError())
      Xenith::Debug::Log("Error in creating box shape reference! Error: %s", cube_shape_result.GetError().c_str());

    return cube_shape_result.Get();
  }


  Xenith::Physics::Rigidbody CreateRigidbody(Xenith::Physics::BodyInterface &body_interface,
                                             const RigidbodySettings &rigidbody_settings,
                                             const ecsxn::Entity entity_id,
                                             bool add_body_to_body_interface_automatically,
                                             const Xenith::Physics::ActivationMode activation_mode)
  {
    // --- Creation
    // Body creation settings
    JPH::BodyCreationSettings jolt_body_creation_settings(
      rigidbody_settings.shape_reference,
      GLMVec3ToJoltRVec3(rigidbody_settings.position),
      GLMQuaternionToJoltQuat(rigidbody_settings.rotation_quaternion),
      rigidbody_settings.motion_type,
      rigidbody_settings.object_layer
    );

    // Id
    jolt_body_creation_settings.mUserData = static_cast<uint64_t>(entity_id);

    // ---- Additional settings
    // Degrees of freedom
    jolt_body_creation_settings.mAllowedDOFs = rigidbody_settings.allowed_degrees_of_freedom_flags;

    jolt_body_creation_settings.mFriction = 0.0f;
    jolt_body_creation_settings.mMassPropertiesOverride = { .mMass = rigidbody_settings.mass };
    jolt_body_creation_settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
                                               

    JPH::Body* jolt_body = body_interface.CreateBody(jolt_body_creation_settings);
    if(!jolt_body)
    {
      Xenith::Debug::Log("Failed to create rigidbody");
      return Xenith::Physics::Rigidbody{ .body_id = JPH::BodyID() };
    }

    auto &body_id = jolt_body->GetID();

    if(add_body_to_body_interface_automatically)
    {
      body_interface.AddBody(body_id, activation_mode);
    }

    return Xenith::Physics::Rigidbody { .body_id = body_id };
  }
}
