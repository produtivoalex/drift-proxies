# Drift Video Editor - Diretrizes & Superpoderes do Projeto para Agentes IA
> **Arquivo de Instruções Automáticas do Workspace (Carregado Automaticamente pelo Antigravity)**  
> **Repositório:** `https://github.com/produtivoalex/drift-proxies.git` (branch `main`)  
> **Diretório Oficial:** `C:\Users\Alex\Documents\Antigravity\drift`

---

## 🚨 REGRAS CRÍTICAS E INVIOLÁVEIS DO PROJETO

1. **NÃO COMPILE NADA AINDA (Proibido Rodar Builds)**:
   - **NUNCA** execute comandos de compilação como `cmake --build`, `ninja`, `msbuild` ou scripts de build.
   - O usuário determinou que todo o trabalho atual deve ser focado na engenharia de código, estruturação, shaders, DSPs e interfaces QML.
   - Nenhuma compilação deve ser disparada sem que o usuário peça explicitamente com palavras como *"pode compilar"*.

2. **Fluxo de Versionamento e Sincronização Obrigatória**:
   - Todo arquivo implementado ou alterado deve ser verificado com `git status` e `git diff`.
   - Faça `git add <arquivos>` e `git commit -m "feat/fix/docs: ..."` com mensagens claras em inglês ou português.
   - Sempre envie com `git push origin main`.

3. **Formatação de Encerramento de Mensagem**:
   - **TODA E QUALQUER RESPOSTA** deve terminar com:
     `💡 **Sugestão de Próxima Implementação:** ...`  
     `**Dificuldade:** ...`

---

## ⚡ OS SUPERPODERES NATIVOS DO DRIFT (NÃO REINVENTE A RODA!)

O Drift **NÃO é um editor básico** que precisa de dependências externas improvisadas. Ele possui um motor C++20 de classe mundial já integrado no diretório `src/engine/`. Antes de tentar criar algo do zero, consulte esta lista de recursos prontos para uso:

### 1. IA & Redes Neurais Locais (ONNX Runtime em `src/engine/`)
* **`RvmMatter.cpp` / `.h`**: *Robust Video Matting* nativo. Recorta pessoas e remove fundos de vídeo em tempo real sem chroma key.
* **`Sam2Segmenter.cpp` / `.h`**: *Segment Anything Model 2 (Meta)*. Segmentação cirúrgica de qualquer objeto ou pessoa na cena.
* **`WhisperTranscriber.cpp` / `WhisperTokenizer.cpp`**: Transcrição local ultrarrápida de áudio/voz para legendas e decupagem.
* **`DeepFilterDenoiser.cpp`**: Redutor de ruído de áudio neural de estúdio baseado em DeepFilterNet.
* **`FaceLandmarker.cpp`, `FaceMesh.cpp`, `FaceTrack.cpp`, `ObjectDetector.cpp`**: Rastreamento facial (68/468 pontos) e tracking de objetos em tempo real.
* **`TtsSynthesizer.cpp`**: Motor de síntese de voz (Texto para Fala local).

### 2. Motor Gráfico & Shaders (Skia 2D/3D + OpenGL em `src/engine/`)
* **`GpuCompositor.cpp` & `SkiaShading.cpp`**: Renderizador central com aceleração por GPU.
* **Modos de Mesclagem (`SkBlendMode`)**: O `Clip.h` já possui `enum class BlendMode` suportando *Screen*, *Multiply*, *Overlay*, *Color Dodge*, etc.
* **`SkiaTextEffects.cpp`, `SkiaTextPainter.cpp`, `TextLayout.cpp`**: Sistema tipográfico avançado com suporte a sombras, bordas, gradientes e animações de texto palavra por palavra.
* **`TransitionCatalog.cpp` & `TransitionPackageLoader.cpp`**: Gerenciador de transições GLSL com pacotes em `transitions/`.

### 3. Áudio Profissional & DSP (JUCE Nativo em `src/engine/audio/`)
* **`FilterProcessors.h` / `.cpp`**: Filtros de alta performance, incluindo `VocalIsolatorProcessor` (Mid-Side vocal splitter), `NoiseGate`, `Compressor`, `StereoWiden`, `AutoPan`, `Echo`, `BandFilter`.
* **`AudioEffectFactory.cpp`**: Registro de efeitos de áudio em tempo real com manifestos JSON em `audio-effects/`.
* **`AudioOnsets.cpp` / `.h`**: Detector de transientes e batidas musicais (*kicks, snares*) para snap rítmico automático na timeline.
* **`LoudnessMeter.cpp`**: Medição de volume comercial em LUFS e True Peak.
* **`SfxCatalog.cpp`**: Catálogo nativo de efeitos sonoros (*Whoosh*, cliques, transições).

### 4. Matemática de Animações & Curvas de Velocidade
* **`src/core/SpeedCurve.h`**: Motor completo de curvas Bézier cúbicas para aceleração, desaceleração e câmera lenta fluida (*Speed Ramping*).
* **Keyframes Animáveis**: Todas as propriedades de transform (Posição X/Y, Escala, Rotação, Opacidade, Volume) possuem trilhos de interpolação Bézier prontos.

### 5. Frontend Reativo Qt6 / QML (`src/qml/`)
* **`src/models/AppController.h` / `.cpp`**: A ponte de controle principal entre a UI QML e o motor C++. É aqui que expomos novas propriedades e métodos Q_INVOKABLE.
* **`src/qml/components/properties/`**: Painéis de propriedades (`AudioInspector.qml`, `VideoInspector.qml`, `TransformInspector.qml`).
* **`src/qml/components/timeline/`**: Linha do tempo multifaixa com agulha (playhead), suporte a atalhos rápidos (`B`, `Q`, `W`).

---

## 🧭 ONDE ESTAMOS E PARA ONDE VAMOS

* **Arquivo de Estado Atual**: Leia [`ESTADO_ATUAL.md`](file:///C:/Users/Alex/Documents/Antigravity/drift/ESTADO_ATUAL.md) para ver o que acabou de ser concluído (Parte 1: Áudio Pro & IA com 6 grandes recursos concluídos).
* **Arquivo de Próximos Passos**: Leia [`PROXIMOS_PASSOS.md`](file:///C:/Users/Alex/Documents/Antigravity/drift/PROXIMOS_PASSOS.md) para ver as 3 partes centrais restantes (Parte 2: Transições de Alto Impacto, Parte 3: Motions e Speed Ramping, Parte 4: Modos de Mesclagem) e as 5 complementares.
