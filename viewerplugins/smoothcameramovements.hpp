#ifndef SMOOTHCAMERAMOVEMENTS_MECACELLVIEWERPLUGIN_HPP
#define SMOOTHCAMERAMOVEMENTS_MECACELLVIEWERPLUGIN_HPP

struct SmoothMoveAroundTargetPlugin {
	double raideurContrainte = 12.0;
	double desiredDistance = 3200.0;
	QVector3D rotationForce = {{100.0, 30.0, 0.0}};

	template <typename R> void onLoad(R* r) { r->getCamera().setMode(Camera::centered); }

	template <typename R> void preLoop(R* r) {
		double X = desiredDistance -
		           (r->getCamera().getPosition() - r->getCamera().getTarget()).length();
		r->getCamera().force += QVector3D(0.0, 0.0, raideurContrainte * X) + rotationForce;
	}
};

#endif
