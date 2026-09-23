# Drift Video Editor - Estado Atual do Projeto & Guia de Continuação
> **Documento de Contexto Instantâneo para o Antigravity / Agentes de IA**  
> **Data:** 23 de Setembro de 2026  
> **Branch Git:** `main` | **Repositório:** `https://github.com/produtivoalex/drift-proxies.git`  
> **Pasta Oficial do Projeto:** `C:\Users\Alex\Documents\Antigravity\drift`

---

## ⚠️ REGRA DE OURO / RESTRIÇÃO CRÍTICA
* **NÃO COMPILAR NADA AINDA**: O usuário determinou explicitamente que **nenhum comando de compilação deve ser executado** (não rode `cmake --build`, `ninja`, `msbuild` ou geradores de build).
* **Fluxo de Trabalho**: Implementar código-fonte com precisão cirúrgica, realizar validações estáticas/estruturais, versionar com `git commit` e fazer `git push origin main`.

---

## 📍 Arquitetura Tecnológica do Drift
* **Frontend:** Qt6 / QML moderno e reativo (`src/qml/`).
* **Backend Core & Controladores:** C++20 (`src/models/`, `src/core/`, `src/engine/`).
* **Renderização Gráfica:** Google Skia + Shaders OpenGL/GLSL (`src/engine/GpuCompositor.cpp`, `transitions/`).
* **Motor de Áudio:** JUCE Audio & DSP nativo em C++ (`src/engine/audio/FilterProcessors.cpp`, `AudioEffectFactory.cpp`, `audio-effects/`).
* **IA & Redes Neurais Locais:** ONNX Runtime local (Whisper para transcrição, RVM e SAM2 para segmentação humana).

---

## ✅ O Que Foi Concluído com Sucesso

### 1. Atalhos Rápidos & Edição de Vídeo por Texto
* **Tecla `B`**: Corte instantâneo inteligente no playhead sem necessidade de dois cliques com o mouse (`AppController::splitAtPlayheadSmart`).
* **Teclas `Q` e `W`**: Ripple trim automático à esquerda/direita da agulha.
* **Edição por Transcrição (`SubtitleEditor.qml`)**: Visualização em modo texto de roteiro contínuo, tesoura para fatiar o vídeo deletando trechos de fala com ripple automático e botão de corte em massa de silêncios/pausas mortas (> 0.6s).

### 2. PARTE 1 DO PLANO CAPCUT PRO: Efeitos de Áudio Pro & IA (100% Concluída)
* **Voz de Estúdio (*Enhance Voice*)**: Cadeia nativa de 5 estágios (Noise Gate, 3-Band Broadcast EQ, De-Esser 6kHz, Optical Compressor, Commercial Leveler) + 3 presets de 1 clique (*Podcast Quente*, *Cristalina*, *Rádio FM*).
* **Auto-Ducking Inteligente**: Detecção de fala multicamada (Whisper SRT/legendas ou faixas de diálogo), curvas cúbicas Bézier (*Ease In/Out*), janela de retenção anti-pumping de 1.0s e sliders/presets de atenuação (-9 dB a -20 dB até -30 dB).
* **Áudio 8D / Binaural 360°**: Efeito imersivo tridimensional ao redor da cabeça com atraso interaural (*ITD*), rotação senoidal de pan estéreo e ambiência (*Lenta, Média, Rápida*).
* **Festa ao Lado (Vizinho / Parede)**: Som abafado através da parede com passa-baixas íngreme, ressonância de subwoofer e eco de cômodo/banheiro.
* **Isolador Vocal & Karaokê Music Splitter**: Processamento DSP Mid-Side (*M/S*) em tempo real, alternador de 1 clique (*Isolar Voz* vs *Remover Voz/Instrumental*) e botão de 1 clique para duplicar e fatiar em duas faixas separadas na timeline (`[Voz Isolada]` e `[Instrumental]`).
* **Efeitos Retrô Vintage**: *Voz de Telefone Vintage* (Chamada, Telefone Antigo, Walkie-Talkie) e *Rádio Lo-Fi & Vinil* (Lo-Fi Beats, Disco de Vinil, Fita Cassete).
* **Interface**: Cards dedicados e modernos integrados no Inspetor de Áudio do clipe (`AudioInspector.qml`).

