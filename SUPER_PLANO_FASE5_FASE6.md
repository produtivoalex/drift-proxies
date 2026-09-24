# SUPER PLANO DRIFT FASE 5 e 6

## VISAO GERAL: O QUE FALTA E POR QUE IMPORTA

| Fraqueza Atual | O que DarkPlanner/CapCut Faz | O que o Drift vai Fazer |
|---|---|---|
| Usuario precisa escrever o roteiro | DarkPlanner tem gerador com IA | Integracao nativa com LLMs (OpenAI + fallback local) |
| B-Rolls importados manualmente | CapCut tem library integrada | Auto-fetch Pexels/Pixabay direto na timeline |
| Sem traducao/multilingue | CapCut Pro tem dublagem | Clonagem de voz + dublagem 1 clique (5 idiomas) |
| Kanban sem automacao de upload | DarkPlanner publica pelo app | Upload direto YouTube/TikTok via OAuth |
| Sem geracao de Thumbnails | DarkPlanner sugere cover art | Motor de thumbnail IA com rosto detectado pelo SAM2 |
| Sem avatares/apresentadores | CapCut Pro tem avatares | Lip-sync 2D com modelo local + auto-reframe 9:16 |

---

## FASE 5: ROTEIRISTA IA + AUTO B-ROLL + INTERNACIONALIZACAO

### 5A - Roteirista com LLM Embutido (src/engine/ai/)

O usuario digita um tema, escolhe o nicho e o Drift escreve o roteiro completo otimizado para retencao, passando diretamente para o WizardEngine.

#### Backend C++: ScriptGenerator.h / ScriptGenerator.cpp

```cpp
// src/engine/ai/ScriptGenerator.h
#pragma once
#include <QObject>
#include <QString>
#include <QNetworkAccessManager>

namespace drift {

struct ScriptRequest {
    QString topic;         // "Os 5 maiores misterios do Egito"
    QString niche;         // "dark_mystery" | "finance" | "motivation" | "history" | "crime"
    QString format;        // "shorts_60s" | "youtube_8min" | "podcast_20min"
    QString language;      // "pt-BR" | "en-US" | "es-ES" | "de-DE" | "fr-FR"
    QString tone;          // "dramatic" | "calm" | "energetic" | "authoritative"
    int targetDurationSec; // 60, 480, 1200
};

struct GeneratedScript {
    QString hook;           // Gancho inicial de impacto (0-5s)
    QString body;           // Corpo - blocos semanticos separados por dupla quebra de linha
    QString callToAction;   // CTA final
    QStringList brollHints; // Keywords para busca automatica de B-Roll
    QStringList sfxHints;   // Sugestoes de SFX por bloco
    QString fullText;       // Texto completo para o WizardEngine
};

class ScriptGenerator : public QObject {
    Q_OBJECT
public:
    explicit ScriptGenerator(QObject *parent = nullptr);

    void setApiKey(const QString &key, const QString &provider = "openai");
    bool hasApiKey() const;
    void generateScript(const ScriptRequest &request);
    void cancel();

signals:
    void progressChanged(double fraction, const QString &status);
    void scriptGenerated(const GeneratedScript &script, bool success, const QString &error);

private:
    QString buildPrompt(const ScriptRequest &req) const;
    void callOpenAI(const QString &prompt);
    void callFallbackLocal(const QString &prompt); // llama.cpp Phi-3-Mini Q4_K_M

    QNetworkAccessManager *m_net;
    QString m_apiKey;
    QString m_provider; // "openai" | "anthropic" | "gemini" | "local"
    bool m_cancelled{false};
};

} // namespace drift
```

**Provedores suportados (fallback chain):**
1. OpenAI GPT-4o-mini (~$0.001/roteiro de 60s)
2. Anthropic Claude Haiku
3. Google Gemini Flash 1.5
4. Fallback Local: llama.cpp + Phi-3-Mini 3.8B quantizado (Q4_K_M, ~2.3GB) - offline, custo zero

**Prompt Engineering em buildPrompt():**
- System Role: roteirista viral de canais Dark do YouTube Brasil
- Output JSON estruturado: hook, body_blocks[], cta, broll_keywords[], sfx_suggestions[]
- Restricao: cada bloco com 2-4 frases de max 15 palavras (ideal para legendas virais)
- Tom por nicho via tabela de mapeamento embutida

