#pragma once

#include <glm/common.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include "glm/glm/gtc/quaternion.hpp"

#include "glm/glm/ext/matrix_float4x4.hpp"
#include "glm/glm/ext/matrix_transform.hpp"
#include "glm/glm/ext/vector_float3.hpp"

#include "glm/glm/fwd.hpp"
#include "glm/glm/geometric.hpp"
#include "glm/glm/trigonometric.hpp"


namespace Xenith::Math
{
  using Vec2 = glm::vec2;
  using Vec3 = glm::vec3;
  using Vec4 = glm::vec4;

  using Matrix4 = glm::mat4;

  using Quaternion = glm::quat;



  // Translate
  inline void TranslateReference(Matrix4 &matrix, const Vec3 &translate_vector)
  {
    matrix = glm::translate(matrix, translate_vector);
  }

  inline Matrix4 TranslateReturn(const Matrix4 &matrix, const Vec3 &translate_vector)
  {
    return glm::translate(matrix, translate_vector);
  }

  // Rotate
  inline void RotateReference(Matrix4 &matrix, const Vec3 &axis_vector, float angle)
  {
    matrix = glm::rotate(matrix, angle, axis_vector);
  }

  inline Matrix4 RotateReturn(const Matrix4 &matrix, const Vec3 &axis_vector, float angle)
  {
    return glm::rotate(matrix, angle, axis_vector);
  }


  inline Matrix4 ScaleVectorReturn(const Matrix4 &matrix, const Vec3 &scale_vector)
  {
    return glm::scale(matrix, scale_vector);
  }

  inline void ScaleVectorReference(Matrix4 &matrix, const Vec3 &scale_vector)
  {
    matrix = glm::scale(matrix, scale_vector);
  }

  // Scale Scalar
  inline void ScaleScalarReference(Matrix4 &matrix, float scalar)
  {
    matrix = glm::scale(matrix, Vec3(scalar, scalar, scalar));
  }

  inline Matrix4 ScaleScalarReturn(const Matrix4 &matrix, float scalar)
  {
    return glm::scale(matrix, Vec3(scalar, scalar, scalar));
  }


  enum class CoordinateSystem
  {
    RIGHT_HANDED,
    LEFT_HANDED
  };

  inline Matrix4 PerspectiveProjection(float fov_degrees, float screen_aspect,
                                       float z_near, float z_far,
                                       Xenith::Math::CoordinateSystem coordinate_system)
  {
    Matrix4 perspective_proj = glm::perspective(glm::radians(fov_degrees), screen_aspect,
                                                z_near, z_far);

    if(coordinate_system == CoordinateSystem::RIGHT_HANDED)
      perspective_proj[1][1] *= -1; // flip y axis

    return perspective_proj;
  }

  // Look at
  inline Matrix4 LookAt(Vec3 eye, Vec3 center, Vec3 up)
  {
    return glm::lookAt(eye, center, up);
  }


  // Normalize
  inline Vec3 NormalizeReturn(Vec3 vector)
  {
    return glm::normalize(vector);
  }

  // Cross product
  inline Vec3 CrossProductReturn(Vec3 vector_1, Vec3 vector_2)
  {
    return glm::cross(vector_1, vector_2);
  }


  inline Xenith::Math::Quaternion QuaternionIdentity()
  {
    return Xenith::Math::Quaternion{ 1.0f, 0.0f, 0.0f, 0.0f };
  };


  inline float Length(Xenith::Math::Vec3 vector)
  {
    return glm::length(vector);
  }

  inline float LengthSquared(Xenith::Math::Vec3 vector)
  {
    return Length(vector) * Length(vector);
  }


  inline void ClampValueReference(float &value, float min_value, float max_value)
  {
    value = glm::clamp(value, min_value, max_value);
  }
  inline float ClampValueReturn(float value, float min_value, float max_value)
  {
    return glm::clamp(value, min_value, max_value);
  }
} // namespace Molten::Math
