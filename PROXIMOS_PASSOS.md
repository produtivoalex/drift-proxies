# Drift Video Editor - Roteiro de Evolução & Próximos Passos
> **Roteiro Estratégico para Superar o CapCut Pro e Consolidar o Drift como o Editor de Escolha dos Criadores**  
> *100% Offline • Sem Assinaturas • Privacidade Total • Alto Desempenho em C++/Skia/JUCE*

Este documento consolida o status atual do projeto, detalha as **3 partes centrais restantes** do plano mestre inicial e lista as **frentes complementares** que formam o ecossistema definitivo de edição moderna para criadores de conteúdo (TikTok, Reels, YouTube e Cinema).

---

## 🏆 Status Atual do Projeto

### ✅ PARTE 1: Efeitos de Áudio Pro & IA (100% CONCLUÍDA)
Toda a suíte de áudio profissional foi desenvolvida em C++ nativo (JUCE/DSP), consumindo menos de 2% de CPU e sem necessidade de conexão com a nuvem:

1. **🎙️ Voz de Estúdio com IA (*Enhance Voice*)**:
   - Cadeia nativa de transmissão em 5 estágios: *Noise Gate*, *Broadcast 3-Band EQ*, *De-Esser* (6 kHz), *Compressor Óptico* e *Leveler de Volume Comercial*.
   - 3 Presets de 1 clique: *Podcast Quente*, *Cristalina* e *Rádio FM*.
2. **⚡ Auto-Ducking Inteligente**:
   - Detecção de fala multicamada (via legendas Whisper com precisão de ms ou faixas de diálogo).
   - Atenuação por curvas cúbicas suaves (*Ease In / Ease Out*), janela de retenção anti-pumping (1.0s) e presets de atenuação (-9 dB a -20 dB ou customizável até -30 dB).
3. **🎧 Áudio 8D (Binaural 360°)**:
   - Rotação espacial tridimensional ao redor da cabeça com atraso interaural (*ITD*), modulação de pan e ambiência de sala (*Velocidades: Lenta, Média, Rápida*).
4. **🚪 Efeito Festa ao Lado (Vizinho / Parede)**:
   - Filtro passa-baixas acentuado com ganho de graves de subwoofer e reflexão de ambiente (*Presets: Quarto ao Lado, No Banheiro da Balada, Vizinho de Cima*).
5. **✂️ Isolador Vocal & Karaokê Music Splitter**:
   - DSP Mid-Side (*M/S*) em tempo real com cancelamento de fase vocal e preservação de bumbo/subgrave.
   - Alternador de 1 clique (*Isolar Voz* vs *Remover Voz/Instrumental*) e botão para dividir o clipe em 2 faixas independentes na timeline.
6. **📻 Retrô & Texturas Vintage**:
   - *Voz de Telefone Vintage* (Chamada, Telefone Antigo de Carbono, Interfone Walkie-Talkie).
   - *Rádio Lo-Fi & Vinil* (Lo-Fi Beats aveludado, Vinil com flutter orgânico, Fita Cassete analógica).

---

## 🚀 As 3 Partes Centrais Restantes

---

### 🔄 PARTE 2: Transições de Alto Impacto (Virais & Cinematográficas)
*O Drift já possui 33 transições de corte e wipe básicos, mas precisa do conjunto dinâmico que define os vídeos virais modernos.*

* **2.1 Transições com Desfoque de Movimento (*Motion Blur Zooms*)**:
  - **Smooth Zoom In / Out**: Zoom rápido cinematográfico com rotação sutil e desfoque direcional nas bordas.
  - **Whip Pan Direcional**: Chicotada de câmera horizontal e vertical com arrasto natural de velocidade.
* **2.2 Transições de Luz e Película (*Light & Film Burn*)**:
  - **Film Roll & Burn**: Queima de película analógica vintage com vazamento de luz quente (*light leaks*).
  - **Lens Flare Flash**: Clarão óptico com brilho (*glow*) suave para transições enérgicas.
* **2.3 Transições de Textura e Estilo (*Paper Tear & Glitch Pro*)**:
  - **Rasgo de Papel (*Paper Rip*)**: Transição orgânica estilo colagem/stop-motion revelando a cena seguinte.
  - **Glitch Pro com Aberração Cromática**: Distorção RGB split digital com fatiamento horizontal de blocos de pixel.
* **🔥 Diferencial Exclusivo: Transições com Sound FX Whoosh Embutido**:
  - No CapCut, o criador precisa pesquisar e posicionar o efeito sonoro de "whoosh" manualmente. No Drift, a transição já pode carregar seu efeito sonoro nativo perfeitamente sincronizado ao ser aplicada no corte.
* **Dificuldade:** Média.

---

### 🎬 PARTE 3: Animações e Motions Dinâmicos (Entrada, Saída, Combo & Speed Ramping)
*A capacidade de dar vida a textos, títulos, B-rolls e clipes principais com presets prontos de movimento orgânico.*

