#include "ProxyRenderer.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>

#include <algorithm>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/display.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

namespace drift {

namespace {

// GOP size of 1 makes every frame a keyframe (All-Intra).
// This eliminates inter-frame dependencies, allowing the timeline playhead to seek
// to any frame in < 1 millisecond with zero CPU decoding overhead.
constexpr int kProxyGopSize = 1;

class ProxyTranscoder
{
public:
    ~ProxyTranscoder() { cleanup(); }

    bool run(const QString &sourcePath, const QString &outPath, int targetWidth, int targetHeight,
             QString *errorOut, const std::function<bool(double)> &onProgress);

private:
    void cleanup();

    AVFormatContext *m_inFmt = nullptr;
    AVFormatContext *m_outFmt = nullptr;
    AVCodecContext *m_decCtx = nullptr;
    AVCodecContext *m_encCtx = nullptr;
    SwsContext *m_sws = nullptr;

    int m_inVideoIndex = -1;
    int m_inAudioIndex = -1;
    int m_outVideoIndex = -1;
    int m_outAudioIndex = -1;

    QString m_tmpPath;
    QString m_finalPath;
};

void ProxyTranscoder::cleanup()
{
    if (m_sws) {
        sws_freeContext(m_sws);
        m_sws = nullptr;
    }
    if (m_encCtx) {
        avcodec_free_context(&m_encCtx);
    }
    if (m_decCtx) {
        avcodec_free_context(&m_decCtx);
    }
    if (m_inFmt) {
        avformat_close_input(&m_inFmt);
    }
    if (m_outFmt) {
        if (!(m_outFmt->oformat->flags & AVFMT_NOFILE) && m_outFmt->pb) {
            avio_closep(&m_outFmt->pb);
        }
        avformat_free_context(m_outFmt);
        m_outFmt = nullptr;
    }
    if (!m_tmpPath.isEmpty() && QFile::exists(m_tmpPath)) {
        QFile::remove(m_tmpPath);
    }
}

bool ProxyTranscoder::run(const QString &sourcePath, const QString &outPath, int targetWidth,
                         int targetHeight, QString *errorOut,
                         const std::function<bool(double)> &onProgress)
{
    auto fail = [&](const QString &msg) {
        if (errorOut)
            *errorOut = msg;
        cleanup();
        return false;
    };

    m_finalPath = outPath;
    m_tmpPath = outPath + QStringLiteral(".part");
    if (QFile::exists(m_tmpPath))
        QFile::remove(m_tmpPath);

    // Open source file
    const QByteArray srcUtf8 = sourcePath.toUtf8();
    if (avformat_open_input(&m_inFmt, srcUtf8.constData(), nullptr, nullptr) < 0) {
        return fail(QCoreApplication::translate("ProxyRenderer", "Failed to open source media"));
    }
    if (avformat_find_stream_info(m_inFmt, nullptr) < 0) {
        return fail(QCoreApplication::translate("ProxyRenderer", "Failed to read stream metadata"));
    }

    // Locate video and audio streams
    for (unsigned int i = 0; i < m_inFmt->nb_streams; ++i) {
        const AVCodecParameters *par = m_inFmt->streams[i]->codecpar;
        if (par->codec_type == AVMEDIA_TYPE_VIDEO && m_inVideoIndex < 0) {
            m_inVideoIndex = static_cast<int>(i);
        } else if (par->codec_type == AVMEDIA_TYPE_AUDIO && m_inAudioIndex < 0) {
            m_inAudioIndex = static_cast<int>(i);
        }
    }

    if (m_inVideoIndex < 0) {
        return fail(QCoreApplication::translate("ProxyRenderer", "No video stream found in source"));
    }

    AVStream *inVideoStream = m_inFmt->streams[m_inVideoIndex];
    const AVCodec *decoder = avcodec_find_decoder(inVideoStream->codecpar->codec_id);
    if (!decoder) {
        return fail(QCoreApplication::translate("ProxyRenderer", "Compatible video decoder not found"));
    }

    m_decCtx = avcodec_alloc_context3(decoder);
    if (!m_decCtx || avcodec_parameters_to_context(m_decCtx, inVideoStream->codecpar) < 0) {
        return fail(QCoreApplication::translate("ProxyRenderer", "Failed to initialize video decoder"));
    }
    if (avcodec_open2(m_decCtx, decoder, nullptr) < 0) {
        return fail(QCoreApplication::translate("ProxyRenderer", "Could not open video decoder"));
    }

    // Allocate output MP4 container
    const QByteArray tmpUtf8 = m_tmpPath.toUtf8();
    avformat_alloc_output_context2(&m_outFmt, nullptr, "mp4", tmpUtf8.constData());
    if (!m_outFmt) {
        return fail(QCoreApplication::translate("ProxyRenderer", "Could not allocate output container"));
    }

    // Setup H.264 video encoder
    const AVCodec *encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!encoder) {
        return fail(QCoreApplication::translate("ProxyRenderer", "H.264 encoder not available"));
    }

