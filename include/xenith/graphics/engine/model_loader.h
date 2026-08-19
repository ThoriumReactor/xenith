#pragma once

#include "xenith/graphics/graphics_vk.h"

#include "fastgltf/types.hpp"
#include "xenith/graphics/texture.h"


namespace Xenith::Graphics
{
  enum class ModelParsingVec2DataType
  {
    UV
  };

  enum class ModelParsingVec3DataType
  {
    POSITION,
    NORMAL
  };


  Xenith::Graphics::ModelData LoadModelDataGLB(const std::string &model_path);

  /// Load mesh with filetype .glb/gltf 2.0
  Xenith::Graphics::Mesh LoadMeshGLB(Xenith::Graphics::ModelData &model_data, uint32_t base_texture_offset);

  static void ParseModelVec3Data(fastgltf::Asset &model, std::vector<Xenith::Graphics::Vertex3D> &vertices,
                                fastgltf::Attribute* attribute, fastgltf::Primitive &primitive,
                                ModelParsingVec3DataType vec3_data_type, size_t base_offset);

  static void ParseModelVec2Data(fastgltf::Asset &model, std::vector<Xenith::Graphics::Vertex3D> &vertices,
                                 fastgltf::Attribute* attribute, fastgltf::Primitive &primitive,
                                 ModelParsingVec2DataType vec2_data_type, size_t base_offset);

  std::vector<Xenith::Graphics::ImageData> LoadTexturesImageDataGLB(Xenith::Graphics::ModelData &model_data);
} // namespace Xenith::Graphics
