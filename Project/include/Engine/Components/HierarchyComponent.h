#pragma once
#include <entt/entt.hpp>

struct HierarchyComponent {
	entt::entity parent{entt::null};
	entt::entity firstChild{entt::null};
	entt::entity nextSibling{entt::null};
	entt::entity prevSibling{entt::null};
};