    AVStream *outVideoStream = avformat_new_stream(m_outFmt, nullptr);
    if (!outVideoStream) {
        return fail(QCoreApplication::translate("ProxyRenderer", "Could not create output video stream"));
    }
    m_outVideoIndex = outVideoStream->index;

    m_encCtx = avcodec_alloc_context3(encoder);
    if (!m_encCtx) {
        return fail(QCoreApplication::translate("ProxyRenderer", "Could not allocate video encoder"));
    }

    m_encCtx->width = targetWidth & ~1;
    m_encCtx->height = targetHeight & ~1;
    m_encCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    m_encCtx->time_base = inVideoStream->time_base;
    m_encCtx->framerate = av_guess_frame_rate(m_inFmt, inVideoStream, nullptr);
    m_encCtx->gop_size = kProxyGopSize;
    m_encCtx->max_b_frames = 0; // Zero B-frames for zero latency
    m_encCtx->color_range = m_decCtx->color_range;
    m_encCtx->colorspace = m_decCtx->colorspace;
    m_encCtx->color_primaries = m_decCtx->color_primaries;
    m_encCtx->color_trc = m_decCtx->color_trc;

    if (m_outFmt->oformat->flags & AVFMT_GLOBALHEADER)
        m_encCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    // Tuning for instantaneous seek & ultra-fast encoding
    av_opt_set(m_encCtx->priv_data, "preset", "ultrafast", 0);
    av_opt_set(m_encCtx->priv_data, "tune", "fastdecode", 0);
    av_opt_set(m_encCtx->priv_data, "crf", "23", 0);

    if (avcodec_open2(m_encCtx, encoder, nullptr) < 0) {
        return fail(QCoreApplication::translate("ProxyRenderer", "Could not open proxy encoder"));
    }

    avcodec_parameters_from_context(outVideoStream->codecpar, m_encCtx);
    outVideoStream->time_base = m_encCtx->time_base;
    outVideoStream->avg_frame_rate = m_encCtx->framerate;
    outVideoStream->r_frame_rate = m_encCtx->framerate;

    // Propagate rotation / display matrix
    for (int s = 0; s < inVideoStream->codecpar->nb_coded_side_data; ++s) {
        const AVPacketSideData *sd = &inVideoStream->codecpar->coded_side_data[s];
        if (sd->type == AV_PKT_DATA_DISPLAYMATRIX) {
            AVPacketSideData *outSd = av_packet_side_data_new(&outVideoStream->codecpar->coded_side_data,
                                                              &outVideoStream->codecpar->nb_coded_side_data,
                                                              AV_PKT_DATA_DISPLAYMATRIX, sd->size, 0);
            if (outSd)
                memcpy(outSd->data, sd->data, sd->size);
            break;
        }
    }

    // Remux audio stream without transcoding if present
    if (m_inAudioIndex >= 0) {
        AVStream *inAudioStream = m_inFmt->streams[m_inAudioIndex];
        AVStream *outAudioStream = avformat_new_stream(m_outFmt, nullptr);
        if (outAudioStream) {
            m_outAudioIndex = outAudioStream->index;
            avcodec_parameters_copy(outAudioStream->codecpar, inAudioStream->codecpar);
            outAudioStream->codecpar->codec_tag = 0;
            outAudioStream->time_base = inAudioStream->time_base;
        }
    }

    // Open output file
    if (!(m_outFmt->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&m_outFmt->pb, tmpUtf8.constData(), AVIO_FLAG_WRITE) < 0) {
            return fail(QCoreApplication::translate("ProxyRenderer", "Could not open destination file"));
        }
    }

    AVDictionary *opts = nullptr;
    av_dict_set(&opts, "movflags", "faststart", 0);
    if (avformat_write_header(m_outFmt, &opts) < 0) {
        av_dict_free(&opts);
        return fail(QCoreApplication::translate("ProxyRenderer", "Could not write proxy header"));
    }
    av_dict_free(&opts);

