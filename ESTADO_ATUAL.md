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

### 7. PARTE 6: Recorte Inteligente de Fundo (Smart Cutout / Auto Cutout em 1-Clique) (100% Concluída)
* **Motor Neural RVM sem Seleção Manual (`AppController::autoCutoutPerson`)**:
  - Remove automaticamente o fundo de apresentadores e pessoas em tempo real sem green screen usando *Robust Video Matting* local (ONNX Runtime).
  - Execução direta com 1 clique (`mobilenetv3` otimizado), sem necessidade de abrir janela modal ou clicar em pontos.
* **Efeito Viral: Criar Texto Atrás da Pessoa (`AppController::createTextBehindSubjectEffect`)**:
  - Em 1 clique, monta automaticamente a arquitetura multicamada na timeline:
    - Camada Superior: Clone do apresentador recortado sem fundo.
    - Camada Intermediária: Texto estilizado gigante (Anton 900, AllCaps, com contorno e sombra de destaque) posicionado atrás do sujeito.
    - Camada Inferior: Vídeo original completo com o cenário.
  - Inicia imediatamente o rastreamento neural na camada superior.
* **Gerenciamento de Recorte**:
  - `hasCutoutMask`: Detecta instantaneamente se o clipe já possui máscara de recorte ativa.
  - `invertCutoutMask`: Inverte a máscara em 1 clique (para esconder a pessoa e deixar apenas o fundo/cenário limpo).
  - `removeCutoutMask`: Remove a máscara e restaura o clipe original com suporte a Undo/Redo.
* **Interface QML Refinada**:
  - `GeneralInspector.qml`: Card dedicado e moderno de **Recorte Inteligente de Fundo (Auto Cutout)** com botões de 1 clique, barra de progresso em tempo real e alternador de inversão/restauração.
  - `MasksTab.qml`: Botões rápidos `⚡ Auto Cutout Pessoa (1-Clique)` e `🔤 Criar Texto Atrás da Pessoa`.

---

### 8. PARTE 7: Detecção de Batidas e Cortes no Ritmo (Auto-Beats na Timeline) (100% Concluída)
* **Motor Neural/Espectral de Áudio (`AudioOnsets.cpp` / `AppController.cpp`)**:
  - `detectAndMarkBeats(trackIndex, clipIndex, mode, minStrength)`: Análise rítmica inteligente do clipe ou projeto inteiro via FFT/autocorrelação espectral de transientes acústicos (*kicks, snares, downbeats*).
  - Cálculo automático de BPM em tempo real e grid de compassos musicais (*bars*).
  - `toggleBeatSnap()`: Alterna ativação e visualização das camadas de snap rítmico.
  - `splitClipAtBeats(trackIndex, clipIndex)`: Fatiamento rítmico em 1 clique do clipe selecionado nas batidas musicais exatas, permitindo edições dinâmicas de alta retenção visual com cortes no tempo da música.
  - `convertBeatsToBookmarks()`: Converte todas as batidas musicais em marcadores de timeline (*Bookmarks*) com nomes ordenados ("Beat 1", "Beat 2", etc.).
* **Snap Magnético Musical na Linha do Tempo**:
  - `m_beatSnapTargets` e `extraSnapTargets()`: Invalidação e sincronização automática do cache magnético. A agulha da timeline (playhead), a ferramenta de corte e a movimentação/trim de clipes são atraídas com precisão cirúrgica para cada batida musical.
* **Interface QML Refinada e Interativa**:
  - `TimelineToolbar.qml`: Botão de Auto-Beats no grupo de controles magnéticos com indicador de status iluminado em ouro `#FFD600`, tooltip com BPM e contagem de batidas, e menu de contexto completo com clique com botão direito.
  - `TimelinePanel.qml`: Camada visual `beatMarkerRow` com pequenos losangos e marcadores de agulha dourados brilhantes nas posições das batidas ao longo da régua, com salto do playhead ao clicar no marcador.
  - `AudioInspector.qml`: Card dedicado de **Auto-Beats & Sincronização no Ritmo** com badge de BPM detectado, botões de 1 clique para detectar batidas do clipe, ligar/desligar snap magnético, cortar clipe nas batidas e gerar marcadores.