**Metodos no AppController:**
```cpp
Q_INVOKABLE void configureScriptApiKey(const QString &key, const QString &provider);
Q_INVOKABLE bool hasScriptApiKey() const;
Q_INVOKABLE void generateScript(const QString &topic, const QString &niche,
                                 const QString &format, const QString &language,
                                 const QString &tone, int targetDurationSec);
Q_INVOKABLE void cancelScriptGeneration();
Q_INVOKABLE void generateTimelineFromScript(const QString &fullText,
                                             const QString &vibe, const QString &voiceId);
void scriptGenerationProgress(double fraction, QString status);
void scriptReady(QString hook, QString body, QString cta,
                 QStringList brollHints, QStringList sfxHints);
void scriptError(QString message);
```

**UI QML: DarkStudioPanel.qml (src/qml/views/)**
```
Etapa 1 - Tema e Nicho
  [Campo: Digite o tema do seu video...]
  Nicho: [Misterio] [Financas] [Motivacao] [Crime] [Historia] [Ciencia]
  Formato: [Shorts 60s] [YouTube 8min] [Podcast 20min]

Etapa 2 - Voz e Idioma
  Voz: [Antonio Epico] [Thalita Viral] [Francisca] [Fabio] [Yara]
  Idioma: [PT-BR] [EN] [ES] [DE] [FR]

Etapa 3 - Acao
  [GERAR ROTEIRO + TIMELINE MAGICA]
  [Preview editavel do roteiro]
  [CONFIRMAR E GERAR TIMELINE]
```

---

### 5B - Auto-Fetch de B-Rolls via API (src/engine/media/)

O WizardEngine detecta palavras-chave nos blocos do roteiro, busca automaticamente stock footage, baixa em background e insere na track de B-Roll.

#### Backend C++: StockFootageFetcher.h / .cpp

```cpp
// src/engine/media/StockFootageFetcher.h
#pragma once
#include <QObject>
#include <QString>
#include <QNetworkAccessManager>

namespace drift {

struct StockResult {
    QString videoUrl;
    QString previewUrl;
    QString author;
    QString license; // "CC0" | "Pexels" | "Pixabay"
    int durationMs;
    int width, height;
};

class StockFootageFetcher : public QObject {
    Q_OBJECT
public:
    explicit StockFootageFetcher(QObject *parent = nullptr);

    void setPexelsApiKey(const QString &key);
    void setPixabayApiKey(const QString &key);
    void search(const QString &keyword, const QString &orientation = "portrait",
                int maxResults = 5);
    void download(const StockResult &result, const QString &destinationDir);

signals:
    void resultsReady(QList<StockResult> results, QString keyword);
    void downloadProgress(double fraction);
    void downloadFinished(QString localPath, QString keyword, bool success);

private:
    void searchPexels(const QString &keyword, const QString &orientation, int max);
    void searchPixabay(const QString &keyword, const QString &orientation, int max);
    QNetworkAccessManager *m_net;
    QString m_pexelsKey, m_pixabayKey;
};

} // namespace drift
```

**Integracao no WizardEngine::processAsync():**
1. Extrair brollHints de cada bloco (gerados pelo ScriptGenerator)
2. Chamar StockFootageFetcher::search(hint, "portrait", 3) em paralelo (QtConcurrent)
3. Selecionar resultado mais relevante por duracao x relevancia
4. Download para %AppData%/drift/broll_cache/<md5_keyword>.mp4
5. Inserir clip na brollTrack com duracao exata do bloco de fala
6. Aplicar ClipAnimKind::KenBurns automaticamente
7. Cache: se hash existir e tiver menos de 30 dias, reutiliza sem nova chamada

---

### 5C - Internacionalizacao: Dublagem Multi-idioma (src/engine/i18n/)

Com 1 clique, expande o projeto para 5 idiomas simultaneos, traduzindo, gerando voz e exportando versoes separadas.

#### Backend C++: ProjectLocalizer.h / .cpp

```cpp
// src/engine/i18n/ProjectLocalizer.h
#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QNetworkAccessManager>

namespace drift {

class Project;

struct LocalizationConfig {
    QStringList targetLanguages;   // ["en-US", "es-ES", "de-DE", "fr-FR"]
    QString sourceLanguage;        // "pt-BR"
    bool cloneVoiceTone{true};
    bool preserveSubtitleStyle{true};
};

class ProjectLocalizer : public QObject {
    Q_OBJECT
public:
    explicit ProjectLocalizer(QObject *parent = nullptr);

    // Traduz SubtitleCues via DeepL API (gratis ate 500k chars/mes)
    void translateSubtitles(Project *project, const LocalizationConfig &config);

    // Sintetiza audio traduzido com voz correta do idioma alvo via TtsSynthesizer
    void synthesizeAudio(Project *project, const QString &targetLang);

    // Cria copia .drift com audio e legendas substituidos
    void exportLocalization(Project *project, const QString &targetLang,
                            const QString &outputDir);

    // Pipeline completo: traduz + sintetiza + exporta todas as versoes
    void localizeAll(Project *project, const LocalizationConfig &config);
    void cancel();

signals:
    void progress(double fraction, QString status, QString currentLanguage);
    void languageReady(QString language, QString projectFilePath);
    void allDone(QStringList generatedPaths, bool success);

private:
    QString translateText(const QString &text, const QString &from, const QString &to);
    QNetworkAccessManager *m_net;
    bool m_cancelled{false};
};

} // namespace drift
```

