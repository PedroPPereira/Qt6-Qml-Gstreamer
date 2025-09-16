#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <gst/gst.h>

#include <QQuickItem>
#include <QQuickWindow>
#include <QRunnable>

class SetPlaying : public QRunnable {
public:
  SetPlaying(GstElement *, GstElement *);
  ~SetPlaying();

  void run();

private:
  GstElement *pipeline1_;
  GstElement *pipeline2_;
};

SetPlaying::SetPlaying(GstElement *pipeline1, GstElement *pipeline2) {
  this->pipeline1_ =
      pipeline1 ? static_cast<GstElement *>(gst_object_ref(pipeline1)) : NULL;
  this->pipeline2_ =
      pipeline2 ? static_cast<GstElement *>(gst_object_ref(pipeline2)) : NULL;
}

SetPlaying::~SetPlaying() {
  if (this->pipeline1_)
    gst_object_unref(this->pipeline1_);
  if (this->pipeline2_)
    gst_object_unref(this->pipeline2_);
}

void SetPlaying::run() {
  if (this->pipeline1_) {
    g_print("Starting pipeline 1...\n");
    gst_element_set_state(this->pipeline1_, GST_STATE_PLAYING);
  }
  if (this->pipeline2_) {
    g_print("Starting pipeline 2...\n");
    gst_element_set_state(this->pipeline2_, GST_STATE_PLAYING);
  }
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

  QQuickItem *videoItemLeft;
  QQuickItem *videoItemRight;
  QQuickWindow *rootObject;

  /* find and set the videoItems on the sinks */
  rootObject = static_cast<QQuickWindow *>(engine.rootObjects().first());
  videoItemLeft = rootObject->findChild<QQuickItem *>("videoItemLeft");
  videoItemRight = rootObject->findChild<QQuickItem *>("videoItemRight");
  
  g_assert(videoItemLeft);
  g_assert(videoItemRight);
  
  g_print("Found video items: left=%p, right=%p\n", videoItemLeft, videoItemRight);

  // Create first pipeline for left camera
  GstElement *pipeline1 = gst_parse_launch(
      "souphttpsrc location=http://127.0.0.1:5000/stream ! multipartdemux ! jpegdec ! videoconvert !\
          glupload ! glcolorconvert ! qml6glsink name=sink1",
      NULL);
  GstElement *sink1 = gst_bin_get_by_name((GstBin *)pipeline1, "sink1");
  g_assert(sink1);
  g_object_set(sink1, "widget", videoItemLeft, NULL);
  g_print("Pipeline 1 created and connected to left video item\n");

  // Create second pipeline for right camera  
  GstElement *pipeline2 = gst_parse_launch(
      "souphttpsrc location=http://127.0.0.1:5000/stream1 ! multipartdemux ! jpegdec ! videoconvert !\
          glupload ! glcolorconvert ! qml6glsink name=sink2",
      NULL);
  GstElement *sink2 = gst_bin_get_by_name((GstBin *)pipeline2, "sink2");
  g_assert(sink2);
  g_object_set(sink2, "widget", videoItemRight, NULL);
  g_print("Pipeline 2 created and connected to right video item\n");

  // Start both pipelines
  rootObject->scheduleRenderJob(new SetPlaying(pipeline1, pipeline2),
                                QQuickWindow::BeforeSynchronizingStage);

  int result = app.exec();
  
  // Cleanup
  g_print("Cleaning up pipelines...\n");
  gst_element_set_state(pipeline1, GST_STATE_NULL);
  gst_element_set_state(pipeline2, GST_STATE_NULL);
  gst_object_unref(pipeline1);
  gst_object_unref(pipeline2);
  gst_deinit();
  
  return result;
}