#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <gst/gst.h>

#include <QQuickItem>
#include <QQuickWindow>
#include <QRunnable>

class SetPlaying : public QRunnable {
public:
  SetPlaying(GstElement *);
  ~SetPlaying();

  void run();

private:
  GstElement *pipeline_;
};

SetPlaying::SetPlaying(GstElement *pipeline) {
  this->pipeline_ =
      pipeline ? static_cast<GstElement *>(gst_object_ref(pipeline)) : NULL;
}

SetPlaying::~SetPlaying() {
  if (this->pipeline_)
    gst_object_unref(this->pipeline_);
}

void SetPlaying::run() {
  if (this->pipeline_)
    gst_element_set_state(this->pipeline_, GST_STATE_PLAYING);
}

G_BEGIN_DECLS
GST_PLUGIN_STATIC_DECLARE(qml6);

G_END_DECLS
int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);
  QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
  gst_init(&argc, &argv);
  GST_PLUGIN_STATIC_REGISTER(qml6);

  /* the plugin must be loaded before loading the qml file to register the
   * GstGLVideoItem qml item */

  QQmlApplicationEngine engine;
  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
  engine.loadFromModule("untitled10", "Main");
  QQuickItem *videoItem;
  QQuickWindow *rootObject;

  /* find and set the videoItem on the sink */
  rootObject = static_cast<QQuickWindow *>(engine.rootObjects().first());
  videoItem = rootObject->findChild<QQuickItem *>("videoItem");
  g_assert(videoItem);
  GstElement *sink = gst_element_factory_make("qml6glsink", NULL);

  GstElement *_testpipeline = gst_parse_launch(
      "souphttpsrc location=http://127.0.0.1:5000/stream ! multipartdemux ! jpegdec ! videoconvert !\
          glupload ! glcolorconvert ! qml6glsink name=sink",
      NULL);
  GstElement *testsink = gst_bin_get_by_name((GstBin *)_testpipeline, "sink");

  g_assert(testsink);
  g_object_set(testsink, "widget", videoItem, NULL);

  rootObject->scheduleRenderJob(new SetPlaying(_testpipeline),
                                QQuickWindow::BeforeSynchronizingStage);
  return app.exec();
  gst_element_set_state(_testpipeline, GST_STATE_NULL);
  gst_object_unref(_testpipeline);
  gst_deinit();
}
