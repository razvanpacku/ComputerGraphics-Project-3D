#include "Engine/Renderer/ShadowFramebuffer.h"

#include "Engine/Renderer/GLStateCache.h"

// =========================================================
// ShadowFramebuffer
// =========================================================

ShadowFramebuffer::ShadowFramebuffer(ShadowMapType type)
	: isCube(type == ShadowMapType::Point)
{
	auto& _tm = ResourceManager::Get().textures;
	texHandle = isCube ? _tm.GetHandle("shadow/point") :
		_tm.GetHandle("shadow/dir");

	auto* tex = _tm.Get(texHandle);
	size = tex->GetHeight(); // assuming square textures

	// create fbo
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// attach depth texture
	if (isCube) {
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex->id, 0);
	}
	else {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex->id, 0);
	}

	// no color output
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "ShadowFramebuffer: incomplete FBO status: " << status << "\n";
	}

	glEnable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


ShadowFramebuffer::~ShadowFramebuffer()
{
	if (fbo != 0) {
		glDeleteFramebuffers(1, &fbo);
		fbo = 0;
	}
}

void ShadowFramebuffer::BindForWriting(int faceIndex) const
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	Clear();
	glEnable(GL_CULL_FACE);
	if( !isCube) glCullFace(GL_FRONT);

	if (isCube && faceIndex >= 0 && faceIndex < 6) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIndex,
			ResourceManager::Get().textures.Get(texHandle)->id, 0);
	}
	glViewport(0, 0, size, size);
}

void ShadowFramebuffer::Unbind(const GLStateCache& glState)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glCullFace(GL_BACK); // restore default

	// only disable culling if it was disabled before, to reduce state changes
	if(!glState.cullBackfaces)
		glDisable(GL_CULL_FACE);
}

void ShadowFramebuffer::Clear() const
{
	glClear(GL_DEPTH_BUFFER_BIT);
}
