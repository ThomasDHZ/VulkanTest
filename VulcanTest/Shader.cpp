#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

ShaderCompiler::ShaderCompiler()
{
}

std::vector<char> ShaderCompiler::ReadShaderFile(const std::string& ShaderName)
{
	std::ifstream file(ShaderName, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

VkShaderModule  ShaderCompiler::CompileShader(const VkDevice Device, const std::vector<char>& ShaderCode)
{
	VkShaderModule ShaderModule;

	VkShaderModuleCreateInfo CreateShaderInfo = {};
	CreateShaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	CreateShaderInfo.codeSize = ShaderCode.size();
	CreateShaderInfo.pCode = reinterpret_cast<const unsigned int*>(ShaderCode.data());

	VkResult Result = vkCreateShaderModule(Device, &CreateShaderInfo, nullptr, &ShaderModule);
	if (Result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return ShaderModule;
}

VkShaderModule ShaderCompiler::CompileShader(const VkDevice Device, const std::string& ShaderName)
{
	std::vector<char> ShaderCode = ReadShaderFile(ShaderName);
	return CompileShader(Device, ShaderCode);
}
