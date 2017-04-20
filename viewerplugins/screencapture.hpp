#ifndef SCREENCAPTURE_MECACELLVIEWERPLUGIN_HPP
#define SCREENCAPTURE_MECACELLVIEWERPLUGIN_HPP
#include <QImage>
/**
 * @brief ScreenCapturePlugin creates a checkable option in the menu that enables screen
 * capture
 */

struct ScreenCapturePlugin {
	struct SCPaintStep {
		int cap = 0;

		void saveImg(int W, int H, const QString &path) {
			std::vector<GLubyte> pixels;
			pixels.resize(3 * W * H);
			GL->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			GL->glReadPixels(0, 0, W, H, GL_RGB, GL_UNSIGNED_BYTE, &pixels[0]);
			QImage img(&pixels[0], W, H, QImage::Format_RGB888);
			img.mirrored().save(path + QString("capture_") + QString::number(cap++) + ".jpg");
		}

		void call(R *r, const QString &path) {
			if (r->getCurrentFBO()) {
				auto s = r->getWindow()->renderTargetSize();
				saveImg(r->getWindow()->width() * 2.0, r->getWindow()->height() * 2.0, path);
			}
		}
	};

	QString path = "./";
	SCPaintStep scps;
	int skippedFrame = 0;
	template <typename R> void onLoad(R *renderer) {
		MenuElement<R> *nativeDisplayMenu = renderer->getDisplayMenu();
		MenuElement<R> capture = {"Enable screen capture", true};
		scps.load();
		capture.onToggled = [&](R *r, MenuElement<R> *me) {
			if (me->isChecked())
				r->addPaintStepsMethods(1000000000, [&](R *r2) {
					if (r->getFrame() % (1 + skippedFrame) == 0) {
						scps.call(r2, path);
					}
				});
			else
				r->erasePaintStepsMethods(1000000000);
		};
		nativeDisplayMenu->add(capture);
	}
};

#endif
