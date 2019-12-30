#pragma once
#include <optional>
struct QueueFamilyIndices
{
	std::optional<unsigned int> GraphicsFamily;
	std::optional<unsigned int> PresentationFamily;

	bool IsComplete()
	{
		return GraphicsFamily.has_value() &&
			   PresentationFamily.has_value();
	}
};