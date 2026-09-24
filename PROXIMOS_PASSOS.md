# Drift Video Editor - Roteiro de Evolução & Próximos Passos
> **Roteiro Estratégico Master Plan: Superando o DarkPlanner e CapCut**  
> *O Foco Supremo em Automação Inteligente e Produtividade Multicanal*

Este documento consolida o roteiro definitivo para transformar o Drift em um verdadeiro **Motor de Criação Assistida por IA** (Dark Studio Wizard) e um **Dashboard de Gestão de Produção** (Pipeline Kanban), suplantando totalmente ferramentas como DarkPlanner e CapCut.

---

## 🏆 Status Atual do Projeto

### ✅ FASE 1: Interface UI/UX Respirável & Macro-Hubs (100% CONCLUÍDA)
O caos visual foi dominado, criando muito mais espaço para a inteligência artificial trabalhar:
* **Consolidação das Macro-Hubs**: De múltiplas abas confusas para as definitivas 8 abas (`Mídia, Templates, Legendas, Cenários, Efeitos, Transições, Áudio, Atalhos`).
* **Workspaces Dinâmicos 1-Clique**: O Drift agora reconfigura inteiramente a sua interface para *Shorts (9:16)*, *Legendas (100% altura)*, *Podcast (Mixer focus)* e *WebDoc* com apenas um clique.
* **Infra de Templates Embutidos**: Botão de Templates 1-clique criado para carregar edições prontas com facilidade.

---

## 🚀 As Fases Restantes (O Motor Mágico)

---

### 🪄 FASE 2: Motor Dark Studio Wizard (Core) - **< EM ANDAMENTO >**
*A fundação C++ para que o Drift consiga pegar um roteiro em texto, gerar a voz, e sincronizar a fala com a timeline.*

* **2.1. Parseamento de Roteiro e TTS (`WizardEngine.cpp`)**:
  - Receber o texto do roteiro, separar blocos e chamar o `TtsSynthesizer` com a voz selecionada.
* **2.2. Speech-to-Timeline (Alinhamento Automático)**:
  - Usar a engine Whisper já embutida para detectar os tempos exatos do áudio gerado e jogar o arquivo na timeline com marcadores de fala.
* **Dificuldade:** Alta.

---

### 🎥 FASE 3: Automação Cinematográfica (Ritmo, B-Rolls e SFX do Wizard)
*Uma vez que a locução está na timeline, o Drift atua como o editor humano, aplicando recursos.*

* **3.1. B-Roll Automático & Jump Cuts**:
  - Buscador local inteligente para preencher blocos de fala com B-Rolls ou imagens correspondentes ao nicho.
  - Corte automático em silêncios/pausas (Jump Cut semântico).
* **3.2. Ken Burns & Impactos**:
  - Adição automática da Curva de Velocidade *Ken Burns* em imagens paradas.
* **3.3. Sonorização Inteligente**:
  - Detecção de frases de forte impacto e injeção automática de **Whoosh**, **Risers** e **Booms**.
  - Ativação automática do **Auto-Ducking** sobre o *Bed Track* (música de fundo) inserido pelo usuário.
* **Dificuldade:** Média-Alta.

---

### 📊 FASE 4: Pipeline Multicanal (Kanban de Produção)
*Deixando de ser apenas o software de edição para ser o estúdio de gerenciamento dos canais Dark e criadores.*

* **4.1. Dashboard Kanban C++ (`ProjectPipelineManager`)**:
  - Salvar progresso de vídeos (Ideia -> Roteiro -> Edição -> Exportando -> Publicado).
* **4.2. UI de Arrastar e Soltar (`PipelineView.qml`)**:
  - Nova visualização QML que permite ao usuário ver todos os vídeos em fila, arrastá-los entre colunas e abrir o editor exatamente onde parou ao clicar no cartão.
* **Dificuldade:** Média.

---

## 📌 Próxima Etapa Imediata
O usuário já concluiu toda a **Fase 1**. O passo exato agora é iniciar a **Fase 2: Motor Dark Studio Wizard (Core)**, especificamente criando o cérebro `src/engine/wizard/WizardEngine.cpp` e a ponte via `AppController` para parsear o texto e disparar o TTS para a timeline.