### 3. PARTE 2: Transições de Alto Impacto & Shaders Virais (100% Concluída)
* **Smooth Zoom In (`transitions/smooth_zoom_in/`)**: Zoom contínuo com aceleração cúbica ($1.0\times \rightarrow 2.2\times$), micro-rotação dinâmica ($4^\circ$), radial motion blur multiamostrado com dither anti-banding e aberração cromática RGB split.
* **Smooth Zoom Out (`transitions/smooth_zoom_out/`)**: Recuo dinâmico centrípeto, contra-rotação, desfoque radial convergente e dispersão óptica.
* **Whip Pan Direcional (`transitions/whip_pan/`)**: Chicotada de câmera 4-way (Esquerda, Direita, Cima, Baixo) com curva de velocidade cúbica Hermite extrema, desfoque de movimento direcional 9-tap e separação cromática prismática.
* **Glitch Pro (`transitions/glitch_pro/`)**: Fatiamento digital em blocos 2D (*macro block tearing*), micro-scanlines com jitter, dispersão cromática RGB 2D e inversão estroboscópica de cores nos blocos críticos, integrado ao som nativo `glitch_rise`.
* **Film Roll & Burn (`transitions/film_roll_burn/`)**: Rolagem de película de 35mm com jitter analógico (*gate weave*), linha divisória de quadros, queima orgânica FBM e vazamento de luz quente (*warm light leak bloom*), com `soundFx: "whoosh_deep"`.
* **Lens Flare Flash (`transitions/lens_flare_flash/`)**: Clarão de alta energia com feixe anamórfico horizontal cinemático, estouramento de exposição, halo óptico circular e dispersão prismática, com `soundFx: "whoosh_fast"`.
* **Rasgo de Papel / Paper Rip (`transitions/paper_rip/`)**: Efeito colagem stop-motion com rasgo fractal procedural via FBM, borda de celulose branca exposta (*torn fiber*), sombra projetada (*drop shadow*) e sintetizador acústico procedural nativo `paper_rip` em C++ (JUCE/DSP).
* **Sound FX Whoosh, Glitch & Paper Rip Nativo Integrado**:
  - Propriedade nativa `soundFx` no manifesto `transition.json` (`whoosh_fast`, `whoosh_deep`, `glitch_rise`, `paper_rip`).
  - Suporte no backend C++ (`TransitionCatalog.h`, `TransitionPackageLoader.cpp`, `SfxCatalog.cpp`, `AppController::addTransitionWithSfx`, `AppController::attachTransitionSfx`).
  - Painel de propriedades (`TransitionInspector.qml`) com card dedicado para sincronização dinâmica do som sugerido com 1 clique na timeline, com pico acústico perfeitamente sincronizado ao corte.

---

### 4. PARTE 3: Animações e Motions Dinâmicos (Entrada, Saída, Combo & Speed Ramping) (100% Concluída)
* **Novos Motions de Entrada e Saída (In/Out)**:
  - `ElasticPop`: Pop-up com overshoot acentuado e amortecimento oscilatório realista ($1 + \sin(\dots) \cdot e^{-t}$).
  - `ZoomPunch`: Soco visual com contração e expansão explosiva para cortes de impacto.
* **Sistema de Motions Combo / Câmera Viva (Loop Contínuo de Clipe Inteiro)**:
  - `ClipAnimation animCombo`: Campo nativo adicionado ao `Clip` em C++, com serialização JSON e retrocompatibilidade em `Project.cpp`.
  - Avaliação multicamada sem alocação em `FrameCompositor.cpp` e `ClipAnimation.cpp` (camada In + camada Combo contínua + camada Out).
  - Presets de Câmera Viva:
    - `Pendulum`: Balanço senoidal suave em rotação ($\pm 2.5^\circ$) e oscilação horizontal ($x$).
    - `Shake`: Tremor cinemático multiharmônico de câmera na mão e impacto de graves.
    - `Pulse`: Batimento cardíaco / pulso ritmado de escala ($1.0 \rightarrow 1.05$).
    - `KenBurns`: Pan & zoom lento e elegante através de todo o clipe.
* **Curvas de Velocidade & Speed Ramping de 1 Clique**:
  - Implementação estática nativa em `SpeedCurve.h` / `SpeedCurve.cpp`:
    - `montage()`: Rampa rápida-lenta-rápida ($2.5\times \rightarrow 0.5\times \rightarrow 2.5\times$).
    - `hero()`: Câmera lenta cinemática centrada com curvas de entrada/saída suaves ($1.0\times \rightarrow 0.3\times \rightarrow 1.0\times$).
    - `bullet()`: Bullet-time com desaceleração extrema ($3.5\times \rightarrow 0.2\times \rightarrow 3.5\times$).
    - `flashInOut()`: Flash in / flash out veloz nas pontas ($4.0\times \rightarrow 1.0\times \rightarrow 4.0\times$).
  - Backend reativo: `AppController::applySpeedCurvePreset(trackIndex, clipIndex, presetId)`.
* **Interface QML Refinada**:
  - `AnimationInspector.qml`: Chips rápidos para `Elastic Pop` e `Zoom Punch`, seção dedicada para **Combo / Câmera Viva** com seletor de tipo e ajuste de período/duração.
  - `SpeedFadeInspector.qml`: Chips de 1 clique (*Montage*, *Hero*, *Bullet*, *Flash*) para aplicar curvas virais diretamente no painel de propriedades.
  - `SpeedCurveWindow.qml`: Botões de presets rápidos na barra de ferramentas da janela gráfica de curva de velocidade.

