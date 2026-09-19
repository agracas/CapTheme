# CapTheme — Alpha

**Versão: `v0.1.0-alpha.1` · Pré-lançamento experimental · Linux**

Esta alpha permite testar a nova interface. Ainda não é uma versão estável;
use cópias dos seus projetos ao experimentar. Não há instalador ou AppImage nesta versão.

Fork do Kdenlive com interface de edição inspirada no CapCut desktop.
O código-fonte completo está em `kdenlive/`, com os avisos de autoria e as licenças originais.
Esta publicação contém um snapshot modificado do Kdenlive; a origem está identificada abaixo.
Base: `https://github.com/KDE/kdenlive`, commit `046da7d04fc66206d61468664f079ab84902eaac`.

![Interface real do CapTheme](docs/captheme.png)

## Executar

Ambiente validado: **CachyOS (base Arch Linux), x86_64**. Windows, macOS,
Flatpak e outras distribuições ainda não foram validados nesta alpha.

1. Instale as ferramentas e dependências:

   ```sh
   sudo pacman -S --needed base-devel git cmake ninja extra-cmake-modules qt6-tools kdenlive
   ```

   O pacote oficial `kdenlive` fornece as dependências de execução e os headers
   correspondentes no Arch/CachyOS. O CapTheme será compilado e instalado em outra pasta.

2. Baixe a alpha:

   ```sh
   git clone --branch v0.1.0-alpha.1 --depth 1 https://github.com/agracas/CapTheme.git
   cd CapTheme
   ```

3. Compile e abra:

   ```sh
   ./build.sh
   ./run.sh
   ```

A primeira compilação pode demorar e precisa de alguns gigabytes livres.
O download **Source code** da release também contém o código completo: extraia-o,
abra um terminal na pasta extraída e execute os mesmos scripts.

A compilação também usa Qt 6.10+, KDE Frameworks 6.21+, FFmpeg e os demais
componentes listados em `kdenlive/CMakeLists.txt`. O build usa 6 tarefas por padrão;
`CAPTHEME_JOBS=4 ./build.sh` reduz o consumo de memória.

O programa é instalado em `install/`, sem substituir um Kdenlive do sistema.
O launcher mantém configurações e caches em `.runtime/` e aceita caminhos de
projetos: `./run.sh /caminho/projeto.kdenlive`.

## Interface

Para experimentar o painel atualizado na versão de desenvolvimento (ainda alpha),
baixe a branch `main` em uma nova pasta:

```sh
git clone --branch main https://github.com/agracas/CapTheme.git CapTheme-dev
cd CapTheme-dev
./build.sh
./run.sh
```

Para atualizar essa cópia posteriormente, feche o programa e execute
`git pull --ff-only`, `./build.sh` e `./run.sh`. A release `v0.1.0-alpha.1`
preserva a primeira versão e não inclui o novo painel.

- Tema grafite com seleção verde-água, abas e campos compactos.
- Barra com Importar, Mídia, Áudio, Texto, Efeitos, Composições, Legendas e Exportar.
- Mídia à esquerda, monitor ao centro, propriedades à direita, timeline embaixo.
- Os botões usam os comandos reais do Kdenlive, inclusive seus estados habilitados.
- O layout é criado no primeiro uso. Alterações posteriores são preservadas.
- **Reset Workspace** restaura a organização proposta; os menus completos ficam no botão de menu.

“Áudio” abre o mixer; “Texto” cria um título; “Legendas” adiciona uma legenda à
timeline. “Composições” abre as composições do Kdenlive. Essas funções não são
catálogos online do CapCut. O projeto não inclui recursos, serviços ou marcas do CapCut.

Para abrir o fork com a interface original e configurações separadas:

```sh
CAPTHEME_CLASSIC=1 ./run.sh --config classicrc
```

## Código

### Painel Details: transformação e animação

Na versão de desenvolvimento, selecione um clipe e use os atalhos no painel direito:

- **Transform**: posição X/Y, escala, largura/altura, opacidade, rotação e ponto de rotação.
- **Crop**: recorte por retângulo, cantos arredondados ou círculo, com preenchimento.
- **Volume**: ganho de áudio com animação por keyframes.

