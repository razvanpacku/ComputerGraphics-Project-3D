#include "Engine/Resources/UboWriter.h"

UboWriter::UboWriter(const UniformBlockInfo* uboInfo)
	: ubo(uboInfo)
{
	data.assign(ubo->dataSize, 0);
}

void UboWriter::Upload() const
{
	glBindBuffer(GL_UNIFORM_BUFFER, ubo->bufferID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, data.size(), data.data());
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UboWriter::PrintDebugInfo() const
{
	// print the content of the data buffer in hex
	std::cout << "UboWriter Debug Info for UBO: " << ubo->name << "\n";
	for (size_t i = 0; i < data.size(); i++) {
		if (i % 16 == 0) {
			std::cout << "\n0x" << std::hex << i << ": ";
		}
		std::cout << std::hex << static_cast<int>(data[i]) << " ";
	}
	std::cout << std::dec << "\nEnd of UboWriter Debug Info\n";
}