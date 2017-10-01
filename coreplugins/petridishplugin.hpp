#ifndef PETRIDISHPLUGIN_HPP
#define PETRIDISHPLUGIN_HPP

#include <functional>
#include <unordered_map>
#include <utility>
#include <vector>

template <typename cell_t> struct PetriDishPlugin {
	using V = decltype(((cell_t*)nullptr)->getPosition());  // Vec3D type

	struct Spring {  // simple dampered spring class
		V origin;
		double k, c, l, x;
		Spring(double K, double C, double L) : k(K), c(C), l(L), x(0) {}
		double computeForce(double currentLength, double speed) {
			x = l - currentLength;
			return k * x - c * speed;
		}
	};

	double radius{500};
	double height{100};

	V centerPosition{0, 0, 0};
	V upVector{0, 1, 0};
	std::function<std::pair<double, double>(const cell_t&)> getKC = [](const cell_t&) {
		return std::make_pair(1.0, 1.0);
	};  // used to compute the k and c parameters for each cell

	std::unordered_map<cell_t*, Spring> groundCollisions;
	std::unordered_map<cell_t*, Spring> wallCollisions;

	template <typename W> void beginUpdate(W* w) {
		for (const auto& c : w->cells) {
			// detect if cell is colliding with the ground
			const auto cellPos = c->getPosition();
			const auto cellRad = c->getBoundingBoxRadius();
			auto projPos = V::getProjectionOnPlane(centerPosition, upVector, cellPos);
			if ((projPos - cellPos).sqlength() < cellRad) {
				// we are colliding with the ground
				auto gndSql = (projPos - centerPosition).sqlength();
				if (gndSql < radius * radius) {
					// we are within the cylinder
					if (!groundCollisions.count(c)) {
						// it's a new collision
						std::pair<double, double> kc = getKC(*c);
						groundCollisions.insert({c, Spring(kc.first, kc.second, cellRad)});
					}
				} else if (gndSql * 0.95 > radius * radius &&
				           groundCollisions.count(c)) {  // little hysteresis
					groundCollisions.erase(c);
				}
			}
		}
	}

	template <typename W> void allForcesAppliedToCells(W* w) {
		const double minDist = 0.01;
		for (auto& gc : groundCollisions) {
			auto c = gc.first;
			auto cellPos = c->getPosition();
			const auto cellRad = c->getBoundingBoxRadius();
			auto projPos = V::getProjectionOnPlane(centerPosition, upVector, cellPos);
			gc.second.l = cellRad;
			double currentL = (projPos - cellPos).length();
			auto dir = (cellPos - projPos);
			if (currentL <= minDist || dir.dot(upVector) <= 0) {
				// cell is below
				c->getBody().setPosition(projPos + upVector * minDist);
				c->getBody().setVelocity(V::zero());
				cellPos = c->getPosition();
				dir = (cellPos - projPos);
				currentL = minDist;
			}
			dir /= currentL;
			auto vel = c->getBody().getVelocity().dot(dir);
			double f = gc.second.computeForce(currentL, vel);
			c->getBody().receiveForce(f * dir);
		}
	}
};

#endif