**Mapeamento de vozes OneCore por idioma:**
- pt-BR: Antonio, Thalita, Francisca, Fabio, Yara (ja implementadas)
- en-US: Microsoft Ryan Online, Jenny, Aria
- es-ES: Alvaro, Elvira, Dalia
- de-DE: Conrad, Katja
- fr-FR: Henri, Denise

**UI QML: LocalizationPanel.qml**
- Mapa visual com bandeiras clicaveis por idioma
- Barra de progresso por idioma: Traduzindo -> Gerando Voz -> Pronto
- Botao "Globalizar Agora" na coluna Publicado do Kanban

---

## FASE 6: PUBLICACAO DIRETA + THUMBNAIL IA + AVATARES + AUTO-REFRAME

### 6A - Publicacao Direta nas Redes (src/models/publishing/)

Da coluna Publicado do Kanban, agenda e publica o video diretamente nas redes sociais.

#### Backend C++: PublishingManager.h / .cpp

```cpp
// src/models/publishing/PublishingManager.h
#pragma once
#include <QObject>
#include <QString>
#include <QDateTime>
#include <QNetworkAccessManager>

namespace drift {

enum class Platform { YouTube, TikTok, InstagramReels, TwitterX };

struct PublishJob {
    Platform platform;
    QString videoFilePath;
    QString title;
    QString description;
    QStringList tags;
    QString thumbnailPath;
    QDateTime scheduledAt; // QDateTime() = publicar agora
    bool isPublic{true};
    QString playlistId;    // YouTube only
    QString language;
};

struct PublishResult {
    bool success;
    QString videoUrl;
    QString error;
    QString platformVideoId;
};

class PublishingManager : public QObject {
    Q_OBJECT
public:
    explicit PublishingManager(QObject *parent = nullptr);

    // OAuth2: abre browser sistema para autorizar
    void authenticateYouTube();
    void authenticateTikTok();
    void authenticateInstagram();

    bool isAuthenticatedFor(Platform platform) const;
    QString accountNameFor(Platform platform) const;
    void publish(const PublishJob &job);
    void cancelScheduled(const QString &jobId);
    QList<PublishJob> scheduledJobs() const;

signals:
    void authenticationSuccess(Platform platform, QString accountName);
    void authenticationFailed(Platform platform, QString error);
    void publishProgress(double fraction, QString status);
    void publishFinished(PublishResult result);
    void scheduledJobsChanged();

private:
    void uploadToYouTube(const PublishJob &job);
    void uploadToTikTok(const PublishJob &job);
    void uploadToInstagram(const PublishJob &job);
    QMap<Platform, QString> m_accessTokens;
    QMap<Platform, QString> m_accountNames;
    QList<PublishJob> m_scheduledJobs;
    QNetworkAccessManager *m_net;
};

} // namespace drift
```

**Implementacao OAuth2:**
- QDesktopServices::openUrl() com link de consent da plataforma
- Servidor HTTP local temporario em localhost:8374 para capturar callback
- access_token + refresh_token encriptado em QSettings (AES-256)
- Auto-renovacao do token antes de expirar

**UI: PublishingPanel.qml integrado ao PipelineView.qml (coluna Publicado)**
- Titulo auto-preenchido pelo ScriptGenerator
- Tags em chips editaveis
- Seletor de data/hora de agendamento
- Botoes: YouTube / TikTok / Instagram / Todas
- Botoes: PUBLICAR AGORA / AGENDAR

---

### 6B - Gerador de Thumbnails com IA (src/engine/thumbnail/)

Analisa o video, detecta o melhor frame com rosto expressivo via FaceLandmarker, aplica overlays e gera thumbnail de alta conversao.

#### Backend C++: ThumbnailGenerator.h / .cpp

