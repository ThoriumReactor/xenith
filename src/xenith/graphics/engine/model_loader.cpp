#include "xenith/graphics/engine/model_loader.h"

#include <algorithm>
#include <cfloat>
#include <utility>
#include <vector>
#include <cassert>
#include <cstddef>
#include <cstdint>

#include "xenith/debug_xn/logging.h"
#include "xenith/graphics/graphics_vk.h"
#include "xenith/graphics/texture.h"
#include "xenith/math/math.h"

#include "fastgltf/core.hpp"
#include "fastgltf/math.hpp"
#include "fastgltf/types.hpp"
#include "fastgltf/tools.hpp"


namespace Xenith::Graphics
{
  Xenith::Graphics::ModelData LoadModelDataGLB(const std::string &model_path)
  {
    ModelData model_data_to_return;

    auto data_buffer = fastgltf::GltfDataBuffer::FromPath(model_path);
    if(!data_buffer)
      Xenith::Debug::Log("Failed to load from string: %s", model_path.c_str());


    fastgltf::Parser parser;
    auto asset = parser.loadGltfBinary(data_buffer.get(), model_path,
                                                        fastgltf::Options::None);
    if(!asset)
      Xenith::Debug::Log("Failed to parse GLB binary: %s", model_path.c_str());


    model_data_to_return = {
      .asset = std::move(asset.get()),
      .data_buffer = std::move(data_buffer.get())
    };

    return model_data_to_return;
  }


  Xenith::Graphics::Mesh LoadMeshGLB(Xenith::Graphics::ModelData &model_data, uint32_t base_texture_offset)
  {
    Xenith::Graphics::Mesh mesh_to_return = {};
    
    mesh_to_return.local_aabb.min = Xenith::Math::Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    mesh_to_return.local_aabb.max = Xenith::Math::Vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    Xenith::Graphics::AxisAlignedBoundingBox &mesh_local_aabb = mesh_to_return.local_aabb;

    fastgltf::Asset &model = model_data.asset;
    std::vector<Xenith::Graphics::Vertex3D> vertices;
    std::vector<uint32_t> indices;

    float primitive_offset_x = 0.0f;

    // Parse every primitive
    for(auto &mesh : model.meshes)
    {
      for(auto &primitive : mesh.primitives)
      {
        if(primitive.type != fastgltf::PrimitiveType::Triangles)
          continue;

        auto pos_attr    = primitive.findAttribute("POSITION");
        if(pos_attr == primitive.attributes.end()) continue;

        auto normal_attr = primitive.findAttribute("NORMAL");
        if(normal_attr == primitive.attributes.end()) continue;

        auto uv_attr     = primitive.findAttribute("TEXCOORD_0");

        size_t base_vertex_offset = vertices.size();
        const auto &pos_accessor = model.accessors[pos_attr->accessorIndex];

        vertices.resize(base_vertex_offset + pos_accessor.count);


        // Parse position
        ParseModelVec3Data(model, vertices, pos_attr, primitive,
                           ModelParsingVec3DataType::POSITION, base_vertex_offset);

        // Parse normal
        ParseModelVec3Data(model, vertices, normal_attr, primitive,
                           ModelParsingVec3DataType::NORMAL, base_vertex_offset);

        // Parse UV
        ParseModelVec2Data(model, vertices, uv_attr, primitive,
                           ModelParsingVec2DataType::UV, base_vertex_offset);


        // Materials
        uint32_t local_material_id = 0;
        if(primitive.materialIndex.has_value())
        {
          const auto &material = model.materials[primitive.materialIndex.value()];
          if(material.pbrData.baseColorTexture.has_value())
          {
            uint32_t texture_index = material.pbrData.baseColorTexture->textureIndex;
            if(model.textures[texture_index].imageIndex.has_value())
            {
              local_material_id = static_cast<uint32_t>(model.textures[texture_index].imageIndex.value());
            }
          }
        }

        uint32_t absolute_texture_id = local_material_id + base_texture_offset;
        for(size_t i = base_vertex_offset; i < vertices.size(); ++i)
        {
          vertices[i].local_texture_id = absolute_texture_id;
        }


        // Indices
        if(!primitive.indicesAccessor.has_value()) continue;

        const auto &indices_accessor = model.accessors[primitive.indicesAccessor.value()];

        fastgltf::iterateAccessor<uint32_t>(
          model,
          indices_accessor,
          [&](uint32_t index)
          {
            indices.push_back(index + static_cast<uint32_t>(base_vertex_offset));
        });

        // Local AABB
        fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(model, pos_accessor, 
            [&](fastgltf::math::fvec3 pos, size_t index) {
              mesh_local_aabb.min.x = std::min(mesh_local_aabb.min.x, pos.x());
              mesh_local_aabb.min.y = std::min(mesh_local_aabb.min.y, pos.y());
              mesh_local_aabb.min.z = std::min(mesh_local_aabb.min.z, pos.z());

              mesh_local_aabb.max.x = std::max(mesh_local_aabb.max.x, pos.x());
              mesh_local_aabb.max.y = std::max(mesh_local_aabb.max.y, pos.y());
              mesh_local_aabb.max.z = std::max(mesh_local_aabb.max.z, pos.z());
            });
      }
    }

    // give vertices default 
    for(auto &vertex : vertices)
    {
      // purple color
      vertex.color = { 255, 255, 255, 255 };
    }

    mesh_to_return.cpu_vertices = std::move(vertices);
    mesh_to_return.cpu_indices = std::move(indices);
    mesh_to_return.index_count = static_cast<uint32_t>(mesh_to_return.cpu_indices.size());

    return mesh_to_return;
  }


