#ifndef PETRIDISHVIEWERPLUGIN_HPP
#define PETRIDISHVIEWERPLUGIN_HPP
#include <mecacellviewer/primitives/roll.hpp>
#include <mecacellviewer/primitives/simpledisk.hpp>

template <typename P> struct PetriDishRenderer {
	QOpenGLShaderProgram shader;
	Disk disk{150};
	Roll roll{150};
	P& petriDish;

	PetriDishRenderer(P& p) : petriDish(p){};

	void load() {
		shader.addShaderFromSourceCode(
		    QOpenGLShader::Vertex, MecacellViewer::shaderWithHeader(":/shaders/mvp.vert"));
		shader.addShaderFromSourceCode(
		    QOpenGLShader::Fragment,
		    MecacellViewer::shaderWithHeader(":/shaders/smooth.frag"));
		shader.link();
		roll.load(shader);
		disk.load(shader);
	}

	template <typename R> void draw(R* r) {
		const auto& view = r->getViewMatrix();
		const auto& projection = r->getProjectionMatrix();

		shader.bind();
		shader.setUniformValue(shader.uniformLocation("projection"), projection);
		shader.setUniformValue(shader.uniformLocation("view"), view);
		shader.setUniformValue(shader.uniformLocation("eyeVec"),
		                       r->getCamera().getViewVector());
		shader.setUniformValue(shader.uniformLocation("upVec"), r->getCamera().getUpVector());
		{
			QVector4D color(1.0f, 1.0f, 1.0f, 1.0f);
			shader.setUniformValue(shader.uniformLocation("color"), color);
			disk.vao.bind();
			QMatrix4x4 model;
			// model.rotate(rot.teta * 180.0 / M_PI, toQV3D(rot.n));
			model.scale(petriDish.radius, 0, petriDish.radius);
			QMatrix4x4 nmatrix = (model).inverted().transposed();
			shader.setUniformValue(shader.uniformLocation("model"), model);
			shader.setUniformValue(shader.uniformLocation("normalMatrix"), nmatrix);
			MecacellViewer::GL()->glDrawElements(GL_TRIANGLES, disk.indices.size(),
			                                     GL_UNSIGNED_INT, 0);

			disk.vao.release();
		}
		{
			QVector4D color(1.0f, 1.0f, 1.0f, 1.0f);
			shader.setUniformValue(shader.uniformLocation("color"), color);
			roll.vao.bind();
			QMatrix4x4 model;
			// model.rotate(rot.teta * 180.0 / M_PI, toQV3D(rot.n));
			model.translate(0, petriDish.height, 0);
			model.scale(petriDish.radius, petriDish.height, petriDish.radius);
			QMatrix4x4 nmatrix = (model).inverted().transposed();
			shader.setUniformValue(shader.uniformLocation("model"), model);
			shader.setUniformValue(shader.uniformLocation("normalMatrix"), nmatrix);
			MecacellViewer::GL()->glDrawElements(GL_TRIANGLES, roll.indices.size(),
			                                     GL_UNSIGNED_INT, 0);
			roll.vao.release();
		}
		shader.release();
	}
};

template <typename P> struct PetriDishViewerPlugin {
	PetriDishRenderer<P> pdr;
	PetriDishViewerPlugin(P& p) : pdr(p) {}

	template <typename R> void onLoad(R* renderer) {
		MenuElement<R>* nativeDisplayMenu = renderer->getDisplayMenu();
		MenuElement<R> pdView = {"Petri Dish", true};
		pdr.load();
		pdView.onToggled = [&](R* r, MenuElement<R>* me) {
			if (me->isChecked()) {
				r->addPaintStepsMethods(19, [&](R* r2) { pdr.draw(r2); });
			} else {
				r->erasePaintStepsMethods(19);
			}
		};
		nativeDisplayMenu->at("Cells").add(pdView);
	}
};
#endif