```cpp
// src/engine/thumbnail/ThumbnailGenerator.h
#pragma once
#include <QObject>
#include <QString>
#include <QImage>

namespace drift {

class Project;

struct ThumbnailStyle {
    QString templateId;      // "dark_mystery" | "finance_gold" | "viral_react"
    QString title;           // Max 4 palavras para o overlay
    QString subtitle;
    QString backgroundColor;
    bool useGlowEffect{true};
    bool useArrowOverlay{true};
    bool useEmojiBadge{false};
    QString emojiBadge;
};

struct ThumbnailCandidate {
    QImage image;
    double faceDetectionScore;
    double expressionIntensity;
    int frameIndex;
    double timestampSec;
};

class ThumbnailGenerator : public QObject {
    Q_OBJECT
public:
    explicit ThumbnailGenerator(QObject *parent = nullptr);

    // Varre o video buscando frames com rosto expressivo via FaceLandmarker.h
    void findBestFrames(Project *project, int trackIndex, int clipIndex,
                        int candidateCount = 5);

    // Aplica template e gera thumbnail 1280x720 (YouTube) ou 1080x1920 (Shorts)
    void generateThumbnail(const ThumbnailCandidate &frame, const ThumbnailStyle &style,
                           const QString &outputPath);

    // Gera 3 variantes automaticas
    void generateVariants(Project *project, int trackIndex, int clipIndex,
                          const QString &title, const QString &outputDir);

signals:
    void candidatesFound(QList<ThumbnailCandidate> candidates);
    void thumbnailGenerated(QString path, int variantIndex);
    void allVariantsReady(QStringList paths);

private:
    double scoreFrame(const QImage &frame);
    void applyTemplate(QImage &frame, const ThumbnailStyle &style);
    void applyGlowText(QImage &frame, const QString &text, const QColor &color);
    void applyVignetteEffect(QImage &frame);
};

} // namespace drift
```

**3 Templates automaticos gerados:**
1. Dark Mystery: fundo preto degradee, texto branco com borda vermelha, icone interrogacao
2. Finance Gold: gradiente dourado, texto preto bold, badges de dinheiro
3. Viral React: rosto recortado SAM2 em fundo colorido, texto GIGANTE, setas

**UI: ThumbnailPreview.qml no Kanban**
- Grid de 3 variantes ao lado do card de publicacao
- Clique para selecionar qual usar no upload
- Editor inline: trocar texto, cor, emoji badge

---

### 6C - Avatares 2D com Lip-Sync (src/engine/avatar/)

Adiciona avatar apresentador animado que faz lip-sync com o audio. Alternativa para criadores que nao aparecem no video.

#### Backend C++: AvatarRenderer.h / .cpp

```cpp
// src/engine/avatar/AvatarRenderer.h
#pragma once
#include <QObject>
#include <QString>
#include <QImage>
#include <vector>

namespace drift {

enum class AvatarMouthShape {
    Closed, SlightOpen, Open, WideOpen, Round
};

struct AvatarKeyframe {
    double timestampSec;
    AvatarMouthShape mouthShape;
    float eyeBlink;
    float headTilt;
    float expressionIntensity;
};

class AvatarRenderer : public QObject {
    Q_OBJECT
public:
    explicit AvatarRenderer(QObject *parent = nullptr);

    bool loadAvatar(const QString &avatarPackPath); // .driftavatar = ZIP com sprites PNG
    void analyzeAudio(const QString &audioPath);    // FFT por bandas para detectar visemas
    QImage renderFrame(double timestampSec, int width, int height);
    void exportFrameSequence(const QString &audioPath, const QString &outputDir,
                             int width, int height, double fps);

signals:
    void analysisProgress(double fraction);
    void analysisComplete(QList<AvatarKeyframe> keyframes);
    void exportProgress(double fraction);
    void exportComplete(QString framesDir, int frameCount);

private:
    QMap<AvatarMouthShape, QImage> m_sprites;
    QList<AvatarKeyframe> m_keyframes;
    float detectMouthShape(const std::vector<float> &audioChunk) const;
};

} // namespace drift
```

**Formato .driftavatar:**
- ZIP renomeado com sprites PNG por visema + metadata.json
- Distribuicao via Addon Manager do Drift
- 3 avatares bundled iniciais: Apresentador Dark, Apresentadora Jovem, Personagem Animado

---

### 6D - Auto-Reframe 9:16 com Face Tracking

Video 16:9 convertido para 9:16 automaticamente com sujeito sempre centralizado.

#### Metodos no AppController:

```cpp
// Em src/models/AppController.h

// Usa FaceTrack.cpp (ja implementado) para detectar XY do rosto em cada frame,
// aplica filtro de Kalman para suavizacao e gera keyframes em transform.positionX
Q_INVOKABLE void applyAutoReframe(int trackIndex, int clipIndex,
                                   const QString &targetAspect = "9:16",
                                   double smoothingFactor = 0.15);

Q_INVOKABLE void removeAutoReframe(int trackIndex, int clipIndex);
Q_INVOKABLE bool hasAutoReframe(int trackIndex, int clipIndex) const;

void autoReframeProgress(double fraction, QString status);
void autoReframeFinished(int trackIndex, int clipIndex, bool success);
```

