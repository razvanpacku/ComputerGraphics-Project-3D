#include "Engine/SceneGraph/Entities/ParticleEmitter.h"

#include "Engine/Renderer/RenderableProvider/MeshRenderableProvider.h"

glm::vec3 RandomDirectionInCone(
    const glm::vec3& dir,
    float angleDegrees)
{
    float angleRad = glm::radians(angleDegrees);

    float u = (float)rand() / RAND_MAX;
    float v = (float)rand() / RAND_MAX;

    float theta = u * glm::two_pi<float>();
    float phi = acos(1.0f - v * (1.0f - cos(angleRad)));

    glm::vec3 local(
        sin(phi) * cos(theta),
        cos(phi),
        sin(phi) * sin(theta)
    );

    glm::vec3 up = abs(dir.y) < 0.99f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
    glm::vec3 right = normalize(cross(up, dir));
    glm::vec3 forward = normalize(dir);
    up = cross(forward, right);

    return normalize(
        local.x * right +
        local.y * forward +
        local.z * up
    );
}

// ================================================================
// ParticleEmitter
// ================================================================

ParticleEmitter::ParticleEmitter(const std::string& name)
	: Entity(name), RenderEntity(name), TransformEntity(name)
{
	renderableProvider = new MeshRenderableProvider();
}

void ParticleEmitter::EmitParticle()
{
    if (!templateInitialized)
        return;

    ParticleInstanceData p;
    p.position = glm::vec3(0.0f);
    p.velocity = RandomDirectionInCone(direction, spreadAngle) * particleSpeed;
    p.lifetime = p.maxLifetime = particleLifetime;

    particles.push_back(p);

    renderableComponent->renderables.push_back(particleTemplate);
}

void ParticleEmitter::ProvideRenderables(std::vector<Renderable>& outRenderables)
{
    auto* provider = dynamic_cast<MeshRenderableProvider*>(renderableProvider);
	provider->meshHandle = ResourceManager::Get().meshes.GetHandle("primitive/quad_backface");
	provider->materialHandle = ResourceManager::Get().materials.GetHandle("particle");

    provider->GenerateRenderables(outRenderables);

	// Store template
    if (!templateInitialized)
    {
        particleTemplate = outRenderables.back();
        templateInitialized = true;
		particleTemplate.layer = RenderLayer::Transparent;
	}
    outRenderables.clear();
}

void ParticleEmitter::UpdateRenderables(double deltaTime, std::vector<Renderable>& renderables)
{
    if (isEmitting) {
        emissionAccumulator += emissionRate * (float)deltaTime;

        while (emissionAccumulator >= 1.0f)
        {
            EmitParticle();
            emissionAccumulator -= 1.0f;
        }
    }
    // Update particles
    for (int i = (int)particles.size() - 1; i >= 0; --i)
    {
        auto& p = particles[i];

        p.lifetime -= (float)deltaTime;
        if (p.lifetime <= 0.0f)
        {
            particles.erase(particles.begin() + i);
            renderables.erase(renderables.begin() + i);
            continue;
        }
        if(p.lifetime < p.maxLifetime * 0.75f && !p.detached)
        {
            // Detach particle from emitter after half lifetime
            p.detached = true;
            p.spawnPointMatrix = transformComponent->worldMatrix;
		}

        p.position += p.velocity * (float)deltaTime;

        float scale = maxScale * p.lifetime / p.maxLifetime;

        glm::mat4 local =
            glm::translate(glm::mat4(1.0f), p.position) *
            glm::scale(glm::mat4(1.0f), glm::vec3(scale));

        if(!p.detached)
            renderables[i].modelMatrix = transformComponent->worldMatrix * local;
        else {
			renderables[i].modelMatrix = p.spawnPointMatrix * local;
            p.velocity *= 0.98f;
        }

    } 
}

void ParticleEmitter::UpdateTransform(const glm::mat4& newTransform)
{
    
    for (size_t i = 0; i < particles.size(); i++)
    {
        if(particles[i].detached)
			continue;
        glm::mat4 local =
            glm::translate(glm::mat4(1.0f), particles[i].position);

		renderableComponent->renderables[i].modelMatrix = newTransform * local;
    }
}


