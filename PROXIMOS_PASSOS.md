# Drift Video Editor - Próximos Passos e Sugestões Estratégicas
> **Roteiro para Superar o CapCut Desktop e Consolidar o Drift como o Editor de Escolha dos Criadores**

Este documento detalha os recursos restantes para completar 100% da paridade com o CapCut e as inovações que colocarão o Drift à frente no mercado.

---

## 🚀 1. Recursos para Alcançar 100% do CapCut

### 1.1 Recorte Automático de Fundo (Auto Cutout de Pessoa em 1-Clique)
* **Conceito:** Permitir que o criador selecione qualquer vídeo com uma pessoa e remova o fundo com apenas 1 clique, sem precisar de tela verde (chroma key).
* **Por que é essencial:** Criadores de conteúdo usam isso constantemente para colocar textos grandes atrás do orador, duplicar a camada com efeito de brilho ou trocar o cenário do quarto/estúdio.
* **Como implementar no Drift:**
  * O motor do Drift já possui a biblioteca `RvmMatter` e `Sam2Segmenter` em C++ com ONNX Runtime.
  * Adicionar o botão *"Remover Fundo (Auto Cutout)"* na aba de Máscaras/Propriedades do clipe de vídeo.
  * Ao ativar, o renderizador aplica a máscara alfa em tempo real.
* **Dificuldade:** Média.

---

### 1.2 Presets de Curvas de Velocidade Viral (Speed Ramping)
* **Conceito:** Variações dramáticas de velocidade no mesmo clipe (rápido $\rightarrow$ câmera lenta no momento do impacto $\rightarrow$ rápido novamente).
* **Presets Prontos no Estilo CapCut:**
  * **Montage:** Início acelerado ($3.0\times$), desaceleração cinematográfica no clímax ($0.4\times$) e saída rápida.
  * **Hero:** Rápido $\rightarrow$ Slow-mo estendido $\rightarrow$ Velocidade normal.
  * **Bullet:** Velocidade normal $\rightarrow$ Congelamento / câmera lenta quase total $\rightarrow$ Saída rápida.
  * **Flash In / Flash Out:** Efeito chicote de aceleração nas extremidades do clipe.
* **Como implementar no Drift:**
  * O Drift já possui o motor matemático de curvas Bézier em `src/core/SpeedCurve.h`.
  * Adicionar botões de 1-clique com mini-gráficos ilustrativos na janela `SpeedCurveWindow.qml` e no Inspetor.
* **Dificuldade:** Baixa a Média.

---

### 1.3 Detecção Rítmica de Batidas (Auto-Beats na Timeline)
* **Conceito:** Analisar a trilha musical selecionada e marcar automaticamente os pontos de batida (kick e snare) com pequenos pontos amarelos na timeline.
* **Por que é essencial:** Permite que o criador corte os clipes exatamente no tempo da música sem precisar ficar ouvindo e pausando repetidamente.
* **Como implementar no Drift:**
  * O Drift já tem o componente `AudioOnsets.cpp` para detecção de transientes.
  * Conectar a detecção aos marcadores visuais da régua da timeline em `TimelinePanel.qml`.
* **Dificuldade:** Média.

---

### 1.4 Exportação Otimizada em 1-Clique (TikTok, Reels, Shorts)
* **Conceito:** Um menu de exportação simplificado com botões diretos:
  * `[ Exportar para TikTok / Reels (1080x1920 60fps) ]`
  * `[ Exportar para Shorts (1080x1920 30fps) ]`
  * `[ Exportar para YouTube Horizontal (4K/1080p 60fps) ]`
* **Benefício:** Ajusta automaticamente o perfil de cores (BT.709), o codec H.264/HEVC e a taxa de bits ideal para que o algoritmo do Instagram e TikTok não degrade a qualidade do vídeo ao fazer o upload.
* **Dificuldade:** Baixa.

---

## 🏆 2. O Que Colocar para Deixar o Drift AINDA MELHOR que o CapCut

Para ser o editor favorito, o Drift pode explorar as maiores fraquezas e reclamações dos usuários do CapCut:

| Vantagem Competitiva | Como Implementar no Drift | Impacto para o Usuário |
| :--- | :--- | :--- |
| **100% Gratuito e Sem Bloqueios** | Manter todas as funções profissionais liberadas (sem a assinatura "Pro" abusiva do CapCut). | Atrai milhões de criadores cansados de pagar mensalidades. |
| **Privacidade Total (Zero Cloud)** | Garantir que vídeos confidenciais nunca saiam da máquina do usuário. | Atrai agências, empresas e criadores com sigilo contratual. |
| **Edição Autônoma com Agentes de IA** | Usar a integração MCP nativa para permitir que agentes montem o primeiro corte do vídeo sozinhos. | Nenhum outro editor no mercado possui essa capacidade integrada. |
| **Formatos Profissionais (ProRes / DNxHR)** | Suporte completo a renderização master sem perdas para pós-produção profissional. | Supera a limitação do CapCut que só foca em MP4 básico. |

---

## 📌 Ordem Recomendada de Execução

1. **Auto Cutout (Recorte de Fundo de Pessoa sem Chroma Key)** - *Dificuldade: Média*
2. **Presets de Speed Ramping (Curvas de Velocidade)** - *Dificuldade: Baixa/Média*
3. **Exportação Rápida para TikTok / Reels / Shorts** - *Dificuldade: Baixa*
4. **Auto-Beats na Timeline (Detecção Rítmica de Música)** - *Dificuldade: Média*