  static void ParseModelVec3Data(fastgltf::Asset &model, std::vector<Xenith::Graphics::Vertex3D> &vertices,
                                 fastgltf::Attribute* attribute, fastgltf::Primitive &primitive,
                                 ModelParsingVec3DataType vec3_data_type, size_t base_offset)
  {
    if(attribute == primitive.attributes.end())
      return;

    const fastgltf::Accessor &accessor = model.accessors[attribute->accessorIndex];

    size_t local_index = 0;

    fastgltf::iterateAccessor<fastgltf::math::f32vec3>(
      model,
      accessor,
      [&](fastgltf::math::f32vec3 vec3_data_value)
      {
        Xenith::Math::Vec3 vec3_data_glm = {vec3_data_value.x(), vec3_data_value.y(), vec3_data_value.z()};
        size_t target_vertex_index = base_offset + local_index;

        // Handle passing vec3 data for different vec3 types
        switch(vec3_data_type)
        {
          case ModelParsingVec3DataType::POSITION:
            vertices[target_vertex_index].position = vec3_data_glm;
            break;

          case ModelParsingVec3DataType::NORMAL:
            vertices[target_vertex_index].normals = vec3_data_glm;
            break;
        }

        local_index++;
    });
  }

  static void ParseModelVec2Data(fastgltf::Asset &model, std::vector<Xenith::Graphics::Vertex3D> &vertices,
                                 fastgltf::Attribute* attribute, fastgltf::Primitive &primitive,
                                 ModelParsingVec2DataType vec2_data_type, size_t base_offset)
  {
    if(attribute == primitive.attributes.end())
      return;

    const fastgltf::Accessor &accessor = model.accessors[attribute->accessorIndex];

    size_t local_index = 0;

    fastgltf::iterateAccessor<fastgltf::math::f32vec2>(
      model,
      accessor,
      [&](fastgltf::math::f32vec2 vec2_data_value)
      {
        Xenith::Math::Vec2 vec2_data_glm = {vec2_data_value.x(), vec2_data_value.y()};
        size_t target_vertex_index = base_offset + local_index;

        // Handle passing vec2 data for different vec2 types
        switch(vec2_data_type)
        {
          case ModelParsingVec2DataType::UV:
            vertices[target_vertex_index].uv = vec2_data_glm;
            break;
        }

        local_index++;
    });
  }


  std::vector<Xenith::Graphics::ImageData> LoadTexturesImageDataGLB(Xenith::Graphics::ModelData &model_data)
  {
    std::vector<ImageData> image_datas_container_to_return;
    const fastgltf::Asset &model = model_data.asset;

    image_datas_container_to_return.reserve(model.textures.size());

    for(const auto& texture : model.textures)
    {
      if(!texture.imageIndex.has_value()) continue;

      const auto &image = model.images[texture.imageIndex.value()];
      const uint8_t* raw_bytes_pointer = nullptr;
      size_t byte_length = 0;

      if(auto* view_source = std::get_if<fastgltf::sources::BufferView>(&image.data))
      {
        const auto &view = model.bufferViews[view_source->bufferViewIndex];
        const auto &buffer = model.buffers[view.bufferIndex];

        std::visit(fastgltf::visitor {
          [](const auto&) {},
          [&](const fastgltf::sources::Vector& vector) {
            raw_bytes_pointer = reinterpret_cast<const uint8_t*>(vector.bytes.data()) + view.byteOffset;
            byte_length = view.byteLength;
          },

          [&](const fastgltf::sources::Array& array) {
            raw_bytes_pointer = reinterpret_cast<const uint8_t*>(array.bytes.data()) + view.byteOffset;
            byte_length = view.byteLength;
          }
        }, buffer.data);
      }

      if (raw_bytes_pointer == nullptr || byte_length == 0) continue;

      int width = 0, height = 0, channels = 0;
      stbi_uc* decoded_pixels = stbi_load_from_memory(
          raw_bytes_pointer, 
          static_cast<int>(byte_length), 
          &width, &height, &channels, 
          STBI_rgb_alpha
      );

      if(!decoded_pixels)
      {
        Xenith::Debug::Log("Failed to decode GLB texture: %s", image.name.c_str());
        continue;
      }


      Xenith::Graphics::ImageData image_data_to_return = {
        .pixels = decoded_pixels,
        .width = width,
        .height = height,
        .channels = channels
      };
      
      image_datas_container_to_return.push_back(image_data_to_return);
    } // foreach texture : model.textures


    return image_datas_container_to_return;
  }
} // namespace Xenith::Graphics
