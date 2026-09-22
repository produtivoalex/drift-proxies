#include "SfxCatalog.h"

#include <QCoreApplication>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

#include <algorithm>
#include <cmath>
#include <random>

namespace {

const QList<SfxCategory> kCategories = {
    {QStringLiteral("transicoes"), QStringLiteral("Transições & Dinâmica")},
    {QStringLiteral("impacto"), QStringLiteral("Impactos & Ênfase")},
    {QStringLiteral("interface"), QStringLiteral("Pop, Notificações & UI")},
};

const QList<SfxItem> kCatalog = {
    {QStringLiteral("whoosh_fast"), QStringLiteral("Whoosh Rápido"), QStringLiteral("transicoes"), 0.35, QStringLiteral("chevronsRight")},
    {QStringLiteral("whoosh_deep"), QStringLiteral("Whoosh Profundo"), QStringLiteral("transicoes"), 0.50, QStringLiteral("chevronsRight")},
    {QStringLiteral("glitch_rise"), QStringLiteral("Glitch Transição"), QStringLiteral("transicoes"), 0.28, QStringLiteral("sparkles")},

    {QStringLiteral("boom_bass"), QStringLiteral("Bass Drop / Boom"), QStringLiteral("impacto"), 0.85, QStringLiteral("volumeHigh")},
    {QStringLiteral("thud_punch"), QStringLiteral("Pancada Seca"), QStringLiteral("impacto"), 0.40, QStringLiteral("box")},

    {QStringLiteral("pop_clean"), QStringLiteral("Pop Bolha (Texto)"), QStringLiteral("interface"), 0.12, QStringLiteral("messageSquare")},
    {QStringLiteral("mouse_click"), QStringLiteral("Clique / Snap"), QStringLiteral("interface"), 0.08, QStringLiteral("mousePointer")},
    {QStringLiteral("camera_shutter"), QStringLiteral("Camera Shutter"), QStringLiteral("interface"), 0.22, QStringLiteral("camera")},
    {QStringLiteral("ding_bell"), QStringLiteral("Sino / Ding"), QStringLiteral("interface"), 0.65, QStringLiteral("bookmark")},
    {QStringLiteral("coin_success"), QStringLiteral("Moeda / Sucesso"), QStringLiteral("interface"), 0.38, QStringLiteral("star")},
};

bool writeWav(const QString &path, const QVector<int16_t> &samples, int sampleRate = 44100)
{
    QFileInfo info(path);
    QDir().mkpath(info.absolutePath());

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    QDataStream out(&file);
    out.setByteOrder(QDataStream::LittleEndian);

    const quint32 dataSize = static_cast<quint32>(samples.size() * sizeof(int16_t));
    const quint32 totalSize = 36 + dataSize;

    // RIFF header
    file.write("RIFF", 4);
    out << totalSize;
    file.write("WAVE", 4);

    // fmt subchunk
    file.write("fmt ", 4);
    out << static_cast<quint32>(16);
    out << static_cast<quint16>(1);  // PCM
    out << static_cast<quint16>(1);  // Mono
    out << static_cast<quint32>(sampleRate);
    out << static_cast<quint32>(sampleRate * sizeof(int16_t));
    out << static_cast<quint16>(sizeof(int16_t));
    out << static_cast<quint16>(16); // Bits

    // data subchunk
    file.write("data", 4);
    out << dataSize;

    for (int16_t s : samples)
        out << s;

    return true;
}

QVector<int16_t> synthesize(const QString &id)
{
    const int sr = 44100;
    const double pi = 3.14159265358979323846;

    if (id == QLatin1String("whoosh_fast") || id == QLatin1String("whoosh_deep")) {
        const double dur = (id == QLatin1String("whoosh_fast")) ? 0.35 : 0.50;
        const int n = static_cast<int>(dur * sr);
        QVector<int16_t> out(n);

        std::mt19937 rng(42);
        std::uniform_real_distribution<double> dist(-1.0, 1.0);

        double filterState = 0.0;
        for (int i = 0; i < n; ++i) {
            const double t = static_cast<double>(i) / n;
            // Gaussian bell curve envelope
            const double env = std::pow(std::sin(pi * t), 2.0);
            const double fStart = (id == QLatin1String("whoosh_fast")) ? 900.0 : 500.0;
            const double fEnd = (id == QLatin1String("whoosh_fast")) ? 180.0 : 70.0;
            const double freq = fStart * (1.0 - t) + fEnd * t;

            // Low-pass filtered noise
            const double alpha = std::min(1.0, 2.0 * pi * freq / sr);
            filterState += alpha * (dist(rng) - filterState);

            const double tone = std::sin(2.0 * pi * freq * (static_cast<double>(i) / sr));
            const double sample = env * (0.65 * filterState + 0.35 * tone);
            out[i] = static_cast<int16_t>(std::clamp(sample * 30000.0, -32767.0, 32767.0));
        }
        return out;
    }

    if (id == QLatin1String("pop_clean")) {
        const double dur = 0.12;
        const int n = static_cast<int>(dur * sr);
        QVector<int16_t> out(n);
        double phase = 0.0;
        for (int i = 0; i < n; ++i) {
            const double t = static_cast<double>(i) / sr;
            // Pitch sweeps from 450Hz to 1600Hz in first 20ms
            const double freq = (t < 0.02) ? 450.0 + (1600.0 - 450.0) * (t / 0.02) : 1600.0 * std::exp(-25.0 * (t - 0.02));
            phase += 2.0 * pi * freq / sr;
            const double env = std::exp(-40.0 * t);
            const double sample = env * std::sin(phase);
            out[i] = static_cast<int16_t>(std::clamp(sample * 31000.0, -32767.0, 32767.0));
        }
        return out;
    }

    if (id == QLatin1String("mouse_click")) {
        const double dur = 0.08;
        const int n = static_cast<int>(dur * sr);
        QVector<int16_t> out(n);
        for (int i = 0; i < n; ++i) {
            const double t = static_cast<double>(i) / sr;
            const double p1 = std::sin(2.0 * pi * 2800.0 * t) * std::exp(-250.0 * t);
            const double p2 = (t > 0.015) ? 0.6 * std::sin(2.0 * pi * 3400.0 * (t - 0.015)) * std::exp(-300.0 * (t - 0.015)) : 0.0;
            const double sample = p1 + p2;
            out[i] = static_cast<int16_t>(std::clamp(sample * 30000.0, -32767.0, 32767.0));
        }
        return out;
    }

    if (id == QLatin1String("camera_shutter")) {
        const double dur = 0.22;
        const int n = static_cast<int>(dur * sr);
        QVector<int16_t> out(n);
        for (int i = 0; i < n; ++i) {
            const double t = static_cast<double>(i) / sr;
            // Click 1: mirror up at t = 0
            const double c1 = std::sin(2.0 * pi * 1900.0 * t) * std::exp(-90.0 * t);
            // Click 2: shutter snap at t = 0.075
            const double c2 = (t > 0.075) ? 1.2 * std::sin(2.0 * pi * 2400.0 * (t - 0.075)) * std::exp(-80.0 * (t - 0.075)) : 0.0;
            const double sample = c1 + c2;
            out[i] = static_cast<int16_t>(std::clamp(sample * 28000.0, -32767.0, 32767.0));
        }
        return out;
    }

    if (id == QLatin1String("ding_bell")) {
        const double dur = 0.65;
        const int n = static_cast<int>(dur * sr);
        QVector<int16_t> out(n);
        for (int i = 0; i < n; ++i) {
            const double t = static_cast<double>(i) / sr;
            const double env = std::exp(-6.5 * t);
            const double f1 = std::sin(2.0 * pi * 1760.0 * t);
            const double f2 = 0.35 * std::sin(2.0 * pi * 3520.0 * t);
            const double f3 = 0.15 * std::sin(2.0 * pi * 5280.0 * t);
            const double sample = env * (f1 + f2 + f3);
            out[i] = static_cast<int16_t>(std::clamp(sample * 27000.0, -32767.0, 32767.0));
        }
        return out;
    }

    if (id == QLatin1String("coin_success")) {
        const double dur = 0.38;
        const int n = static_cast<int>(dur * sr);
        QVector<int16_t> out(n);
        for (int i = 0; i < n; ++i) {
            const double t = static_cast<double>(i) / sr;
            double sample = 0.0;
            if (t < 0.09) {
                sample = std::sin(2.0 * pi * 987.77 * t) * (1.0 - t / 0.09);
            } else {
                const double t2 = t - 0.09;
                const double env = std::exp(-7.5 * t2);
                sample = env * (std::sin(2.0 * pi * 1318.51 * t2) + 0.3 * std::sin(2.0 * pi * 2637.0 * t2));
            }
            out[i] = static_cast<int16_t>(std::clamp(sample * 28000.0, -32767.0, 32767.0));
        }
        return out;
    }

    if (id == QLatin1String("boom_bass")) {
        const double dur = 0.85;
        const int n = static_cast<int>(dur * sr);
        QVector<int16_t> out(n);
        double phase = 0.0;
        for (int i = 0; i < n; ++i) {
            const double t = static_cast<double>(i) / sr;
            const double freq = 120.0 * std::exp(-3.5 * t) + 38.0;
            phase += 2.0 * pi * freq / sr;
            const double env = std::min(1.0, t / 0.008) * std::exp(-3.2 * t);
            const double raw = std::sin(phase);
            const double sample = env * std::tanh(1.5 * raw);
            out[i] = static_cast<int16_t>(std::clamp(sample * 31000.0, -32767.0, 32767.0));
        }
        return out;
    }

    if (id == QLatin1String("thud_punch")) {
        const double dur = 0.40;
        const int n = static_cast<int>(dur * sr);
        QVector<int16_t> out(n);
        double phase = 0.0;
        for (int i = 0; i < n; ++i) {
            const double t = static_cast<double>(i) / sr;
            const double freq = 160.0 * std::exp(-12.0 * t) + 55.0;
            phase += 2.0 * pi * freq / sr;
            const double env = std::exp(-7.0 * t);
            const double sample = env * std::sin(phase);
            out[i] = static_cast<int16_t>(std::clamp(sample * 31000.0, -32767.0, 32767.0));
        }
        return out;
    }

    if (id == QLatin1String("glitch_rise")) {
        const double dur = 0.28;
        const int n = static_cast<int>(dur * sr);
        QVector<int16_t> out(n);
        std::mt19937 rng(1337);
        std::uniform_real_distribution<double> dist(-1.0, 1.0);
        for (int i = 0; i < n; ++i) {
            const double t = static_cast<double>(i) / sr;
            const double env = (t / dur) * (t / dur);
            const double mod = std::sin(2.0 * pi * 80.0 * t) > 0.0 ? 1.0 : -0.5;
            const double sample = env * mod * dist(rng);
            out[i] = static_cast<int16_t>(std::clamp(sample * 26000.0, -32767.0, 32767.0));
        }
        return out;
    }

    // Default 0.2s beep fallback
    const int n = static_cast<int>(0.2 * sr);
    QVector<int16_t> out(n);
    for (int i = 0; i < n; ++i) {
        const double t = static_cast<double>(i) / sr;
        out[i] = static_cast<int16_t>(std::sin(2.0 * pi * 440.0 * t) * std::exp(-5.0 * t) * 20000.0);
    }
    return out;
}

} // namespace

const QList<SfxCategory> &sfxCategories()
{
    return kCategories;
}

const QList<SfxItem> &sfxCatalog()
{
    return kCatalog;
}

QString sfxFilePath(const QString &id)
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString path = QDir(base).filePath(QStringLiteral("sfx/%1.wav").arg(id));

    if (!QFile::exists(path)) {
        const QVector<int16_t> pcm = synthesize(id);
        if (!writeWav(path, pcm))
            return {};
    }
    return path;
}
