Primeiro pré-lançamento do **CapTheme**, uma modificação do Kdenlive com interface inspirada no CapCut desktop.

**Alpha experimental:** a interface ainda pode mudar e esta versão não está validada para produção. Experimente com cópias dos seus projetos.

### Como rodar no CachyOS/Arch

```bash
sudo pacman -S --needed base-devel git cmake ninja extra-cmake-modules qt6-tools kdenlive
git clone --branch v0.1.0-alpha.1 --depth 1 https://github.com/agracas/CapTheme.git
cd CapTheme
./build.sh
./run.sh
```

O pacote oficial do Kdenlive fornece as dependências. A instalação do CapTheme fica em `install/`, com configurações separadas em `.runtime/`.
Para reduzir o uso de memória: `CAPTHEME_JOBS=2 ./build.sh`.

### Incluído

Tema escuro com destaque verde-água; navegação por mídia, áudio, texto e efeitos; exportação em destaque; workspace reorganizado e restauração de layout.

### Estado da validação

Compilação e instalação locais concluídas. Os testes gráficos passaram na primeira execução e após reiniciar (18 verificações em cada execução).
Edição prolongada e renderização de um projeto completo ainda não foram validadas nesta alpha.

Esta release distribui **código-fonte**, sem instalador ou AppImage. Ambiente validado: CachyOS x86_64. Não inclui serviços exclusivos do CapCut e não reproduz todos os seus controles.

[Instruções completas e captura](https://github.com/agracas/CapTheme/blob/v0.1.0-alpha.1/README.md) · [Relatar problema](https://github.com/agracas/CapTheme/issues)
