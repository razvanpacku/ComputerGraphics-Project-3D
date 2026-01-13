#include "Engine/SceneGraph/Systems/CollisionSystem.h"

#include "Engine/SceneGraph/Systems/TransformSystem.h"
#include "Engine/SceneGraph/Systems/PhysicsSystem.h"

#include "Engine/SceneGraph/Entities/TransformEntity.h"

// ======================================================
// CollisionSystem
// ======================================================

CollisionSystem::CollisionSystem(Scene* scene, int16_t order, entt::registry* registry)
	: ISystem(scene, order), registry(registry)
{
	runOnStartup = true;
}

void CollisionSystem::OnUpdate(double deltaTime)
{
	BroadPhase();
	NarrowPhase();
	ResolveContacts(deltaTime);
}

void CollisionSystem::BroadPhase() {
	candidatePairs.clear();

	auto view = registry->view<ColliderComponent>();
	std::vector<entt::entity> entities;

	for (auto& ent : view) entities.push_back(ent);

	for (size_t i = 0; i < entities.size(); i++) {
		for (size_t j = i + 1; j < entities.size(); j++) {
			candidatePairs.push_back({ entities[i], entities[j] });
		}
	}
}

void CollisionSystem::NarrowPhase() {
	contacts.clear();

	for (auto& [a, b] : candidatePairs) {
		auto& ca = registry->get<ColliderComponent>(a);
		auto& cb = registry->get<ColliderComponent>(b);

		TransformEntity* entA = dynamic_cast<TransformEntity*>(scene->GetEntityFromHandle(a));
		TransformEntity* entB = dynamic_cast<TransformEntity*>(scene->GetEntityFromHandle(b));

		// SPHERE - SPHERE
		if (ca.shape == RigidBodyShape::Sphere && cb.shape == RigidBodyShape::Sphere) {
			glm::vec3 centerA = entA->GetGlobalPosition();
			glm::vec3 centerB = entB->GetGlobalPosition();

			float aScale = glm::length(entA->GetGlobalScale()) / sqrt(3.0f);
			float bScale = glm::length(entB->GetGlobalScale()) / sqrt(3.0f);

			glm::vec3 delta = centerA - centerB;
			float dist2 = glm::dot(delta, delta);
			float r = ca.radius * aScale + cb.radius * bScale;

			if (dist2 < r * r) {
				float dist = sqrt(dist2);
				glm::vec3 normal = (dist > 1e-6f) ? (delta / dist) : glm::vec3(0, 1, 0);
				float penetration = r - dist;

				Contact c;
				c.a = entA;
				c.b = entB;
				c.normal = normal;
				c.penetration = penetration;
				c.point = centerA + normal * (ca.radius - penetration * 0.5f);

				contacts.push_back(c);
			}
		}
		// SPHERE - CYLINDER
		if ((ca.shape == RigidBodyShape::Sphere && cb.shape == RigidBodyShape::Cylinder) ||
			(ca.shape == RigidBodyShape::Cylinder && cb.shape == RigidBodyShape::Sphere))
		{
			TransformEntity* sphereEnt = (ca.shape == RigidBodyShape::Sphere) ? entA : entB;
			TransformEntity* cylEnt = (ca.shape == RigidBodyShape::Cylinder) ? entA : entB;

			const ColliderComponent& sphereCol = (ca.shape == RigidBodyShape::Sphere) ? ca : cb;
			const ColliderComponent& cylCol = (ca.shape == RigidBodyShape::Cylinder) ? ca : cb;

			glm::vec3 S = sphereEnt->GetGlobalPosition();		// sphere center
			glm::vec3 C = cylEnt->GetGlobalPosition();			// cylinder center

			float sphereScale = glm::length(sphereEnt->GetGlobalScale()) / sqrt(3.0f);
			float cylScale = glm::length(cylEnt->GetGlobalScale()) / sqrt(3.0f);

			float r = sphereCol.radius * sphereScale;
			float R = cylCol.radius * cylScale;
			float h = cylCol.height * 0.5f * cylScale;			 // half height

			glm::quat cylinderRotation = cylEnt->GetGlobalRotation();

			// Cylinder axis in world space
			glm::vec3 A = glm::normalize(cylinderRotation * glm::vec3(0.f, 1.f, 0.f));

			// Vector from cylinder base to sphere center
			glm::vec3 d = S - C;
			float y = glm::dot(d, A);

			// Closest point ON cylinder axis
			glm::vec3 proj = C + A * y;

			// Vector from axis to sphere center
			glm::vec3 l = d - A * y;
			float radialDist2 = glm::dot(l, l);

			Contact c;
			c.a = sphereEnt;
			c.b = cylEnt;

			bool hit = false;

			// =================================
			// Case 1: Side wall
			// =================================
			if (-h <= y && y <= h) {
				float limit = R + r;
				if (radialDist2 < limit * limit) {
					float radialDist = sqrt(radialDist2);
					glm::vec3 normal = (radialDist > 1e-6f) ? l / radialDist : glm::vec3(0, 1, 0);

					// Point on cylinder surface
					glm::vec3 cylSurfacePoint = proj + normal * R;
					// Point on sphere surface
					glm::vec3 sphereSurfacePoint = S - normal * r;

					c.normal = normal;
					c.penetration = (R + r) - radialDist;
					c.point = (cylSurfacePoint + sphereSurfacePoint) * 0.5f;  // Midpoint
					hit = true;
				}
			}
			// =================================
			// Case 2: cap disks or edge rim
			// =================================
			else {
				// Determine which cap we are near
				float capSign = (y > 0) ? 1.0f : -1.0f;
				glm::vec3 capCenter = C + A * (h * capSign);

				glm::vec3 dCap = S - capCenter;
				float yCap = glm::dot(dCap, A);
				glm::vec3 radial = dCap - A * yCap;

				float radialLen2 = glm::dot(radial, radial);

				// 2A: cap disk
				if (radialLen2 <= R * R) {
					float dist = abs(yCap);
					if (dist < r) {
						glm::vec3 normal = A * capSign;

						// Point on cap disk
						glm::vec3 capPoint = S - normal * dist;
						// Point on sphere surface
						glm::vec3 sphereSurfacePoint = S - normal * r;

						c.normal = normal;
						c.penetration = r - dist;
						c.point = (capPoint + sphereSurfacePoint) * 0.5f;  // Midpoint
						hit = true;
					}
				}
				// 2B: edge rim
				else {
					glm::vec3 radialDir = (radialLen2 > 1e-6f) ? radial / sqrt(radialLen2) : glm::vec3(1, 0, 0);
					glm::vec3 edgePoint = capCenter + radialDir * R;

					glm::vec3 dEdge = S - edgePoint;
					float dist2 = glm::dot(dEdge, dEdge);

					float limit = r;
					if (dist2 < limit * limit) {
						float dist = sqrt(dist2);
						c.normal = dEdge / dist;
						c.penetration = r - dist;
						c.point = edgePoint + c.normal * (dist * 0.5f);
						hit = true;
					}
				}
			}

			if (hit) {
				contacts.push_back(c);
			}
		}

	}
}

