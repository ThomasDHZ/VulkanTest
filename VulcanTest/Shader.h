#pragma once
#include <vulkan\vulkan_core.h>
#include <vector>
#include <string>

class ShaderCompiler
{
private:
	std::vector<char> ReadShaderFile(const std::string& ShaderName);
	VkShaderModule  CompileShader(const VkDevice Device, const std::vector<char>& ShaderCode);
public:
	ShaderCompiler();
	VkShaderModule CompileShader(const VkDevice Device, const std::string& ShaderName);
};