* **3.1 Animações de Entrada (*In*) e Saída (*Out*)**:
  - **Pop-Up Bounce Elástico**: Entrada saltitante com amortecimento físico suave.
  - **Slide com Inércia (*Ease In-Out*)**: Deslizamento lateral, superior ou inferior com desaceleração realista.
  - **Zoom Punch & Fade Dinâmico**: Impacto visual instantâneo para cortes de ênfase.
* **3.2 Animações Combo / Câmera Viva (*Loop & Movement*)**:
  - **Efeito Pêndulo**: O elemento balança organicamente como se estivesse suspenso por um fio.
  - **Shake de Batida / Terremoto**: Tremor de câmera dinâmico sincronizado com momentos de clímax ou batidas da música.
  - **Rotação 3D com Perspectiva**: Giro em torno dos eixos Y e X com profundidade Skia 3D.
* **3.3 Curvas de Velocidade Profissionais (*Speed Ramping*)**:
  - O motor do Drift já possui suporte a curvas Bézier em `SpeedCurve.h`.
  - Presets virais de 1 clique:
    - **Montage**: Acelera no início ($3\times$), desacelera em câmera lenta cinematográfica no impacto ($0.4\times$) e retoma em velocidade normal.
    - **Hero**: Entrada rápida $\rightarrow$ Slow-mo estendido $\rightarrow$ Normal.
    - **Bullet**: Ritmo normal $\rightarrow$ Quase congelamento $\rightarrow$ Saída rápida.
    - **Flash In / Out**: Chicote de aceleração nas pontas do take.
* **Dificuldade:** Baixa a Média.

---

### 🎨 PARTE 4: Camadas, Overlays & Modos de Mesclagem Pro (Blending Modes & Efeitos Visuais)
*Transforma o Drift em uma ilha de composição visual rica, permitindo usar texturas, vazamentos de luz e efeitos visuais sem atrito.*

* **4.1 Modos de Mesclagem Nativos (*Blending Modes*) no Skia/OpenGL**:
  - Exposição de `clip.blendMode` no Inspetor de Propriedades com os modos essenciais do mercado:
    - **Screen (Tela)**: Oculta o fundo preto instantaneamente (ideal para fogo, faíscas, raios solares, poeira e fumaça).
    - **Multiply (Multiplicar)**: Oculta fundos brancos (ideal para papéis envelhecidos, desenhos e texturas escuras).
    - **Overlay (Sobrepor) / Soft Light**: Adiciona contraste e saturação mantendo tons médios.
    - **Color Dodge (Subexposição de Cores)**: Gera brilhos intensos de alta energia (*cyberpunk / sci-fi*).
* **4.2 Recorte de Silhueta com Brilho Neon (*Glow Outline / Neon Edge*)**:
  - Utiliza o pipeline neural de segmentação humana (SAM2/RVM) para traçar uma linha luminosa pulsante ou neon contornando o corpo do apresentador.
* **Dificuldade:** Média.

---

## 🌟 O Que Mais Faltava Além Dessas 3? (O Ecossistema Completo do CapCut Pro)

Para além das 4 partes originais, um editor para criadores no topo absoluto da categoria conta com **outras 5 frentes estratégicas**:

---

### 💬 PARTE 5: Legendas Dinâmicas & Estilizadas Virais (Auto-Captions Estilo Hormozi / MrBeast)
* **O que é:** O Drift já transcreve áudio via Whisper e edita vídeo por texto. O próximo salto é a estilização visual automática.
* **Recursos:**
  - Animação palavra por palavra (*Word-by-Word Pop*).
  - Destaque colorido (amarelo/verde limão) na palavra exata falada no momento.
  - Emojis contextuais inseridos automaticamente (ex: palavra "dinheiro" $\rightarrow$ 💸).
* **Dificuldade:** Média.
* **Impacto:** Altíssimo para Reels, Shorts e TikTok.

---

### ✂️ PARTE 6: Recorte Inteligente de Fundo (Smart Cutout / Auto Cutout em 1-Clique)
* **O que é:** Remoção de fundo de apresentadores sem tela verde (*green screen*).
* **Como funciona no Drift:**
  - O Drift já possui o motor C++ com `RvmMatter` e `Sam2Segmenter` usando ONNX Runtime local.
  - Falta apenas o botão intuitivo de 1 clique no painel de vídeo *"Remover Fundo (Auto Cutout)"*.
  - Permite criar o efeito viral de colocar **textos grandes flutuando atrás da pessoa** duplicando a faixa.
* **Dificuldade:** Média.
* **Impacto:** Crítico (uma das funções mais usadas do CapCut Pro).

---