### 5. PARTE 4: Camadas, Overlays & Modos de Mesclagem Pro (Blending Modes & Efeitos Visuais) (100% Concluída)
* **Modos de Mesclagem Nativos (Skia 2D & GPU Shaders OpenGL)**:
  - Expansão do enum `BlendMode` e funções de string com os modos mais requisitados da indústria:
    - `Screen`: Elimina fundos pretos instantaneamente (fogo, faíscas, luz, fumaça, partículas).
    - `Multiply`: Elimina fundos brancos (texturas de papel, rascunhos, poeira escura).
    - `Overlay`: Contraste cinematográfico com preservação de tons médios.
    - `ColorDodge`: Brilho extremo de alta energia / reflexos cibernéticos e sci-fi.
    - `SoftLight`: Iluminação difusa elegante e suave.
    - `Difference`: Inversão criativa e psicodélica de cores.
  - Renderizador Skia (`SkiaShading.cpp`) atualizado para mapear todos os modos diretamente para `SkBlendMode`.
  - Shader OpenGL (`kBlendFragShader` em `GpuCompositor.cpp`) atualizado com as equações matemáticas precisas de cada modo de mesclagem.
  - Correção de persistência no `Project.cpp` (salvando e restaurando `blendMode` no JSON) e ponte reativa C++/QML (`clipToMap` exportando `blendMode`).
* **Interface de Mesclagem de Alta Produtividade (`BlendingInspector.qml`)**:
  - Chips rápidos de 1 clique (*Normal*, *Screen*, *Multiply*, *Overlay*, *Color Dodge*, *Soft Light*).
  - Seletor completo `ThemedComboBox` com 11 modos de mesclagem.
  - Card explicativo em tempo real detalhando o uso ideal de cada modo ativo.
  - Slider integrado de **Opacidade / Transparência (0% a 100%)** com suporte a keyframes e percentual visual direto no painel de mesclagem.
* **Recorte de Silhueta com Brilho Neon (Neon Glow Outline)**:
  - Método C++ `AppController::applyNeonGlowOutline(trackIndex, clipIndex, color)` integrando o pipeline neural SAM2/RVM ao shader procedural `edge_neon` e template `neon_cutout`.
  - Card interativo em `MasksInspector.qml` com paleta de 5 cores luminosas (*Ciano Cyberpunk*, *Magenta Neon*, *Dourado Solar*, *Verde Matrix*, *Roxo*) para aplicar o contorno brilhante com 1 clique.

### 6. PARTE 5: Legendas Dinâmicas & Estilizadas Virais (Auto-Captions Estilo Hormozi / MrBeast) (100% Concluída)
* **Motor de Emojis Automáticos Contextuais (`SubtitleCue.h` / `SubtitleCue.cpp`)**:
  - Dicionário bilíngue inteligente (Português + Inglês) para termos de alto impacto viral:
    - Dinheiro/Lucro/Vendas/Milhão $\rightarrow$ 💸
    - Fogo/Viral/Hype/Quente $\rightarrow$ 🔥
    - Ideia/Sacada/Segredo/Dica $\rightarrow$ 💡
    - Alvo/Meta/Foco/Objetivo $\rightarrow$ 🎯
    - Rápido/Velocidade/Agora/Urgente $\rightarrow$ ⚡
    - Foguete/Crescer/Escalar/Top $\rightarrow$ 🚀
    - Atenção/Cuidado/Pare/Perigo $\rightarrow$ ⚠️
    - Choque/Uau/Inacreditável/Loucura $\rightarrow$ 😱
    - Amor/Coração/Paixão $\rightarrow$ ❤️
    - Vitória/Troféu/Vencer/Campeão $\rightarrow$ 🏆
    - Olhar/Veja/Assista $\rightarrow$ 👀
    - Força/Poder/Treino/Academia $\rightarrow$ 💪
    - Mágica/Brilho/Estrela $\rightarrow$ ✨
    - Erro/Falha/Nunca/Perder $\rightarrow$ ❌
    - Certo/Verdade/Perfeito/Feito $\rightarrow$ ✅
    - Dúvida/Pergunta/Por que $\rightarrow$ ❓
    - Música/Som/Ritmo/Batida $\rightarrow$ 🎵
    - Risos/Engraçado/Piada $\rightarrow$ 😂
  - Funções `enrichSubtitleTextWithEmojis` e `enrichSubtitleCuesWithEmojis` que anexam emojis automaticamente sem duplicações.
