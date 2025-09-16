#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <gst/gst.h>
#include <QQuickItem>
#include <QQuickWindow>
#include <QRunnable>
#include <QDebug>
#include <array>

class SetPlaying : public QRunnable {
public:
    SetPlaying(const std::array<GstElement*, 4>& pipelines);
    ~SetPlaying();
    void run();

private:
    std::array<GstElement*, 4> pipelines_;
};

SetPlaying::SetPlaying(const std::array<GstElement*, 4>& pipelines) {
    for (size_t i = 0; i < pipelines_.size(); ++i) {
        pipelines_[i] = pipelines[i] ? static_cast<GstElement*>(gst_object_ref(pipelines[i])) : nullptr;
    }
}

SetPlaying::~SetPlaying() {
    for (auto* pipeline : pipelines_) {
        if (pipeline) {
            gst_object_unref(pipeline);
        }
    }
}

void SetPlaying::run() {
    for (size_t i = 0; i < pipelines_.size(); ++i) {
        if (pipelines_[i]) {
            g_print("Starting pipeline %zu...\n", i + 1);
            GstStateChangeReturn ret = gst_element_set_state(pipelines_[i], GST_STATE_PLAYING);
            if (ret == GST_STATE_CHANGE_FAILURE) {
                g_print("Failed to start pipeline %zu\n", i + 1);
            }
        }
    }
}

G_BEGIN_DECLS
GST_PLUGIN_STATIC_DECLARE(qml6);
G_END_DECLS

class VideoStreamManager {
public:
    static constexpr size_t CAMERA_COUNT = 4;
    
    VideoStreamManager() {
        // Initialize camera URLs
        cameraUrls_[0] = "http://127.0.0.1:5000/stream0";
        cameraUrls_[1] = "http://127.0.0.1:5000/stream1";
        cameraUrls_[2] = "http://127.0.0.1:5000/stream2";
        cameraUrls_[3] = "http://127.0.0.1:5000/stream3";
        
        pipelines_.fill(nullptr);
        sinks_.fill(nullptr);
        videoItems_.fill(nullptr);
    }
    
    ~VideoStreamManager() {
        cleanup();
    }
    
    bool createPipelines() {
        for (size_t i = 0; i < CAMERA_COUNT; ++i) {
            if (!createPipeline(i)) {
                g_print("Failed to create pipeline %zu\n", i + 1);
                return false;
            }
        }
        return true;
    }
    
    bool connectVideoItems(QQuickWindow* rootObject) {
        for (size_t i = 0; i < CAMERA_COUNT; ++i) {
            QString itemName = QString("videoItem%1").arg(i);
            videoItems_[i] = rootObject->findChild<QQuickItem*>(itemName);
            
            if (!videoItems_[i]) {
                g_print("Could not find video item: %s\n", itemName.toUtf8().constData());
                return false;
            }
            
            if (sinks_[i]) {
                g_object_set(sinks_[i], "widget", videoItems_[i], nullptr);
                g_print("Connected pipeline %zu to video item %s\n", i + 1, itemName.toUtf8().constData());
            }
        }
        return true;
    }
    
    void startPipelines(QQuickWindow* rootObject) {
        rootObject->scheduleRenderJob(new SetPlaying(pipelines_), 
                                      QQuickWindow::BeforeSynchronizingStage);
    }
    
    void cleanup() {
        for (size_t i = 0; i < CAMERA_COUNT; ++i) {
            if (pipelines_[i]) {
                gst_element_set_state(pipelines_[i], GST_STATE_NULL);
                gst_object_unref(pipelines_[i]);
                pipelines_[i] = nullptr;
            }
            sinks_[i] = nullptr; // These are owned by the pipeline
        }
    }

private:
    bool createPipeline(size_t index) {
        QString pipelineStr = QString(
            "souphttpsrc location=%1 is-live=true do-timestamp=true timeout=10 ! "
            "queue max-size-buffers=3 leaky=downstream ! "
            "multipartdemux ! "
            "jpegdec ! "
            "videoconvert ! "
            "glupload ! glcolorconvert ! "
            "qml6glsink name=sink%2"
        ).arg(cameraUrls_[index], QString::number(index));
        
        g_print("Creating pipeline %zu: %s\n", index + 1, pipelineStr.toUtf8().constData());
        
        GError* error = nullptr;
        pipelines_[index] = gst_parse_launch(pipelineStr.toUtf8().constData(), &error);
        
        if (error) {
            g_print("Pipeline %zu creation error: %s\n", index + 1, error->message);
            g_error_free(error);
            return false;
        }
        
        if (!pipelines_[index]) {
            g_print("Failed to create pipeline %zu\n", index + 1);
            return false;
        }
        
        QString sinkName = QString("sink%1").arg(index);
        sinks_[index] = gst_bin_get_by_name(GST_BIN(pipelines_[index]), sinkName.toUtf8().constData());
        
        if (!sinks_[index]) {
            g_print("Failed to get sink for pipeline %zu\n", index + 1);
            return false;
        }
        return true;
    }
  
    std::array<QString, CAMERA_COUNT> cameraUrls_;
    std::array<GstElement*, CAMERA_COUNT> pipelines_;
    std::array<GstElement*, CAMERA_COUNT> sinks_;
    std::array<QQuickItem*, CAMERA_COUNT> videoItems_;
};

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    gst_init(&argc, &argv);
    GST_PLUGIN_STATIC_REGISTER(qml6);

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
                     []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule("untitled10", "Main");

    if (engine.rootObjects().isEmpty()) {
        qCritical() << "Failed to load QML";
        gst_deinit();
        return -1;
    }

    QQuickWindow* rootObject = static_cast<QQuickWindow*>(engine.rootObjects().first());
    if (!rootObject) {
        qCritical() << "Root object is not a QQuickWindow";
        gst_deinit();
        return -1;
    }

    VideoStreamManager streamManager;
    
    if (!streamManager.createPipelines()) {
        qCritical() << "Failed to create pipelines";
        gst_deinit();
        return -1;
    }
    if (!streamManager.connectVideoItems(rootObject)) {
        qCritical() << "Failed to connect video items";
        gst_deinit();
        return -1;
    }
    
    streamManager.startPipelines(rootObject);
    g_print("All pipelines created and started successfully\n");
    
    return app.exec();
}