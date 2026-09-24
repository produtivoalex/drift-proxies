#include "SubtitleCue.h"

#include <QCoreApplication>
#include <QRegularExpression>

#include <algorithm>
#include <cmath>

namespace drift {

QString subtitleClipName(const QList<SubtitleCue> &cues)
{
    if (cues.isEmpty())
        return QCoreApplication::translate("SubtitleCue", "Subtitles");
    return QCoreApplication::translate("SubtitleCue", "Subtitles (%1)").arg(cues.size());
}

namespace {

struct TimedWord
{
    TimeUs startUs = 0;
    TimeUs endUs = 0;
    QString word; // may include a leading space (Whisper-style)
};

// Split "Hello world. How are you?" into ["Hello", " world.", " How", " are", " you?"] so
// joining reproduces the original spacing/punctuation.
QStringList tokenizeWords(const QString &text)
{
    static const QRegularExpression re(QStringLiteral(R"((\s*\S+))"));
    QStringList tokens;
    auto it = re.globalMatch(text);
    while (it.hasNext())
        tokens.append(it.next().captured(1));
    return tokens;
}

QList<TimedWord> wordsFromCue(const SubtitleCue &cue)
{
    const QString text = cue.text;
    const QStringList tokens = tokenizeWords(text);
    QList<TimedWord> words;
    if (tokens.isEmpty())
        return words;

    // Give each word a minimum base weight (3) plus character length so single-letter words
    // like "é", "o", "a" receive natural human pronunciation duration (~200ms+) rather than
    // a vanishingly small fraction that collapses to zero.
    int totalWeight = 0;
    QList<int> weights;
    weights.reserve(tokens.size());
    for (const QString &tok : tokens) {
        const int w = 3 + std::max(1, static_cast<int>(tok.trimmed().size()));
        weights.append(w);
        totalWeight += w;
    }

    const TimeUs span = std::max<TimeUs>(1, cue.endUs - cue.startUs);
    int accWeight = 0;
    for (int i = 0; i < tokens.size(); ++i) {
        TimedWord tw;
        tw.word = tokens.at(i);
        tw.startUs = cue.startUs + static_cast<TimeUs>((static_cast<double>(accWeight) / totalWeight) * span);
        accWeight += weights.at(i);
        if (i + 1 == tokens.size()) {
            tw.endUs = cue.endUs;
        } else {
            tw.endUs = cue.startUs + static_cast<TimeUs>((static_cast<double>(accWeight) / totalWeight) * span);
        }
        if (tw.endUs <= tw.startUs)
            tw.endUs = tw.startUs + 100000; // minimum 100ms
        words.append(tw);
    }
    if (!words.isEmpty()) {
        words.last().endUs = std::max(words.last().endUs, words.last().startUs + 100000);
    }
    return words;
}

QList<TimedWord> flattenWords(const QList<SubtitleCue> &cues)
{
    QList<TimedWord> all;
    for (const SubtitleCue &cue : cues) {
        const QString trimmed = cue.text.trimmed();
        if (trimmed.isEmpty() || cue.endUs <= cue.startUs)
            continue;
        all += wordsFromCue(cue);
    }
    return all;
}

} // namespace

int activeWordIndexAt(const QString &text, TimeUs startUs, TimeUs endUs, TimeUs localUs)
{
    if (endUs <= startUs || localUs < startUs)
        return -1;

    SubtitleCue cue;
    cue.startUs = startUs;
    cue.endUs = endUs;
    cue.text = text;
    const QList<TimedWord> words = wordsFromCue(cue);
    for (int i = 0; i < words.size(); ++i) {
        if (localUs < words.at(i).endUs)
            return i;
    }
    // Past the last word's end (rounding, or the window overrunning the text): keep it lit.
    return words.isEmpty() ? -1 : words.size() - 1;
}

const SubtitleCue *activeSubtitleCueAt(const QList<SubtitleCue> &cues, TimeUs localUs)
{
    for (const SubtitleCue &cue : cues) {
        if (localUs >= cue.startUs && localUs < cue.endUs)
            return &cue;
    }
    return nullptr;
}

int subtitleCueIndexAt(const QList<SubtitleCue> &cues, TimeUs localUs)
{
    for (int i = 0; i < cues.size(); ++i) {
        const SubtitleCue &cue = cues.at(i);
        if (localUs >= cue.startUs && localUs < cue.endUs)
            return i;
    }
    return -1;
}

void sortSubtitleCues(QList<SubtitleCue> &cues)
{
    std::sort(cues.begin(), cues.end(), [](const SubtitleCue &a, const SubtitleCue &b) {
        if (a.startUs != b.startUs)
            return a.startUs < b.startUs;
        return a.endUs < b.endUs;
    });
}

QList<SubtitleCue> packSubtitleCues(const QList<SubtitleCue> &cues, int maxLineWidth,
                                    int maxLineCount, int maxWordsPerCue)
{
    if (cues.isEmpty())
        return {};

    // Mirror openai-whisper SubtitlesWriter: packing only applies when both limits are set.
    if (maxLineWidth <= 0 || maxLineCount <= 0)
        return cues;

    const QList<TimedWord> words = flattenWords(cues);
    if (words.isEmpty())
        return {};

    constexpr TimeUs kLongPauseUs = 3 * kUsPerSecond;

    QList<SubtitleCue> packed;
    QList<TimedWord> subtitle;
    int lineLen = 0;
    int lineCount = 1;
    int wordCount = 0;
    TimeUs lastStart = words.first().startUs;

    auto flush = [&]() {
        if (subtitle.isEmpty())
            return;
        SubtitleCue cue;
        cue.startUs = subtitle.first().startUs;
        cue.endUs = subtitle.last().endUs;
        QString text;
        for (const TimedWord &w : subtitle) {
            const QString item = w.word;
            if (item.isEmpty())
                continue;
            if (!text.isEmpty() && !text.endsWith(QLatin1Char(' ')) && !text.endsWith(QLatin1Char('\n'))
                && !item.startsWith(QLatin1Char(' ')) && !item.startsWith(QLatin1Char('\n'))
                && !item.startsWith(QLatin1Char('.')) && !item.startsWith(QLatin1Char(','))
                && !item.startsWith(QLatin1Char('!')) && !item.startsWith(QLatin1Char('?'))
                && !item.startsWith(QLatin1Char(':')) && !item.startsWith(QLatin1Char(';'))
                && !item.startsWith(QLatin1Char(')')) && !item.startsWith(QLatin1Char(']'))) {
                text += QLatin1Char(' ');
            }
            text += item;
        }
        cue.text = text.trimmed().replace(QLatin1Char('\n'), QLatin1Char(' '));
        if (!cue.text.isEmpty()) {
            if (cue.endUs <= cue.startUs)
                cue.endUs = cue.startUs + 150000; // minimum 150ms so short words never vanish
            packed.append(cue);
        }
        subtitle.clear();
        lineLen = 0;
        lineCount = 1;
        wordCount = 0;
    };

    for (TimedWord timing : words) {
        const bool longPause = timing.startUs - lastStart > kLongPauseUs;
        const bool hasRoom = lineLen + timing.word.size() <= maxLineWidth;
        const bool wordCapHit = maxWordsPerCue > 0 && wordCount >= maxWordsPerCue;

        if (lineLen > 0 && hasRoom && !longPause && !wordCapHit) {
            lineLen += timing.word.size();
            subtitle.append(timing);
            ++wordCount;
        } else {
            timing.word = timing.word.trimmed();
            if (!subtitle.isEmpty() && (longPause || wordCapHit || lineCount >= maxLineCount)) {
                flush();
            } else if (lineLen > 0) {
                ++lineCount;
                timing.word = QLatin1Char('\n') + timing.word;
            }
            lineLen = timing.word.trimmed().size();
            subtitle.append(timing);
            ++wordCount;
        }
        lastStart = timing.startUs;
    }
    flush();

    sortSubtitleCues(packed);
    return packed;
}

QString formatSubtitleText(const QString &text, SubtitleCapitalization cap)
{
    if (text.isEmpty())
        return text;

    switch (cap) {
    case SubtitleCapitalization::Original:
        return text;
    case SubtitleCapitalization::AllCaps:
        return text.toUpper();
    case SubtitleCapitalization::Lowercase:
        return text.toLower();
    case SubtitleCapitalization::SentenceCase: {
        QString lower = text.toLower();
        bool newSentence = true;
        for (int i = 0; i < lower.size(); ++i) {
            const QChar c = lower.at(i);
            if (newSentence && c.isLetter()) {
                lower[i] = c.toUpper();
                newSentence = false;
            } else if (c == QLatin1Char('.') || c == QLatin1Char('!') || c == QLatin1Char('?') || c == QLatin1Char('\n')) {
                newSentence = true;
            }
        }
        return lower;
    }
    case SubtitleCapitalization::TitleCase: {
        QString lower = text.toLower();
        bool newWord = true;
        for (int i = 0; i < lower.size(); ++i) {
            const QChar c = lower.at(i);
            if (newWord && c.isLetter()) {
                lower[i] = c.toUpper();
                newWord = false;
            } else if (!c.isLetter() && !c.isDigit()) {
                newWord = true;
            }
        }
        return lower;
    }
    }
    return text;
}

QString enrichSubtitleTextWithEmojis(const QString &text)
{
    if (text.trimmed().isEmpty())
        return text;

    struct EmojiRule {
        const char *emojiUtf8;
        const char *const *keywords;
    };

    static const char *kMoneyWords[] = {
        "dinheiro", "lucro", "riqueza", "milhao", "milhão", "grana", "faturamento", "pagar",
        "preco", "preço", "investir", "venda", "vendas", "vender", "money", "cash", "rich",
        "wealth", "profit", "million", "dollar", "pay", "invest", "billion", "dolares", "dólares",
        nullptr
    };

    static const char *kFireWords[] = {
        "fogo", "quente", "viral", "bombando", "chama", "tendencia", "tendência", "hype",
        "fire", "hot", "trend", "trending", "flame", "famoso", "bombou",
        nullptr
    };

    static const char *kIdeaWords[] = {
        "ideia", "sacada", "segredo", "dica", "mente", "cerebro", "cérebro", "insight",
        "pensar", "pensamento", "idea", "secret", "tip", "mind", "brain", "smart", "truque",
        nullptr
    };

    static const char *kTargetWords[] = {
        "alvo", "meta", "foco", "objetivo", "estrategia", "estratégia", "focado", "direcao",
        "target", "focus", "goal", "strategy", "aim", "disciplina",
        nullptr
    };

    static const char *kSpeedWords[] = {
        "rapido", "rápido", "tempo", "velocidade", "agora", "urgente", "minuto", "segundo",
        "instantaneo", "instantâneo", "fast", "speed", "quick", "time", "now", "urgent", "rush",
        nullptr
    };

    static const char *kRocketWords[] = {
        "foguete", "crescer", "escalar", "subir", "top", "explosao", "explosão", "lancamento",
        "lançamento", "avanco", "avanço", "rocket", "growth", "scale", "moon", "launch", "explode",
        nullptr
    };

    static const char *kWarningWords[] = {
        "atencao", "atenção", "cuidado", "pare", "perigo", "aviso", "alerta", "stop", "warning",
        "alert", "danger", "caution", "careful", "proibido",
        nullptr
    };

    static const char *kShockWords[] = {
        "choque", "uau", "caramba", "inacreditavel", "inacreditável", "loucura", "absurdo",
        "shock", "wow", "crazy", "insane", "omg", "unbelievable", "chocado",
        nullptr
    };

    static const char *kHeartWords[] = {
        "amor", "amar", "paixao", "paixão", "coracao", "coração", "adorar", "love", "heart",
        "passion", "adore", "apaixonado",
        nullptr
    };

    static const char *kTrophyWords[] = {
        "vitoria", "vitória", "trofeu", "troféu", "vencer", "campeao", "campeão", "sucesso",
        "vencedor", "ganhar", "ganhou", "win", "winner", "trophy", "champion", "victory", "success",
        nullptr
    };

    static const char *kEyeWords[] = {
        "olha", "olhar", "veja", "assista", "repare", "espia", "look", "watch", "see", "eye",
        "witness", "olhem",
        nullptr
    };

    static const char *kPowerWords[] = {
        "forca", "força", "poder", "treino", "academia", "forte", "firme", "strong", "power",
        "muscle", "gym", "workout", "hard", "potencia", "potência",
        nullptr
    };

    static const char *kSparkleWords[] = {
        "magica", "mágica", "magico", "mágico", "incrivel", "incrível", "brilho", "estrela",
        "show", "magic", "star", "glow", "sparkle", "wonder", "maravilha",
        nullptr
    };

    static const char *kCrossWords[] = {
        "erro", "falha", "nunca", "perder", "errado", "bloqueio", "ban", "perdeu", "wrong",
        "fail", "lose", "never", "error", "block", "mentira",
        nullptr
    };

    static const char *kCheckWords[] = {
        "certo", "verdade", "sim", "perfeito", "correto", "concluido", "concluído", "feito",
        "right", "yes", "true", "check", "done", "correct", "perfect", "aprovado",
        nullptr
    };

    static const char *kQuestionWords[] = {
        "duvida", "dúvida", "pergunta", "question", "doubt",
        nullptr
    };

    static const char *kMusicWords[] = {
        "musica", "música", "som", "batida", "ritmo", "cantar", "cancao", "canção", "audio",
        "áudio", "music", "song", "beat", "sound", "audio", "sing",
        nullptr
    };

    static const char *kLaughWords[] = {
        "engracado", "engraçado", "piada", "haha", "kkk", "lol", "risos", "funny", "laugh",
        "joke", "hilarious",
        nullptr
    };

    static const EmojiRule kRules[] = {
        { "💸", kMoneyWords },
        { "🔥", kFireWords },
        { "💡", kIdeaWords },
        { "🎯", kTargetWords },
        { "⚡", kSpeedWords },
        { "🚀", kRocketWords },
        { "⚠️", kWarningWords },
        { "😱", kShockWords },
        { "❤️", kHeartWords },
        { "🏆", kTrophyWords },
        { "👀", kEyeWords },
        { "💪", kPowerWords },
        { "✨", kSparkleWords },
        { "❌", kCrossWords },
        { "✅", kCheckWords },
        { "❓", kQuestionWords },
        { "🎵", kMusicWords },
        { "😂", kLaughWords }
    };

    // Clean text words into comparable tokens
    static const QRegularExpression wordCleanRe(QStringLiteral(R"([^\p{L}\p{N}]+)"));
    const QStringList rawWords = text.split(QRegularExpression(QStringLiteral(R"(\s+)")), Qt::SkipEmptyParts);

    QString matchedEmoji;
    for (const QString &raw : rawWords) {
        QString clean = raw.toLower();
        clean.remove(wordCleanRe);
        if (clean.isEmpty())
            continue;

        for (const EmojiRule &rule : kRules) {
            for (const char *const *kw = rule.keywords; *kw != nullptr; ++kw) {
                if (clean == QString::fromUtf8(*kw)) {
                    const QString emojiStr = QString::fromUtf8(rule.emojiUtf8);
                    if (!text.contains(emojiStr)) {
                        matchedEmoji = emojiStr;
                        break;
                    }
                }
            }
            if (!matchedEmoji.isEmpty())
                break;
        }
        if (!matchedEmoji.isEmpty())
            break;
    }

    if (!matchedEmoji.isEmpty()) {
        return text + QLatin1Char(' ') + matchedEmoji;
    }
    return text;
}

QList<SubtitleCue> enrichSubtitleCuesWithEmojis(const QList<SubtitleCue> &cues)
{
    QList<SubtitleCue> enriched = cues;
    for (SubtitleCue &cue : enriched) {
        cue.text = enrichSubtitleTextWithEmojis(cue.text);
    }
    return enriched;
}

QString correctPortugueseSpelling(const QString &text)
{
    if (text.trimmed().isEmpty())
        return text;

    static const QHash<QString, QString> kCorrections = {
        // Acoustic misinterpretations and non-existent words (reported by user & common Whisper artifacts)
        {QStringLiteral("esfero"), QStringLiteral("esfera")},
        {QStringLiteral("esferos"), QStringLiteral("esferas")},
        {QStringLiteral("conjuto"), QStringLiteral("conjunto")},
        {QStringLiteral("conjutos"), QStringLiteral("conjuntos")},
        {QStringLiteral("produtivida"), QStringLiteral("produtividade")},
        {QStringLiteral("producao"), QStringLiteral("produção")},
        {QStringLiteral("producão"), QStringLiteral("produção")},
        {QStringLiteral("atencao"), QStringLiteral("atenção")},
        {QStringLiteral("atencão"), QStringLiteral("atenção")},
        {QStringLiteral("voce"), QStringLiteral("você")},
        {QStringLiteral("voces"), QStringLiteral("vocês")},
        {QStringLiteral("tambem"), QStringLiteral("também")},
        {QStringLiteral("nao"), QStringLiteral("não")},
        {QStringLiteral("sao"), QStringLiteral("são")},
        {QStringLiteral("estao"), QStringLiteral("estão")},
        {QStringLiteral("entao"), QStringLiteral("então")},
        {QStringLiteral("irmao"), QStringLiteral("irmão")},
        {QStringLiteral("irmaos"), QStringLiteral("irmãos")},
        {QStringLiteral("facil"), QStringLiteral("fácil")},
        {QStringLiteral("dificil"), QStringLiteral("difícil")},
        {QStringLiteral("rapido"), QStringLiteral("rápido")},
        {QStringLiteral("rapida"), QStringLiteral("rápida")},
        {QStringLiteral("rapidos"), QStringLiteral("rápidos")},
        {QStringLiteral("rapidas"), QStringLiteral("rápidas")},
        {QStringLiteral("otimo"), QStringLiteral("ótimo")},
        {QStringLiteral("otima"), QStringLiteral("ótima")},
        {QStringLiteral("otimos"), QStringLiteral("ótimos")},
        {QStringLiteral("otimas"), QStringLiteral("ótimas")},
        {QStringLiteral("basico"), QStringLiteral("básico")},
        {QStringLiteral("basica"), QStringLiteral("básica")},
        {QStringLiteral("basicos"), QStringLiteral("básicos")},
        {QStringLiteral("basicas"), QStringLiteral("básicas")},
        {QStringLiteral("video"), QStringLiteral("vídeo")},
        {QStringLiteral("videos"), QStringLiteral("vídeos")},
        {QStringLiteral("audio"), QStringLiteral("áudio")},
        {QStringLiteral("audios"), QStringLiteral("áudios")},
        {QStringLiteral("camera"), QStringLiteral("câmera")},
        {QStringLiteral("cameras"), QStringLiteral("câmeras")},
        {QStringLiteral("conteudo"), QStringLiteral("conteúdo")},
        {QStringLiteral("conteudos"), QStringLiteral("conteúdos")},
        {QStringLiteral("musica"), QStringLiteral("música")},
        {QStringLiteral("musicas"), QStringLiteral("músicas")},
        {QStringLiteral("numero"), QStringLiteral("número")},
        {QStringLiteral("numeros"), QStringLiteral("números")},
        {QStringLiteral("unico"), QStringLiteral("único")},
        {QStringLiteral("unica"), QStringLiteral("única")},
        {QStringLiteral("unicos"), QStringLiteral("únicos")},
        {QStringLiteral("unicas"), QStringLiteral("únicas")},
        {QStringLiteral("tecnico"), QStringLiteral("técnico")},
        {QStringLiteral("tecnica"), QStringLiteral("técnica")},
        {QStringLiteral("tecnicos"), QStringLiteral("técnicos")},
        {QStringLiteral("tecnicas"), QStringLiteral("técnicas")},
        {QStringLiteral("logico"), QStringLiteral("lógico")},
        {QStringLiteral("logica"), QStringLiteral("lógica")},
        {QStringLiteral("diferenca"), QStringLiteral("diferença")},
        {QStringLiteral("diferencas"), QStringLiteral("diferenças")},
        {QStringLiteral("mudanca"), QStringLiteral("mudança")},
        {QStringLiteral("mudancas"), QStringLiteral("mudanças")},
        {QStringLiteral("presenca"), QStringLiteral("presença")},
        {QStringLiteral("experiencia"), QStringLiteral("experiência")},
        {QStringLiteral("experiencias"), QStringLiteral("experiências")},
        {QStringLiteral("referencia"), QStringLiteral("referência")},
        {QStringLiteral("referencias"), QStringLiteral("referências")},
        {QStringLiteral("consequencia"), QStringLiteral("consequência")},
        {QStringLiteral("consequencias"), QStringLiteral("consequências")},
        {QStringLiteral("informacao"), QStringLiteral("informação")},
        {QStringLiteral("informacoes"), QStringLiteral("informações")},
        {QStringLiteral("situacao"), QStringLiteral("situação")},
        {QStringLiteral("situacoes"), QStringLiteral("situações")},
        {QStringLiteral("comunicacao"), QStringLiteral("comunicação")},
        {QStringLiteral("apresentacao"), QStringLiteral("apresentação")},
        {QStringLiteral("edicao"), QStringLiteral("edição")},
        {QStringLiteral("edicoes"), QStringLiteral("edições")},
        {QStringLiteral("computacao"), QStringLiteral("computação")},
        {QStringLiteral("otimizacao"), QStringLiteral("otimização")},
        {QStringLiteral("padrao"), QStringLiteral("padrão")},
        {QStringLiteral("padroes"), QStringLiteral("padrões")},
        {QStringLiteral("visao"), QStringLiteral("visão")},
        {QStringLiteral("funcao"), QStringLiteral("função")},
        {QStringLiteral("funcoes"), QStringLiteral("funções")},
        {QStringLiteral("acao"), QStringLiteral("ação")},
        {QStringLiteral("acoes"), QStringLiteral("ações")},
        {QStringLiteral("reacao"), QStringLiteral("reação")},
        {QStringLiteral("reacoes"), QStringLiteral("reações")},
        {QStringLiteral("solucao"), QStringLiteral("solução")},
        {QStringLiteral("solucoes"), QStringLiteral("soluções")},
        {QStringLiteral("evolucao"), QStringLiteral("evolução")},
        {QStringLiteral("criacao"), QStringLiteral("criação")},
        {QStringLiteral("geracao"), QStringLiteral("geração")},
        {QStringLiteral("relacao"), QStringLiteral("relação")},
        {QStringLiteral("relacoes"), QStringLiteral("relações")},
        {QStringLiteral("condicao"), QStringLiteral("condição")},
        {QStringLiteral("condicoes"), QStringLiteral("condições")},
        {QStringLiteral("percepcao"), QStringLiteral("percepção")},
        {QStringLiteral("proximo"), QStringLiteral("próximo")},
        {QStringLiteral("proxima"), QStringLiteral("próxima")},
        {QStringLiteral("proximos"), QStringLiteral("próximos")},
        {QStringLiteral("proximas"), QStringLiteral("próximas")},
        {QStringLiteral("ate"), QStringLiteral("até")},
        {QStringLiteral("ja"), QStringLiteral("já")},
        {QStringLiteral("so"), QStringLiteral("só")},
        {QStringLiteral("possivel"), QStringLiteral("possível")},
        {QStringLiteral("impossivel"), QStringLiteral("impossível")}
    };

    static const QRegularExpression tokenRe(QStringLiteral(R"((\S+))"));
    QString result = text;
    QRegularExpressionMatchIterator it = tokenRe.globalMatch(text);
    QList<QPair<int, int>> matches;
    QStringList replacements;

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        const QString rawToken = match.captured(1);
        int leading = 0;
        while (leading < rawToken.size() && !rawToken.at(leading).isLetterOrNumber())
            ++leading;
        int trailing = 0;
        while (trailing < (rawToken.size() - leading) && !rawToken.at(rawToken.size() - 1 - trailing).isLetterOrNumber())
            ++trailing;

        if (leading + trailing >= rawToken.size())
            continue;

        const QString prefix = rawToken.left(leading);
        const QString suffix = rawToken.right(trailing);
        const QString core = rawToken.mid(leading, rawToken.size() - leading - trailing);
        const QString coreLower = core.toLower();

        if (kCorrections.contains(coreLower)) {
            QString corrected = kCorrections.value(coreLower);
            if (core.isUpper() && core.size() > 1) {
                corrected = corrected.toUpper();
            } else if (core.at(0).isUpper()) {
                corrected = corrected.left(1).toUpper() + corrected.mid(1);
            }
            matches.append({match.capturedStart(1), match.capturedLength(1)});
            replacements.append(prefix + corrected + suffix);
        }
    }

    for (int i = matches.size() - 1; i >= 0; --i) {
        result.replace(matches.at(i).first, matches.at(i).second, replacements.at(i));
    }
    return result;
}

QList<SubtitleCue> correctPortugueseSpelling(const QList<SubtitleCue> &cues)
{
    QList<SubtitleCue> corrected = cues;
    for (SubtitleCue &cue : corrected) {
        cue.text = correctPortugueseSpelling(cue.text);
    }
    return corrected;
}

} // namespace drift