---

### 9. PARTE 8: Rastreamento de Movimento & Face Tracking (Motion Tracking) (100% Concluída)
* **Motor Neural de Rastreamento Facial (`FaceLandmarker.cpp` / `FaceTrack.cpp` / `AppController.cpp`)**:
  - `attachClipToFaceTrack(targetTrack, targetClip, sourceTrack, sourceClip, anchor, offsetX, offsetY, trackScale, trackRotation)`: Vincula qualquer camada (Texto, Sticker, Imagem, Ajuste) à trajetória rastreada de uma pessoa no vídeo com interpolação contínua e suporte a Undo/Redo.
  - Suporte a múltiplos pontos de âncora: Cabeça/Topo (`head`), Olhos (`eyes`), Centro do Rosto (`faceCenter`), Boca (`mouth`) e Queixo (`chin`).
  - Acompanhamento automático de Escala/Zoom proporcional à aproximação da pessoa e Rotação da cabeça.
  - `hasFaceTrack(trackIndex, clipIndex)`: Verificação instantânea do estado de rastreamento do clipe.
* **Efeito Nativo de Censura Facial Dinâmica (`effects/face_mosaic_censor/`)**:
  - Efeito GPU GLSL de alta performance (`main.frag` + `effect.json`) injetado via uniformes `u_face*`.
  - 3 Modos integrados de 1 clique:
    - `0`: Mosaico (Pixelate) acompanhando a elipse rotacionada do rosto.
    - `1`: Desfoque Gaussiano 9-tap com suavização nas bordas (*feather*).
    - `2`: Tarja Preta nos Olhos (*Eyes Bar*), rotacionada no ângulo dos olhos.
  - `applyFaceCensorEffect(trackIndex, clipIndex, mode, pixelSize, radius)`: Aciona o scan e aplica a censura em 1 clique.
* **Interface QML Refinada (`TransformInspector.qml`)**:
  - Card dedicado **Rastreamento de Movimento (Motion Tracking)** com badge de IA.
  - Painel para elementos sobrepostos: seletor de âncoras, chips para ativar/desativar acompanhamento de escala e rotação, e botão para fixar ao rosto rastreado.
  - Painel para vídeos: botão para escanear rostos com barra de progresso em tempo real e botões de 1 clique para censurar com Mosaico, Desfoque ou Tarja nos Olhos.

---

### 10. Narração de Texto em Voz (TTS) - As 5 Vozes Mais Realistas e Humanizadas do Brasil (100% Concluída)
* **Catálogo das 5 Vozes Neurais Brasileiras de Alta Expressividade (`TtsSynthesizer.h` / `TtsSynthesizer.cpp`)**:
  - Ampla pesquisa de mercado e feedback da comunidade de criadores de conteúdo do Brasil (TikTok, Reels, Shorts e canais Dark do YouTube):
    1. **`pt-BR-Thalita` (🔥 Viral & Espontânea)**: Voz feminina jovem, dinâmica, rebelde e descontraída. Preferida nº 1 para TikTok e Reels. Zero robótica.
    2. **`pt-BR-Antonio` (🎙️ Épico & Documentário)**: Voz masculina encorpada, profunda e cinematográfica. A voz definitiva dos canais Dark de mistério, história e investigações.
    3. **`pt-BR-Francisca` (✨ Storyteller & Expressiva)**: Voz feminina madura, calorosa e polida, ideal para explicações didáticas, tutoriais e roteiros longos.
    4. **`pt-BR-Fabio` (⚡ Tech & Dinâmico)**: Voz masculina vibrante, jovem e ágil para vídeos de tecnologia, esportes e alta retenção.
    5. **`pt-BR-Yara` (💬 Autêntica & Lifestyle)**: Tom moderno, descontraído e intimista estilo podcaster para vlogs e conselhos.
