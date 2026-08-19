#pragma once

#include "entt/entt.hpp"

#include <utility>


namespace ecsxn
{
  using Entity    = entt::entity;
  using Registry  = entt::registry;

  inline ecsxn::Entity CreateEntityInRegistry(ecsxn::Registry &registry)
  {
    return registry.create();
  }

  template<typename ComponentType, typename... ComponentData>
  inline void EmplaceEntityComponentToRegistry(ecsxn::Registry &registry, 
                                               ecsxn::Entity entity, 
                                               ComponentData&&... component_data)
  {
    registry.emplace<ComponentType>(entity, std::forward<ComponentData>(component_data)...);
  }

  template<typename ComponentType>
  inline ComponentType& GetEntityComponentFromRegistry(ecsxn::Registry &registry,
                                                       ecsxn::Entity entity)
  {
    return registry.get<ComponentType>(entity);
  }


  template<typename ComponentType, typename... Arguments>
  inline ComponentType& EmplaceContextComponentToRegistryAndGetComponentData(ecsxn::Registry &registry, 
                                                                             Arguments&&... arguments)
  {
    return registry.ctx().emplace<ComponentType>(std::forward<Arguments>(arguments)...);
  }

  template<typename ComponentType>
  inline ComponentType& GetContextComponentFromRegistry(ecsxn::Registry &registry)
  {
    return registry.ctx().get<ComponentType>();
  }

  template <typename... ExcludeComponent>
  [[nodiscard]] inline auto ExcludeComponents()
  {
    return entt::exclude<ExcludeComponent...>;
  }

  template<typename... IncludeComponents>
  inline auto CreateFilteredEntityViewFromRegistryWithoutExclusion(ecsxn::Registry &registry)
  {
    return registry.view<IncludeComponents...>();
  }

  template<typename... IncludeComponents, typename... ExcludeComponents>
  inline auto CreateFilteredEntityViewFromRegistryWithExclusion(ecsxn::Registry &registry,
                                                                entt::exclude_t<ExcludeComponents...> exclude_components)
  {
    return registry.view<IncludeComponents...>(exclude_components);
  }
}
