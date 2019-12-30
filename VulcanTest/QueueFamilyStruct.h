#pragma once
#include <optional>
struct QueueFamilyIndices
{
	std::optional<unsigned int> GraphicsFamily;

	bool IsComplete()
	{
		return GraphicsFamily.has_value();
	}
};