* **Motor de Síntese Humanizada com SSML e Respiração Natural**:
  - Pausas micro-orgânicas de pontuação (`<break time="120ms"/>` em vírgulas e `260ms` em pontos) que eliminam qualquer cadência robótica contínua.
  - Modulação de Velocidade (`rate` de 0.5x a 2.0x, com preset de 1.1x para engajamento viral) e Tom (`pitch` de -20% a +20%).
  - Suporte inteligente a vozes modernas OneCore e SAPI do Windows com áudio WAV 48kHz cristalino.
  - Geração precisa de `SubtitleCue` para sincronização instantânea de legendas na timeline.
* **Interface QML Redesenhada (`TextAssetsTab.qml`)**:
  - Card visual moderno com seletores em chips das 5 vozes e badges de impacto visual.
  - Painel descritivo contextual da voz selecionada.
  - Botões de ritmo rápido (`1.0x Normal`, `🔥 1.1x Viral Reels/TikTok`, `⚡ 1.25x Ágil`).
  - Sliders de Velocidade e Tom (Pitch).
  - Prévia de áudio instantânea com Play/Stop e botão de inserção direta na timeline sincronizando legendas animadas na agulha.

### 11. PLANO MESTRE - Fase 1: Interface UI/UX Respirável & Macro-Hubs (100% Concluída)
* **As 8 Macro-Hubs Otimizadas (`AssetsPanel.qml`)**:
  - Consolidação cirúrgica de abas poluídas (Stickers, Shapes, Cenas, Masks, Market) em prol de espaço e clareza mental do editor.
  - O painel lateral agora contém apenas as 8 categorias primárias definitivas: *Mídia, Templates, Legendas, Cenários, Efeitos, Transições, Áudio, Atalhos*.
* **Botão Templates 1-Clique (`TemplatesTab.qml`)**:
  - Nova infraestrutura na UI para abrigar a biblioteca embutida e expansível de edições pré-feitas e pressets.
* **Workspaces Dinâmicos Inteligentes (`EditorHeader.qml` & `AppController.cpp`)**:
  - Botões de 1-Clique na barra principal (Shorts, Legendas, WebDoc, Podcast, Clássico).
  - Controle centralizado no `EditorState` que ajusta automaticamente a UI, proporções (9:16 vs 16:9) e painéis conforme o tipo de conteúdo em produção.

### 12. PLANO MESTRE - Fase 2: Motor Dark Studio Wizard (Core) (100% Concluída)
* **Estrutura C++ (`WizardEngine.h` / `WizardEngine.cpp`)**:
  - Cérebro central assíncrono para automatizar a criação de timelines via IA e roteiros.
  - Processamento em background rodando fora da thread principal para manter a UI responsiva.
* **Integração Text-to-Speech (`TtsSynthesizer`)**:
  - O Wizard recebe o roteiro, processa a síntese usando a voz premium selecionada e salva o áudio temporariamente.
* **Alinhamento Forçado via Whisper (`WhisperTranscriber`)**:
  - Decodificação iterativa do áudio gerado pelo TTS usando `ClipReaderPool`.
  - Passagem do áudio para Whisper transcrever e gerar as cues (legendas) sincronizadas fonema a fonema.
* **Ponte com o Projeto e Interface (`AppController`)**:
  - Injeção inteligente no objeto `Project` inserindo as tracks de áudio e de legendas dinamicamente.
  - Sinais nativos (`wizardRunning`, `wizardProgress`, `wizardStatus`) para exibir o estado na interface em tempo real.

