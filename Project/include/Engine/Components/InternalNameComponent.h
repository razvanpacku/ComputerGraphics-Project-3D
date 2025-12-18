#pragma once
#include "NameComponent.h"

struct InternalNameComponent : NameComponent 
{
	using NameComponent::NameComponent;
	// This component is used to mark entities that are internal to the engine and should not be exposed to the user.
	// It inherits from NameComponent to provide an internal name for identification, which cannot be changed by the user and is used in seatching for a entity by name if no NameComponent has this name
};