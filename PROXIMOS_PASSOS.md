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

### 🪄 FASE 2: Motor Dark Studio Wizard (Core) - **< 100% CONCLUÍDA >**
*A fundação C++ para que o Drift consiga pegar um roteiro em texto, gerar a voz, e sincronizar a fala com a timeline.*

* **2.1. Parseamento de Roteiro e TTS (`WizardEngine.cpp`)**:
  - Implementado o recebimento do roteiro para processar blocos e acionar o `TtsSynthesizer` com as vozes selecionadas.
* **2.2. Speech-to-Timeline (Alinhamento Automático)**:
  - Integração realizada com a engine Whisper embutida que detecta os fonemas gerados e cria as faixas de áudio e as cues (legendas) diretamente no `Project`.

---

### 🎥 FASE 3: Automação Cinematográfica (Ritmo, B-Rolls e SFX do Wizard) - **< 100% CONCLUÍDA >**
*Uma vez que a locução está na timeline, o Drift atua como o editor humano, aplicando recursos.*

* **3.1. B-Roll Automático & Jump Cuts**:
  - Implementado o agrupamento semântico de blocos de fala e jump cuts automáticos cortando silêncios > 400ms.
  - Adição automatizada de B-Rolls correspondentes ao nicho (vibe) por bloco.
* **3.2. Ken Burns & Impactos**:
  - Injeção da Curva de Velocidade contínua (*Ken Burns*) aos vídeos estáticos (B-Rolls).
* **3.3. Sonorização Inteligente**:
  - Detecção léxica de impacto e adição automática de SFX ("Whoosh Deep", "Boom Bass").
  - Criação da Bed Track (música de fundo) com o filtro `auto_ducking` pré-acoplado.

---

### 📊 FASE 4: Pipeline Multicanal (Kanban de Produção) - **< 100% CONCLUÍDA >**
*Deixando de ser apenas o software de edição para ser o estúdio de gerenciamento dos canais Dark e criadores.*

* **4.1. Dashboard Kanban C++ (`ProjectPipelineManager`)**:
  - Salvar progresso de vídeos (Ideia -> Roteiro -> Edição -> Exportando -> Publicado).
* **4.2. UI de Arrastar e Soltar (`PipelineView.qml`)**:
  - Nova visualização QML que permite ao usuário ver todos os vídeos em fila, arrastá-los entre colunas e abrir o editor exatamente onde parou ao clicar no cartão.

---

## 📌 Próxima Etapa Imediata
O usuário concluiu a **Fase 4 (Pipeline Multicanal / Kanban de Produção)**. O Master Plan está agora totalmente concluído e a plataforma foi elevada de um simples editor de vídeo para um Estúdio e Motor de Criação Assistida completo. Aguardar instruções adicionais do usuário.