O primeiro clique adiciona o efeito correspondente; os próximos abrem o efeito já
existente, preservando seus valores. Sem um clipe selecionado, os atalhos ficam
desabilitados. Controles de vídeo e áudio acompanham o tipo de clipe selecionado.

Nos controles do efeito, mova o cursor e use o botão de losango para adicionar ou
remover keyframes. As setas navegam entre eles; os ajustes usam o desfazer/refazer
e o formato de projeto do Kdenlive. Na timeline, as mudanças afetam a instância
selecionada; na biblioteca de mídia, afetam o clipe de origem.

O controle de rotação pode depender da versão do efeito Transform fornecida pelo MLT.

![Inspector com parâmetros e keyframes reais](docs/inspector.png)

Teste do inspector:

```sh
./tests/inspector-smoke.sh
```

O teste importa um vídeo sintético com áudio, altera escala, opacidade e posição,
confere desfazer/refazer e ausência de duplicação, cria um segundo keyframe e salva
o projeto. A validação do XML confirma os valores e os dois keyframes persistidos.
Os arquivos de teste e a captura ficam em `artifacts/`.

### Arquivos principais

- `kdenlive/src/captheme.cpp`: barra, comandos e organização dos painéis.
- `kdenlive/data/captheme/`: paleta KDE e stylesheet Qt embutidos no executável.
- `kdenlive/src/mainwindow.cpp`: integração após restauração do workspace.
- `kdenlive/src/assets/assetpanel.cpp`: atalhos e contexto do inspector.
- `kdenlive/src/effects/effectstack/view/effectstackview.cpp`: abertura de efeitos existentes sem duplicação.

Esta implementação mantém os componentes de monitor, inspector e timeline do
Kdenlive. Não é uma reprodução pixel a pixel do CapCut nem implementa serviços
exclusivos dele. As mudanças do fork seguem a licença GPL do Kdenlive.

## Validação

Compilado e instalado localmente com GCC 16, Qt 6.11.2, KDE Frameworks 6.30 e MLT 7.40.
O teste gráfico passou na primeira abertura e após reiniciar: posição e visibilidade
dos painéis, paleta, menu compacto, comando de exportação, navegação para efeitos,
restauração repetida do workspace e fechamento normal. A captura acima foi feita
no executável real em uma tela virtual de 1920×1080, com janela de 1600×960.

Para repetir (requer `xorg-server-xvfb`):

```sh
./tests/ui-smoke.sh
```

Os logs e a captura ficam em `artifacts/`. Esse teste usa um projeto vazio e não
valida edição ou renderização de um vídeo completo. Os testes de interface são
carregados externamente; o executável não contém comandos de teste.

Referências: [personalização do Kdenlive](https://docs.kdenlive.org/en/user_interface/customizing_interface.html),
[código original](https://github.com/KDE/kdenlive),
[compilação upstream](https://github.com/KDE/kdenlive/blob/master/dev-docs/build.md).

## Limitações da alpha

- A aparência e o layout ainda podem mudar; não é uma cópia exata do CapCut.
- O motor de edição, os monitores e a timeline continuam sendo os do Kdenlive.
- Recursos online, templates e serviços exclusivos do CapCut não estão incluídos.
- Algumas legendas da nova interface podem aparecer em inglês.
- Edição prolongada, renderização de projetos completos e compatibilidade com diferentes GPUs ainda precisam de validação.
- A base é uma revisão de desenvolvimento do Kdenlive, não uma versão estável dele.

## Problemas ao executar

- **“Compile primeiro”**: execute `./build.sh` e confirme que ele terminou sem erros.
- **Dependência ausente no CMake**: confira as versões mínimas acima e o erro do CMake.
- **Memória insuficiente durante a compilação**: use `CAPTHEME_JOBS=2 ./build.sh`.
- **Painéis fora do lugar**: use **Reset Workspace** na barra superior.

Relate problemas nas [Issues](https://github.com/agracas/CapTheme/issues), incluindo
a versão da alpha, distribuição, GPU, passos para reproduzir e o erro do terminal.

## Licença e créditos

O CapTheme é uma modificação independente do [Kdenlive](https://kdenlive.org/),
sem afiliação com KDE ou CapCut. Veja [COPYING](COPYING),
[licenças dos componentes](kdenlive/LICENSES/) e os avisos de autoria no código.
As modificações de interface estão descritas em [CHANGELOG.md](CHANGELOG.md).