void CollisionSystem::ResolveContacts(double deltaTime) {
	auto* trans = GetSystem<TransformSystem>();

	for (auto& c : contacts) {
		auto& aHandle = c.a->GetHandle();
		auto& bHandle = c.b->GetHandle();

		auto& ra = registry->get<RigidBodyComponent>(aHandle);
		auto& rb = registry->get<RigidBodyComponent>(bHandle);

		// Skip static-static
		if (ra.anchored && rb.anchored) continue;

		auto& ta = registry->get<TransformComponent>(aHandle);
		auto& tb = registry->get<TransformComponent>(bHandle);

		// Contact offsets from centers of mass
        glm::vec3 raToPoint = c.point - ta.position;
        glm::vec3 rbToPoint = c.point - tb.position;

        // Relative velocity including angular velocity contributions
        glm::vec3 velA = ra.velocity + glm::cross(ra.angularVelocity, raToPoint);
        glm::vec3 velB = rb.velocity + glm::cross(rb.angularVelocity, rbToPoint);
        glm::vec3 rv = velB - velA;

		float velAlongNormal = glm::dot(rv, c.normal);
		if (velAlongNormal <= 0) continue;

		float e = 0.1f; // restitution
		float invMassA = ra.anchored ? 0.0f : 1.0f / ra.mass;
		float invMassB = rb.anchored ? 0.0f : 1.0f / rb.mass;

		// Rotational term: cross(r, n) * I^-1 * cross(r, n)
		glm::vec3 raCrossN = glm::cross(raToPoint, -c.normal);
		glm::vec3 rbCrossN = glm::cross(rbToPoint, -c.normal);

		float denom =
			invMassA + invMassB +
			glm::dot(c.normal,
				glm::cross(raToPoint, ra.inverseInertiaTensorWorld * raCrossN) +
				glm::cross(rbToPoint, rb.inverseInertiaTensorWorld * rbCrossN));

		float j = -(1 + e) * velAlongNormal;
		j /= denom;

		glm::vec3 impulse = j * c.normal;

		// Apply linear impulse
		if (!ra.anchored) ra.velocity -= impulse * invMassA;
		if (!rb.anchored) rb.velocity += impulse * invMassB;

		// Apply angular impulse
		if (!ra.anchored)
			ra.angularVelocity -= ra.inverseInertiaTensorWorld * glm::cross(raToPoint, impulse);
		if (!rb.anchored)
			rb.angularVelocity += rb.inverseInertiaTensorWorld * glm::cross(rbToPoint, impulse);

		// ------------------------
		// TANGENTIAL FRICTION IMPULSE
		// ------------------------
		const float staticFriction = 0.2f;
		const float dynamicFriction = 0.1f;

		// Compute relative velocity again (we want post-normal-impulse value)
		velA = ra.velocity + glm::cross(ra.angularVelocity, raToPoint);
		velB = rb.velocity + glm::cross(rb.angularVelocity, rbToPoint);
		rv = velB - velA;

		// Remove normal component; get tangent motion
		glm::vec3 tangent = rv - glm::dot(rv, c.normal) * c.normal;
		float tangentMag = glm::length(tangent);

		if (tangentMag > 1e-6f) {
			tangent /= tangentMag; // normalize

			// Compute tangential impulse scalar
			glm::vec3 raCrossT = glm::cross(raToPoint, tangent);
			glm::vec3 rbCrossT = glm::cross(rbToPoint, tangent);

			float frictionDenom =
				invMassA + invMassB +
				glm::dot(tangent,
					glm::cross(raToPoint, ra.inverseInertiaTensorWorld * raCrossT) +
					glm::cross(rbToPoint, rb.inverseInertiaTensorWorld * rbCrossT));

			float jt = -glm::dot(rv, tangent) / frictionDenom;

			// Coulomb model clamp
			float maxFriction = j * staticFriction;
			if (abs(jt) > maxFriction)
				jt = glm::sign(jt) * j * dynamicFriction;

			glm::vec3 frictionImpulse = jt * tangent;

			// Apply friction linear impulse
			if (!ra.anchored) ra.velocity -= frictionImpulse * invMassA;
			if (!rb.anchored) rb.velocity += frictionImpulse * invMassB;

			// Apply friction angular impulse
			if (!ra.anchored)
				ra.angularVelocity -= ra.inverseInertiaTensorWorld * glm::cross(raToPoint, frictionImpulse);
			if (!rb.anchored)
				rb.angularVelocity += rb.inverseInertiaTensorWorld * glm::cross(rbToPoint, frictionImpulse);
		}

		// ------------------------
		// ROLLING RESISTANCE / ANGULAR DAMPING
		// ------------------------
		const float kRoll = 0.0025f; // small rolling resistance torque factor

		if (!ra.anchored) {
			ra.angularVelocity *= (1.0f - kRoll);
			ra.velocity *= (1.0f - kRoll);
		}
		if (!rb.anchored) {
			rb.angularVelocity *= (1.0f - kRoll);
			rb.velocity *= (1.0f - kRoll);
		}

		// Positional correction
		float percent = 0.8f;
		float slop = 0.001f;
		float correction = std::max(c.penetration - slop, 0.0f) / (invMassA + invMassB) * percent;
		glm::vec3 correctionVec = -correction * c.normal;

		if (!ra.anchored) {
			ta.position -= correctionVec * invMassA;
			ta.localDirty = true;
			trans->MarkDirty(aHandle);
		}
		if (!rb.anchored) {
			tb.position += correctionVec * invMassB;
			tb.localDirty = true;
			trans->MarkDirty(bHandle);
		}
	}
}

void CollisionSystem::GiveCollisionShape(Entity* entity, const RigidBodyInitData& rigidBodyData, float mass, bool anchored)
{
	auto& handle = entity->GetHandle();
	auto& collisionC = registry->emplace<ColliderComponent>(handle);
	collisionC.shape = rigidBodyData.shape;
	collisionC.radius = rigidBodyData.radius;
	collisionC.height = rigidBodyData.height;

	auto* phys = GetSystem<PhysicsSystem>();
	phys->GiveRigidBody(entity, rigidBodyData, mass, anchored);
}