* **Presets de Legendas Virais Calibrados (`TextStyle.cpp`)**:
  - `tiktok-viral-yellow`: Montserrat 900, AllCaps, contorno preto 5.5px, sombra 0 5 9, palavra ativa em amarelo elétrico `#FFE600` com escala pop de 1.22x.
  - `hormozi-beast`: Anton 96, AllCaps, contorno preto espesso 6.0px, sombra 0 7 12, palavra ativa em verde limão elétrico `#00FF66` (Alex Hormozi) com escala pop de 1.25x.
  - `mrbeast-pop`: Montserrat 92, AllCaps, contorno preto 5.5px, palavra ativa em ouro brilhante `#FFD700` com destaque pill e pop de 1.26x.
  - `tiktok-cyan-glow`: Montserrat 86, contorno preto 4.0px, glow ciano `#00F2FE`, palavra ativa em ciano com escala 1.22x.
  - `danger-red`: Anton 94, AllCaps, contorno preto 5.5px, sombra dramática, palavra ativa em vermelho fogo `#FF2A2A` com escala 1.24x.
  - `reels-pill-box`: Inter 800, contorno sutil, pílula animada vermelha `#FF3B30` ou verde destacando a palavra falada.
  - `shorts-single-word`: Montserrat 98, AllCaps, retenção máxima de 1 palavra por tela com escala 1.20x.
* **Backend de Alta Produtividade (`AppController.h` / `AppController.cpp`)**:
  - `generateSubtitlesForClip`: suporte a flag `bool addEmojis` integrada ao Whisper local e finalização do clipe.
  - `applyViralCaptionsStyle`: aplica empacotamento de palavras por tela, emojis contextuais, caixa alta e preset viral em 1 clique.
  - `autoEnrichSubtitlesWithEmojis`: enriquece qualquer clipe de legenda existente na timeline com emojis contextuais e suporte a Undo/Redo.
  - `repackSubtitleCues`: reempacota as legendas existentes em 1 palavra por tela (estilo Hormozi) ou 2-3 palavras (estilo Shorts/Reels).
  - Modernização de `setSubtitleClipVisuals` com APIs nativas de `TextStyle` (camadas, stroke, shadow, pixelSize, primaryColor).
* **Interface QML Atualizada (`SubtitlesTab.qml` & `SubtitleEditor.qml`)**:
  - `SubtitlesTab.qml`: Checkbox nativo `ThemedCheckBox` para ativação de emojis automáticos e galeria completa de estilos virais.
  - `SubtitleEditor.qml`: Barra superior de ações virais rápidas (*✨ Emojis Automáticos*, *⚡ 1 Palavra/Tela*, *🔥 2-3 Palavras*) e seletor em fluxo (Flow) com todos os presets estilizados.

---

## 🗺️ O Roteiro Completo dos Próximos Passos (`PROXIMOS_PASSOS.md`)

O documento [`PROXIMOS_PASSOS.md`](file:///C:/Users/Alex/Documents/Antigravity/drift/PROXIMOS_PASSOS.md) detalha todo o ecossistema planejado:

### O Próximo Foco Imediato:
* **PARTE 6: Recorte Inteligente de Fundo (Smart Cutout / Auto Cutout em 1-Clique)**
  - O Drift já possui o motor C++ com `RvmMatter` e `Sam2Segmenter` usando ONNX Runtime local.
  - Adicionar botão de 1 clique no painel de vídeo e timeline *"Remover Fundo (Auto Cutout)"*.
  - Permitir o fluxo viral de duplicar a faixa e colocar **textos grandes flutuando atrás da pessoa**.

### Frentes Complementares Mapeadas:
* **Parte 7**: Detecção de Batidas e Cortes no Ritmo (*Auto-Beats / Snap to Beat* na timeline via `AudioOnsets.cpp`).
* **Parte 8**: Rastreamento de Movimento (*Motion Tracking* via OpenCV).
* **Parte 9**: Presets de Redes Sociais & Guias de Zonas Seguras 9:16 (TikTok/Reels/Shorts).

---

## 📂 Arquivos Chave Recentes no Repositório
* `src/core/SubtitleCue.h` / `SubtitleCue.cpp`: Motor de quebra de legendas, Karaoke timings e enriquecimento com emojis contextuais.
* `src/core/TextStyle.h` / `TextStyle.cpp`: Presets virais com WordAccent Karaoke e escalas pop dinâmicas.
* `src/models/AppController.h` / `AppController.cpp`: Métodos de legendas virais, transcrição Whisper e empacotamento.
* `src/qml/components/assets/SubtitlesTab.qml`: Painel de geração automática de legendas com Whisper.
* `src/qml/components/SubtitleEditor.qml`: Editor ao vivo de legendas estilo lyrics com ações virais e presets rápidos.
* `PROXIMOS_PASSOS.md`: Roteiro estratégico detalhado de todas as 9 partes.
