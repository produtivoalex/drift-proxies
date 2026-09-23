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

## 🗺️ O Roteiro Completo dos Próximos Passos (`PROXIMOS_PASSOS.md`)

O documento [`PROXIMOS_PASSOS.md`](file:///C:/Users/Alex/Documents/Antigravity/drift/PROXIMOS_PASSOS.md) detalha todo o ecossistema planejado:

### As 3 Partes Centrais Restantes:
1. **PARTE 2: Transições de Alto Impacto (Virais & Cinematográficas com Sound FX Integrado)**
   - *Smooth Zoom In / Out* com rotação e motion blur.
   - *Whip Pan Direcional* (chicotada de câmera horizontal e vertical com arrasto de velocidade).
   - *Film Roll & Burn* (queima de película analógica com vazamentos de luz).
   - *Glitch Pro* (aberração cromática RGB split e fatiamento horizontal de blocos).
   - *Paper Rip* (rasgo de papel stop-motion).
   - 🔥 *Transições com Áudio Sound FX Whoosh embutido e sincronizado na timeline!*
2. **PARTE 3: Motions e Animações Dinâmicas (Entrada, Saída, Combo & Speed Ramping)**
   - Animações In/Out: *Pop-Up Bounce Elástico*, *Slide com Inércia*, *Zoom Punch*.
   - Animações Combo / Câmera Viva: *Efeito Pêndulo*, *Shake de Batida/Terremoto*, *Rotação 3D*.
   - Curvas de Velocidade (*Speed Ramping*): Presets *Montage*, *Hero*, *Bullet*, *Flash In/Out*.
3. **PARTE 4: Camadas, Overlays & Modos de Mesclagem Pro (Blending Modes & Efeitos Visuais)**
   - Modos de Mesclagem Nativos Skia: *Screen* (remove fundo preto de partículas e fogo), *Multiply* (remove fundo branco), *Overlay*, *Color Dodge*.
   - Recorte com Brilho Neon (*Glow Outline / Neon Edge*) contornando a pessoa no vídeo via SAM2/RVM.

### Frentes Complementares Mapeadas:
* **Parte 5**: Legendas Dinâmicas Estilizadas Virais (Estilo Hormozi/MrBeast palavra por palavra).
* **Parte 6**: Recorte Inteligente de Fundo em 1-Clique (*Auto Cutout* de pessoa sem tela verde).
* **Parte 7**: Detecção de Batidas e Cortes no Ritmo (*Auto-Beats / Snap to Beat* na timeline).
* **Parte 8**: Rastreamento de Movimento (*Motion Tracking* via OpenCV).
* **Parte 9**: Presets de Redes Sociais & Guias de Zonas Seguras 9:16 (TikTok/Reels/Shorts).

---

## 📂 Arquivos Chave Recentes no Repositório
* `PROXIMOS_PASSOS.md`: Roteiro estratégico detalhado de todas as 9 partes.
* `src/qml/components/properties/AudioInspector.qml`: Controles de UI para todos os efeitos de áudio.
* `src/models/AppController.h` / `src/models/AppController.cpp`: Métodos C++ para orquestração de efeitos, ducking, isolamento vocal e cortes.
* `src/engine/audio/FilterProcessors.h` / `FilterProcessors.cpp`: Implementação dos DSPs (ex: `VocalIsolatorProcessor`).
* `src/engine/audio/AudioEffectFactory.cpp`: Registro de todos os efeitos de áudio.
* `audio-effects/`: Pacotes de manifesto JSON dos efeitos (`utility_studio_voice`, `space_eightd`, `transmission_party_next_door`, `utility_vocal_isolation`).
