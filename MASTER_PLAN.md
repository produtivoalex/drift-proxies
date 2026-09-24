# 🚀 Drift Master Plan: Superando DarkPlanner e CapCut

Este documento detalha o plano arquitetônico e de implementação definitivo para transformar o Drift na **plataforma absoluta de criação de conteúdo para canais Dark, YouTube, Shorts e Podcasts**, superando largamente ferramentas como DarkPlanner e CapCut em automação, inteligência e fluidez de interface.

---

## 🎯 1. O "Dark Studio Wizard" (Motor de Automação de Roteiro para Vídeo)

O DarkPlanner possui um gerador básico, mas o Drift será um **Diretor de Arte com IA**. O "Dark Studio Wizard" transformará um roteiro de texto em um vídeo montado, decupado e sonorizado na timeline em segundos.

### Arquitetura C++ / Engine (`src/engine/wizard/`)
*   **`WizardEngine.cpp`**: O cérebro do assistente. Analisa o roteiro, identifica o humor, o nicho (Terror, Finanças, Mistério) e orquestra as chamadas para as outras sub-engines.
*   **Integração com TTS (`TtsSynthesizer`)**: Gera automaticamente a locução baseada nas vozes premium brasileiras (Antonio, Thalita, etc.).
*   **Alinhamento Forçado (Speech-to-Timeline)**: Usando a engine Whisper interna para sincronizar exatamente os fonemas gerados com os blocos da timeline, garantindo que o B-Roll e as legendas apareçam no frame exato da fala.
*   **B-Roll Inteligente & Ritmo**: 
    *   Análise semântica do texto (via integração local/LLM) para buscar vídeos na biblioteca local de B-Rolls.
    *   Cortes automáticos ("Jump Cuts") nos silêncios.
    *   Aplicação automática do efeito Ken Burns (Zoom in/Pan) sutil nas imagens.
*   **Sonorização Dinâmica (Auto-SFX & BGM)**:
    *   Detecção de "pontos de impacto" no texto para adicionar efeitos sonoros (Whoosh, Boom, Riser).
    *   Bed track (música de fundo) com *auto-ducking* (abaixa o volume da música automaticamente quando a voz fala, usando o `Compressor` e sidechaining em `src/engine/audio/`).

### UI/UX QML (`src/qml/components/wizard/`)
*   **`DarkStudioWizard.qml`**: Um modal flutuante e imersivo.
    *   **Passo 1**: Cole o Roteiro.
    *   **Passo 2**: Escolha a Vibe/Nicho (Investigação, Dinheiro, Motivacional).
    *   **Passo 3**: Escolha a Voz.
    *   **Ação**: "Gerar Timeline Mágica".

---

## 📊 2. Pipeline Multicanal (Kanban de Produção)

Para superar o DarkPlanner, o Drift precisa ser não apenas um editor, mas um **estúdio de gerenciamento**.

### Arquitetura C++ / Engine (`src/models/kanban/`)
*   **`ProjectPipelineManager.cpp`**: Gerencia o estado de múltiplos vídeos ("Ideia", "Roteirizando", "Pronto para Edição", "Renderizando", "Publicado").
*   **Serialização**: Salva o estado do quadro Kanban em um JSON global no workspace do usuário.

### UI/UX QML (`src/qml/views/PipelineView.qml`)
*   Interface arrastar e soltar (Drag and Drop) estilo Trello/Notion.
*   Integração direta com o editor: Clicar em um card na coluna "Pronto para Edição" carrega instantaneamente os assets (roteiro, voz) e abre a timeline do Drift configurada para aquele vídeo.

---

## 🎨 3. UX/UI: Respirável, Intuitiva e Focada (Macro-Hubs)

O CapCut é poluído; o Drift será uma obra de arte do design de software. Combinamos e otimizamos a interface para máximo espaço de timeline e preview.

### As 8 Macro-Hubs
1.  **Mídia**: Seus arquivos (vídeos, áudios, imagens locais).
2.  **Templates (1-Click)**: Layouts inteiros que se adaptam (ex: "Podcast 3 Câmeras", "Shorts Viral Legenda Amarela").
3.  **Legendas**: Geração Whisper, estilos em massa, animação de palavras.
4.  **Cenários**: Acesso à biblioteca integrada de fundos dinâmicos e chroma key / RVM (Robust Video Matting).
5.  **Efeitos**: Filtros GLSL visuais e filtros JUCE de áudio (Isolador de Voz, Denoiser).
6.  **Transições**: Biblioteca visual de transições entre clipes.
7.  **Áudio**: Catálogo premium de SFX e trilhas sonoras.
8.  **Atalhos**: Configuração rápida de workflow.

### Workspaces Dinâmicos (`EditorHeader.qml`)
*   Um clique muda completamente o layout do Drift.
*   **Modo Shorts**: Preview fica vertical (9:16), a aba de Legendas abre automaticamente, painel de ferramentas fica focado em ritmo e corte rápido.
*   **Modo WebDoc**: Preview 16:9 widescreen, timeline multi-track expandida, propriedades de áudio (mixagem LUFS) ganham destaque.

---

## 🎬 4. Biblioteca de Assets Nativos Premium (O Fator "Wow")

Criadores de conteúdo perdem tempo baixando assets. O Drift trará os melhores embutidos ou baixáveis sob demanda.

*   **SFX Categorizados (`SfxCatalog.cpp`)**: 
    *   Impactos, Risers, Atmosferas de Terror, Glitches, Transições de Câmera.
*   **B-Rolls Essenciais**: 
    *   Pacote base offline de fundos animados (Particles, Abstract, Dark Gradients).
*   **Textos e Tipografia**: 
    *   Fontes já licenciadas e efeitos pré-renderizados (Neon, Glitch, 3D Pop-up).

---

## 🧠 5. Superpoderes de IA Invisíveis (Motorização)

Diferente de plugins lentos, o Drift já usa C++ e ONNX. Vamos expor isso de forma mágica.
*   **Segmentação Mágica (SAM2)**: O usuário clica em um objeto no preview, o Drift gera a máscara e permite colocar texto *atrás* da pessoa no vídeo, sem chroma key.
*   **Isolamento Vocal Profissional (DeepFilterNet)**: Um botão "Voz de Estúdio" no inspetor de áudio que remove eco e ruído instantaneamente.

---

## 🛠️ Cronograma de Execução Imediato

1.  **Fase 1: Finalizar UX/UI**: Otimização completa das Macro-Hubs e implementação total do botão de templates 1-click.
2.  **Fase 2: Motor Dark Studio Wizard (Core)**: Implementar a arquitetura C++ para gerar clips baseados em texto e alinhamento via Whisper/TTS.
3.  **Fase 3: Automação e Ritmo (Wizard)**: Adicionar B-Rolls dinâmicos, SFX automáticos com base em impacto no texto e Ken Burns aos geradores da timeline.
4.  **Fase 4: Pipeline Kanban de Produção**: Construir a aba focada na gestão de longo prazo e progresso dos vídeos.
