#include "Engine/Resources/UboWriter.h"

UboWriter::UboWriter(const Ubo& uboInfo)
	: ubo(uboInfo)
{
	data.assign(ubo.dataSize, 0);
}

void UboWriter::Upload() const
{
	glBindBuffer(GL_UNIFORM_BUFFER, ubo.bufferID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, data.size(), data.data());
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}