    // Prepare scaling context
    m_sws = sws_getContext(m_decCtx->width, m_decCtx->height, m_decCtx->pix_fmt,
                           m_encCtx->width, m_encCtx->height, AV_PIX_FMT_YUV420P,
                           SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
    if (!m_sws) {
        return fail(QCoreApplication::translate("ProxyRenderer", "Could not initialize video scaler"));
    }

    AVFrame *rawFrame = av_frame_alloc();
    AVFrame *scaledFrame = av_frame_alloc();
    scaledFrame->format = AV_PIX_FMT_YUV420P;
    scaledFrame->width = m_encCtx->width;
    scaledFrame->height = m_encCtx->height;
    av_frame_get_buffer(scaledFrame, 0);

    AVPacket *pkt = av_packet_alloc();
    AVPacket *outPkt = av_packet_alloc();

    const int64_t totalDuration = inVideoStream->duration > 0 ? inVideoStream->duration : m_inFmt->duration;

    // Transcode loop
    while (av_read_frame(m_inFmt, pkt) >= 0) {
        if (pkt->stream_index == m_inVideoIndex) {
            if (avcodec_send_packet(m_decCtx, pkt) >= 0) {
                while (avcodec_receive_frame(m_decCtx, rawFrame) >= 0) {
                    av_frame_make_writable(scaledFrame);
                    sws_scale(m_sws, rawFrame->data, rawFrame->linesize, 0, m_decCtx->height,
                              scaledFrame->data, scaledFrame->linesize);

                    scaledFrame->pts = rawFrame->best_effort_timestamp != AV_NOPTS_VALUE
                                           ? rawFrame->best_effort_timestamp
                                           : rawFrame->pts;

                    if (avcodec_send_frame(m_encCtx, scaledFrame) >= 0) {
                        while (avcodec_receive_packet(m_encCtx, outPkt) >= 0) {
                            outPkt->stream_index = m_outVideoIndex;
                            av_packet_rescale_ts(outPkt, m_encCtx->time_base, outVideoStream->time_base);
                            av_interleaved_write_frame(m_outFmt, outPkt);
                            av_packet_unref(outPkt);
                        }
                    }

                    if (onProgress && totalDuration > 0) {
                        const double prog = qBound(0.0, static_cast<double>(scaledFrame->pts) / totalDuration, 1.0);
                        if (!onProgress(prog)) {
                            av_packet_unref(pkt);
                            av_frame_free(&rawFrame);
                            av_frame_free(&scaledFrame);
                            av_packet_free(&pkt);
                            av_packet_free(&outPkt);
                            return fail(QCoreApplication::translate("ProxyRenderer", "Proxy generation cancelled"));
                        }
                    }
                }
            }
        } else if (pkt->stream_index == m_inAudioIndex && m_outAudioIndex >= 0) {
            // Passthrough audio
            AVStream *inAud = m_inFmt->streams[m_inAudioIndex];
            AVStream *outAud = m_outFmt->streams[m_outAudioIndex];
            av_packet_rescale_ts(pkt, inAud->time_base, outAud->time_base);
            pkt->stream_index = m_outAudioIndex;
            av_interleaved_write_frame(m_outFmt, pkt);
        }
        av_packet_unref(pkt);
    }

    // Flush encoder
    if (avcodec_send_frame(m_encCtx, nullptr) >= 0) {
        while (avcodec_receive_packet(m_encCtx, outPkt) >= 0) {
            outPkt->stream_index = m_outVideoIndex;
            av_packet_rescale_ts(outPkt, m_encCtx->time_base, outVideoStream->time_base);
            av_interleaved_write_frame(m_outFmt, outPkt);
            av_packet_unref(outPkt);
        }
    }

    av_write_trailer(m_outFmt);

    av_frame_free(&rawFrame);
    av_frame_free(&scaledFrame);
    av_packet_free(&pkt);
    av_packet_free(&outPkt);

    avio_closep(&m_outFmt->pb);

    // Atomic rename from .part to final proxy path
    if (QFile::exists(m_finalPath))
        QFile::remove(m_finalPath);

    if (!QFile::rename(m_tmpPath, m_finalPath)) {
        return fail(QCoreApplication::translate("ProxyRenderer", "Could not commit completed proxy file"));
    }

    m_tmpPath.clear(); // Prevent cleanup from deleting finished file
    return true;
}

} // namespace

bool ProxyRenderer::renderProxy(const QString &sourcePath, const QString &outPath,
                                int targetWidth, int targetHeight, QString *errorOut,
                                const std::function<bool(double)> &onProgress)
{
    ProxyTranscoder transcoder;
    return transcoder.run(sourcePath, outPath, targetWidth, targetHeight, errorOut, onProgress);
}

} // namespace drift
