#include "engine/VirtualBackgrounds.h"

namespace drift {

const QList<VirtualBackgroundPreset> &VirtualBackgroundCatalog::presets()
{
    static const QList<VirtualBackgroundPreset> s_presets = {
        // --- 1. PODCAST & HOME STUDIO ---
        {
            QStringLiteral("podcast-warm"),
            QStringLiteral("🎙️ Podcast Studio Warm"),
            QStringLiteral("podcast"),
            QStringLiteral("Podcast Pro"),
            QStringLiteral("Iluminação quente amadeirada estilo estúdio de podcast profissional com halo âmbar suave."),
            QStringLiteral("gradient"),
            QColor(24, 16, 12),     // Dark mahogany / espresso
            QColor(72, 42, 22),     // Warm walnut
            QColor(220, 140, 60),   // Warm amber backlight
            1,                      // Radial
            0.15
        },
        {
            QStringLiteral("podcast-dark-neon"),
            QStringLiteral("🎙️ Podcast Dark & Neon"),
            QStringLiteral("podcast"),
            QStringLiteral("Neon Studio"),
            QStringLiteral("Fundo escuro e intimista com iluminação de borda ciano e magenta de estúdio de gravação."),
            QStringLiteral("gradient"),
            QColor(10, 10, 18),     // Midnight blue
            QColor(28, 16, 38),     // Deep purple
            QColor(0, 210, 240),    // Vibrant cyan accent
            0,                      // Linear
            0.10
        },

        // --- 2. ESCRITÓRIO & TECH ---
        {
            QStringLiteral("studio-executive"),
            QStringLiteral("🏢 Escritório Executivo Minimalista"),
            QStringLiteral("studio"),
            QStringLiteral("4K Estúdio"),
            QStringLiteral("Visual corporativo moderno em tons neutros de vidro e alumínio com profundidade de campo suave."),
            QStringLiteral("gradient"),
            QColor(20, 24, 28),     // Charcoal slate
            QColor(45, 52, 60),     // Steel grey
            QColor(180, 205, 230),  // Cool glass reflection
            1,                      // Radial
            0.20
        },
        {
            QStringLiteral("studio-loft"),
            QStringLiteral("🛋️ Loft Urbano Contemporâneo"),
            QStringLiteral("studio"),
            QStringLiteral("Warm Loft"),
            QStringLiteral("Estética contemporânea com tons terrosos quentes e iluminação suave de janela matinal."),
            QStringLiteral("gradient"),
            QColor(26, 20, 18),     // Dark brick base
            QColor(60, 48, 42),     // Terracotta earth
            QColor(240, 210, 180),  // Soft morning sun
            0,                      // Linear
            0.15
        },

        // --- 3. CINEMATOGRÁFICOS & GRADIENTES DE ESTÚDIO ---
        {
            QStringLiteral("studio-cyclorama-grey"),
            QStringLiteral("📷 Ciclorama Infinito (Estúdio Cinza)"),
            QStringLiteral("gradients"),
            QStringLiteral("Foto Studio"),
            QStringLiteral("Fundo neutro cinematográfico de estúdio fotográfico com iluminação suave no centro."),
            QStringLiteral("gradient"),
            QColor(18, 18, 18),     // Edge vignette
            QColor(48, 48, 52),     // Studio grey
            QColor(110, 110, 120),  // Center key light
            1,                      // Radial
            0.05
        },
        {
            QStringLiteral("studio-deep-blue"),
            QStringLiteral("🌌 Dark Blue Vignette"),
            QStringLiteral("gradients"),
            QStringLiteral("Cinema HDR"),
            QStringLiteral("Azul marinho escuro profundo com vinheta cinematográfica de alto contraste para apresentadores."),
            QStringLiteral("gradient"),
            QColor(6, 12, 24),      // Deep navy vignette
            QColor(16, 32, 64),     // Midnight royal blue
            QColor(60, 120, 210),   // Rim highlight
            1,                      // Radial
            0.10
        },
        {
            QStringLiteral("studio-clean-light"),
            QStringLiteral("☀️ Estúdio Clean Branco & Prata"),
            QStringLiteral("gradients"),
            QStringLiteral("Clean & Fresh"),
            QStringLiteral("Fundo claro e elegante em degradê de branco acetinado e cinza pérola para vídeos institucionais."),
            QStringLiteral("gradient"),
            QColor(200, 205, 210),  // Soft edge silver
            QColor(235, 240, 245),  // Pearl white
            QColor(255, 255, 255),  // Pure white center
            1,                      // Radial
            0.05
        },

        // --- 4. DINÂMICOS & TECH ---
        {
            QStringLiteral("dynamic-particles-gold"),
            QStringLiteral("✨ Partículas de Luz & Glow Dourado"),
            QStringLiteral("dynamic"),
            QStringLiteral("Dinâmico"),
            QStringLiteral("Fundo escuro enriquecido com halo de partículas de luz e brilho dourado sofisticado."),
            QStringLiteral("gradient"),
            QColor(12, 10, 8),      // Obsidian black
            QColor(40, 32, 20),     // Bronze dark
            QColor(255, 200, 80),   // Golden particle glow
            1,                      // Radial
            0.25
        },
        {
            QStringLiteral("dynamic-cyber-grid"),
            QStringLiteral("⚡ Cyberpunk Neon Grid"),
            QStringLiteral("tech"),
            QStringLiteral("Tech Neon"),
            QStringLiteral("Grade tridimensional futurista com iluminação violeta e reflexos ciano de alta energia."),
            QStringLiteral("gradient"),
            QColor(8, 6, 16),       // Cyber dark
            QColor(36, 12, 54),     // Synthwave purple
            QColor(0, 240, 200),    // Electric turquoise
            0,                      // Linear
            0.15
        },
        {
            QStringLiteral("dynamic-aurora"),
            QStringLiteral("🌈 Gradiente Fluido Apple Style"),
            QStringLiteral("dynamic"),
            QStringLiteral("Viral Trend"),
            QStringLiteral("Transição fluida e elegante de cores suaves estilo Keynote e produtos premium."),
            QStringLiteral("gradient"),
            QColor(22, 12, 38),     // Deep violet
            QColor(56, 24, 76),     // Electric magenta
            QColor(80, 180, 240),   // Sky cyan
            0,                      // Linear
            0.20
        }
    };

    return s_presets;
}

const VirtualBackgroundPreset *VirtualBackgroundCatalog::findPreset(const QString &id)
{
    const auto &all = presets();
    for (const auto &p : all) {
        if (p.id == id)
            return &p;
    }
    return all.isEmpty() ? nullptr : &all.first();
}

} // namespace drift