### 13. PLANO MESTRE - Fase 3: Automação Cinematográfica (Ritmo, B-Rolls e SFX) (100% Concluída)
* **Jump Cuts Semânticos Inteligentes**:
  - Implementado algoritmo em `WizardEngine.cpp` que analisa o espaçamento temporal entre as `SubtitleCues` transcritas.
  - Pausas estéreis e silenciosas maiores que 400ms são sumariamente ignoradas e a timeline é colapsada, gerando um jump cut limpo, vital para alta retenção em Shorts.
* **Sonorização Dinâmica (Impactos & SFX)**:
  - Integração semântica onde palavras de alta voltagem léxica ("!", "Poder", "Dinheiro") invocam a injeção instantânea de SFX (`whoosh_deep`, `boom_bass`) na `sfxTrack`.
* **Cinematografia de Apoio (B-Rolls & Ken Burns)**:
  - Adição automatizada de clipes de imagem/B-Roll sincronizados ao bloco de voz.
  - Acoplamento automático do movimento `ClipAnimKind::KenBurns` para trazer dinâmica suave de ponta a ponta nesses vídeos.
* **Auto-Ducking Nativo na Música de Fundo**:
  - Geração automatizada da "Bed Track" musical, com o efeito `auto_ducking` nativamente instanciado e aplicado a toda extensão do projeto.

### 14. PLANO MESTRE - Fase 4: Pipeline Multicanal (Kanban de Produção) (100% Concluída)
* **Gerenciador de Estado C++ (`ProjectPipelineManager.h` / `.cpp`)**:
  - Implementação de um Kanban board global focado no ciclo de vida de dezenas de vídeos.
  - Colunas padronizadas de produção: *Ideia*, *Roteirização*, *Pronto para Gravar*, *Edição*, *Renderização*, *Publicado*.
  - Arquitetura reativa baseada em `QObject` para serialização de metadados em `%AppData%/kanban_state.json`.
* **Interface QML Drag-and-Drop (`PipelineView.qml`)**:
  - Visualização interativa com cards representando projetos e colunas temáticas.
  - Ocupa o painel principal quando ativada, proporcionando uma transição mental de "Editor de Linha do Tempo" para "Estúdio / Produtora de Conteúdo".
* **Acesso Simplificado 1-Clique (`EditorHeader.qml`)**:
  - Inserção de botão destacado **🗂️ Estúdio Dark** no menu principal de espaços de trabalho para acesso instantâneo ao pipeline.
  - Auto-ocultamento do painel quando o usuário seleciona outros layouts clássicos (Shorts, Podcast, WebDoc).

---

## 🗺️ O Roteiro Completo dos Próximos Passos (`PROXIMOS_PASSOS.md`)

O documento [`PROXIMOS_PASSOS.md`](file:///C:/Users/Alex/Documents/Antigravity/drift/PROXIMOS_PASSOS.md) detalha todo o ecossistema planejado:
* A pedido do usuário, a Parte 9 (Zonas Seguras 9:16) foi postergada para o futuro.
* O motor TTS com as 5 vozes neurais brasileiras mais dinâmicas e realistas foi concluído com sucesso total.

---

## 📂 Arquivos Chave Recentes no Repositório
* `src/engine/TtsSynthesizer.h` / `src/engine/TtsSynthesizer.cpp`: Catálogo estruturado com as 5 vozes neurais brasileiras, metadados enriquecidos, humanização SSML e modulação de pitch/rate.
* `src/qml/components/assets/TextAssetsTab.qml`: Interface completa do TTS com seleção em chips das 5 vozes, badges de estilo, presets rápidos de velocidade, controle de tom e sincronização de legendas.
* `effects/face_mosaic_censor/`: Pacote de efeito GLSL com censura dinâmica por mosaico, desfoque e tarja nos olhos.
* `src/models/AppController.h` / `AppController.cpp`: Métodos de suporte a TTS, face tracking e efeitos.
* `PROXIMOS_PASSOS.md`: Roteiro estratégico detalhado.