**Algoritmo:**
1. FaceTrack.cpp detecta posicao XY do rosto por frame-chave
2. Filtro de Kalman suaviza a trajetoria (sem tremido)
3. Gera keyframes em transform.positionX do clipe
4. Quando nao ha rosto, camera para na ultima posicao conhecida

**Card em TransformInspector.qml:**
- Seletor de formato: 9:16 Shorts / 1:1 Instagram / 4:5 Feed
- Slider de suavizacao: Camera Suave (0.05) ate Camera Viva (0.5)
- Botoes: Aplicar / Remover

---

## CRONOGRAMA DE IMPLEMENTACAO POR PRIORIDADE

| # | Feature | Complexidade | Impacto | Duracao |
|---|---------|-------------|---------|---------|
| 1 | 5A - Roteirista LLM | Media | Alto | 1 sessao |
| 2 | 5B - Auto-fetch B-Rolls | Media | Alto | 1 sessao |
| 3 | 6D - Auto-Reframe 9:16 | Baixa (FaceTrack pronto) | Alto | 1 sessao |
| 4 | 6B - Thumbnails IA | Media | Alto | 1 sessao |
| 5 | 5C - Internacionalizacao | Alta | Alto | 2 sessoes |
| 6 | 6A - Publicacao Direta | Alta | Altissimo | 2 sessoes |
| 7 | 6C - Avatares Lip-Sync | Alta | Medio | 2 sessoes |

---

## ARVORE DE NOVOS ARQUIVOS

```
src/engine/
  ai/
    ScriptGenerator.h / .cpp         (5A)
  media/
    StockFootageFetcher.h / .cpp     (5B)
  i18n/
    ProjectLocalizer.h / .cpp        (5C)
  thumbnail/
    ThumbnailGenerator.h / .cpp      (6B)
  avatar/
    AvatarRenderer.h / .cpp          (6C)

src/models/publishing/
  PublishingManager.h / .cpp         (6A)

src/qml/views/
  DarkStudioPanel.qml                (5A - UI Roteirista)
  LocalizationPanel.qml              (5C - UI Traducao)
  PublishingPanel.qml                (6A - UI Publicacao)

src/qml/components/
  ThumbnailPreview.qml               (6B)
  AvatarSelector.qml                 (6C)
```

---

## APIS E DEPENDENCIAS EXTERNAS

| Servico | Uso | Custo | Auth |
|---------|-----|-------|------|
| OpenAI API | GPT-4o-mini para roteiro | ~$0.001/roteiro | Key usuario |
| DeepL API | Traducao profissional | Gratis ate 500k chars/mes | Key usuario |
| Pexels API | Stock footage (B-Rolls) | 100% GRATUITO | Key publica |
| Pixabay API | Stock footage fallback | 100% GRATUITO | Key publica |
| YouTube Data API v3 | Upload e agendamento | Gratis (quotas) | OAuth2 Google |
| TikTok Content Posting API | Upload TikTok | Gratis (aprovacao) | OAuth2 TikTok |
| Meta Graph API | Upload Instagram | Gratis (aprovacao) | OAuth2 Meta |

Zero Lock-in: todas as chaves configuradas pelo usuario, armazenadas localmente com AES-256.

---

## TABELA FINAL: DRIFT vs DARKPLANNER vs CAPCUT PRO

| Recurso | DarkPlanner | CapCut Pro | Drift pos Fase 6 |
|---|---|---|---|
| Edicao Profissional | NAO | Basico | Estudio completo GPU/Skia |
| Geracao de Roteiro | GPT basico | NAO | Multi-LLM + local offline |
| B-Roll Automatico | NAO | NAO | Pexels/Pixabay automatico |
| Avatares | NAO | SIM (pago) | Local, sem mensalidade |
| Publicacao Direta | YouTube | NAO | YT + TikTok + Instagram |
| Dublagem Multi-idioma | NAO | SIM (pago) | 5 idiomas gratis |
| Thumbnail IA | Basico | NAO | SAM2 + templates virais |
| Auto-Reframe | NAO | SIM | FaceTrack nativo |
| Custo de assinatura | Mensal | Mensal | Open Source (gratis) |
| IA totalmente local | NAO | NAO | SIM (Whisper + ONNX + llama.cpp) |