### 🥁 PARTE 7: Detecção de Batidas e Cortes no Ritmo (Auto-Beats na Timeline) (100% CONCLUÍDA)
* **O que é:** O editor analisa a faixa de música importada e coloca marcadores visuais magnéticos (pontos amarelos) nos bumbos e caixas (*kicks & snares*).
* **Como funciona no Drift:**
  - O motor `AudioOnsets.cpp` analisa transientes espectrais e calcula o BPM do projeto.
  - Conectado à régua da timeline em `TimelinePanel.qml` com losangos e ticks amarelos dourados (`#FFD600`).
  - Snap magnético do cursor, trimming e ferramenta de corte `B` atraídos para cada batida.
  - Fatiamento rítmico automático de clipes com 1 clique (`splitClipAtBeats`) e conversão para Bookmarks da timeline.
  - Botão interativo no `TimelineToolbar.qml` com menu de opções e card no `AudioInspector.qml`.
* **Status:** ✅ **100% Concluído**

---

### 🎯 PARTE 8: Rastreamento de Movimento & Face Tracking (Motion Tracking)
* **O que é:** Fixar um sticker, texto, seta indicativa ou mosaico de desfoque (para censurar placas ou rostos) que acompanha o objeto ou pessoa em movimento.
* **Como funciona no Drift:**
  - Os motores nativos `FaceLandmarker.cpp`, `FaceMesh.cpp` e `ObjectDetector.cpp` calculam os vetores de movimento e atualizam os keyframes de posição X/Y, escala e rotação automaticamente.
* **Dificuldade:** Média a Alta.
* **Impacto:** Recurso profissional avançado.

---

### 📱 PARTE 9: Presets de Redes Sociais & Guias de Zona Segura (TikTok, Reels, Shorts)
* **O que é:** Ferramentas dedicadas para formatos verticais (9:16).
* **Recursos:**
  - Guias de Zona Segura (*Safe Zone Overlays*): Mostra na tela de preview os locais onde ficam os botões nativos do TikTok/Instagram (like, perfil, legenda inferior) para que o editor não posicione textos importantes em áreas cobertas.
  - Exportação direta em 1 clique com bitrate otimizado para não sofrer compressão destrutiva pelos servidores das redes sociais.
* **Dificuldade:** Baixa.
* **Impacto:** Imediato e muito prático para o fluxo diário de criadores.

---

## 📊 Matriz Comparativa do Ecossistema CapCut Pro vs Drift

| Frente / Módulo | CapCut Pro | Drift Video Editor | Status no Drift |
| :--- | :--- | :--- | :--- |
| **Parte 1: Áudio Pro (Voz Estúdio, Ducking, 8D, Isolador Vocal)** | Pago (Pro / Nuvem) | **Nativo C++, 100% Offline e Grátis** | ✅ **100% Concluído** |
| **Parte 2: Transições de Alto Impacto com Sound FX** | Limitado (Som separado) | **Nativo com Whoosh Integrado** | ✅ **100% Concluído** |
| **Parte 3: Motions Dinâmicos & Speed Ramping** | Biblioteca rica | **Bézier Curves & Motions Skia** | ✅ **100% Concluído** |
| **Parte 4: Modos de Mesclagem & Overlays** | Suportado | **Nativo Skia/OpenGL (`SkBlendMode`)** | ✅ **100% Concluído** |
| **Parte 5: Legendas Dinâmicas (Hormozi/MrBeast)** | Pago (Pro) | **Transcrição Whisper Local + Animação** | ✅ **100% Concluído** |
| **Parte 6: Auto Cutout (Recorte de Fundo de Pessoa)** | Pago (Pro) | **RVM/SAM2 ONNX Nativo Local** | ✅ **100% Concluído** |
| **Parte 7: Auto-Beats (Detecção Rítmica na Timeline)** | Suportado | **Engine `AudioOnsets` C++ + Snap Magnético** | ✅ **100% Concluído** |
| **Parte 8: Rastreamento de Movimento (Motion Tracking)** | Pago (Pro) | **Motores Face/Object Tracker Nativos** | ⏳ *Próxima Etapa (Planejado)* |
| **Parte 9: Zonas Seguras de Redes Sociais (9:16 Safe Zones)** | Suportado | **Canvas 9:16 com Guias Visuais** | ⏳ *Planejado* |

---

## 📌 Ordem de Execução Recomendada

1. **Parte 2: Transições de Alto Impacto** (Smooth Zoom, Whip Pan, Film Burn, Glitch Pro e Sound FX Whoosh).
2. **Parte 4: Modos de Mesclagem de Overlays** (*Screen, Multiply, Overlay, Color Dodge* no Skia).
3. **Parte 3: Motions e Animações de Camada** (Entrada, Saída, Shake de Batida e Speed Ramping).
4. **Parte 6: Auto Cutout 1-Clique** (Exposição da interface do RVM para remoção de fundo).
5. **Parte 5: Legendas Estilizadas Virais** (Highlight dinâmico palavra por palavra).
