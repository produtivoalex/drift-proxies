// src/engine/ai/StockFootageFetcher.h
// Fase 5B — Auto-Fetch B-Rolls
// Busca automaticamente vídeos de stock (B-Rolls) do Pexels e Pixabay
// usando as keywords geradas pelo ScriptGenerator.
//
// Fluxo:
//   1. Recebe uma lista de queries (broll_hints do ScriptGenerator)
//   2. Busca em paralelo no Pexels e/ou Pixabay (conforme keys configuradas)
//   3. Baixa os arquivos em cache local (AppData/broll_cache/<hash>.mp4)
//   4. Emite brollReady() com o caminho local de cada vídeo baixado
//
// As API keys são salvas em QSettings("ai/pexelsApiKey", "ai/pixabayApiKey").
// Pexels: https://www.pexels.com/api/ (gratuito, 200 req/hora)
// Pixabay: https://pixabay.com/api/docs/ (gratuito, 100 req/min)
#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QDir>

namespace drift {

// ── Resultado de uma busca individual ──────────────────────────────────────

struct BRollResult {
    QString query;          // Keyword que gerou esse resultado
    QString localPath;      // Caminho do arquivo baixado em cache
    QString previewUrl;     // URL da thumbnail JPEG para mostrar na UI
    QString videoUrl;       // URL original do vídeo (Pexels/Pixabay)
    int durationSec = 0;    // Duração do vídeo em segundos
    QString source;         // "pexels" ou "pixabay"
    int width = 0;
    int height = 0;
    bool success = false;
    QString error;
};

// ── Engine Principal ────────────────────────────────────────────────────────

class StockFootageFetcher : public QObject {
    Q_OBJECT
public:
    explicit StockFootageFetcher(QObject *parent = nullptr);
    ~StockFootageFetcher() override;

    // Configuração de API keys
    void setPexelsApiKey(const QString &key);
    void setPixabayApiKey(const QString &key);
    bool hasPexelsKey() const { return !m_pexelsKey.isEmpty(); }
    bool hasPixabayKey() const { return !m_pixabayKey.isEmpty(); }
    bool hasAnyKey() const { return hasPexelsKey() || hasPixabayKey(); }

    // Busca e baixa B-Rolls para uma lista de queries em paralelo.
    // `maxPerQuery`: máximo de vídeos por keyword (1-3 recomendado para Shorts).
    // `minDurationSec` / `maxDurationSec`: filtro de duração.
    // `portrait`: preferir vídeos 9:16 (True para Shorts/Reels, False para 16:9).
    void fetchBRolls(const QStringList &queries,
                     int maxPerQuery = 1,
                     int minDurationSec = 4,
                     int maxDurationSec = 30,
                     bool portrait = false);

    // Aborta todos os downloads em andamento
    void cancel();

    // Diretório de cache local
    static QString cacheDir();
    // Limpa arquivos de cache com mais de `olderThanDays` dias
    static void pruneCache(int olderThanDays = 7);

signals:
    // Progresso global: 0.0 → 1.0 à medida que cada query é processada
    void progressChanged(double fraction, const QString &status);
    // Emitido quando UM B-Roll fica pronto (localPath já está no disco)
    void brollReady(const drift::BRollResult &result);
    // Emitido quando TODOS os fetches do lote terminam
    void fetchFinished(int successCount, int failCount);

private:
    // Busca metadados via API (sem baixar o vídeo ainda)
    void searchPexels(const QString &query, int maxResults, int minSec, int maxSec, bool portrait);
    void searchPixabay(const QString &query, int maxResults, int minSec, int maxSec, bool portrait);

    // Baixa o vídeo para cache; emite brollReady() ao concluir
    void downloadVideo(const QString &query, const QString &videoUrl,
                       const QString &previewUrl, int durationSec,
                       int width, int height, const QString &source);

    // Escolhe a melhor URL de vídeo do resultado Pexels (HD→SD→Tiny)
    QString bestPexelsVideoUrl(const QJsonObject &videoFile) const;
    // Gera um nome de arquivo determinístico baseado na URL (evita re-download)
    QString cachePathFor(const QString &url) const;

    void onQueryDone();  // Chamado após cada query ser processada

    QNetworkAccessManager m_net;
    QString m_pexelsKey;
    QString m_pixabayKey;

    // Controle de concorrência
    QAtomicInt m_pendingQueries{0};
    QAtomicInt m_successCount{0};
    QAtomicInt m_failCount{0};
    QAtomicInt m_totalQueries{0};
    bool m_cancelled = false;
};

} // namespace drift
