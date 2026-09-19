# Changelog

## Em desenvolvimento

- Painel Details com atalhos para transformação, recorte e volume do clipe selecionado.
- Acesso direto aos controles nativos de posição, escala, rotação, opacidade e keyframes.
- Reabertura do efeito existente sem duplicar efeitos ou substituir parâmetros.
- Estado vazio com orientação e atalhos desabilitados conforme a seleção e o tipo de mídia.

## v0.1.0-alpha.1

Primeira alpha pública do CapTheme, uma interface inspirada no CapCut construída sobre o Kdenlive.

- Paleta escura com destaque verde-água, abas no topo e menu compacto.
- Barra de acesso a mídia, áudio, texto, efeitos, composições, legendas e exportação.
- Workspace com mídia à esquerda, prévia central, propriedades à direita e timeline embaixo.
- Restauração do workspace e preservação do layout entre sessões.
- Launcher com configurações isoladas e instalação local, sem sobrescrever o editor do sistema.
- Scripts de compilação e testes gráficos, instruções de execução e captura real da interface.

Validação: compilação local e 18 verificações gráficas na primeira abertura e após reiniciar.
Os testes usam um projeto vazio; não validam um fluxo completo de edição e renderização.

Distribuição: código-fonte completo. Sem binários, AppImage ou suporte validado a Windows/macOS nesta alpha.
Base upstream: KDE/kdenlive, commit `046da7d04fc66206d61468664f079ab84902eaac`.
