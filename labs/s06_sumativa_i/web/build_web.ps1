# Compila s06_sumativa_i a WebAssembly con Emscripten.
# Requiere: emsdk instalado y emsdk_env.ps1 cargado en la sesion.
# Uso:  .\web\build_web.ps1
$ErrorActionPreference = "Stop"

$lab = Split-Path -Parent $PSScriptRoot           # .../labs/s06_sumativa_i
$out = Join-Path $PSScriptRoot "dist"

if (-not (Get-Command em++ -ErrorAction SilentlyContinue)) {
    throw "No se encuentra em++. Carga el entorno de Emscripten: . <emsdk>\emsdk_env.ps1"
}

New-Item -ItemType Directory -Path $out -Force | Out-Null

$gameSources = @(
    "Juego", "Mapa", "Obstaculos", "Personaje", "Burbuja", "OrdenUI", "UI"
) | ForEach-Object { Join-Path $lab "Game\$_.cpp" }

$assets = Join-Path $lab "assets"

em++ -O2 -std=c++17 `
    -I $lab `
    @gameSources (Join-Path $lab "web\web_main.cpp") `
    -sUSE_SDL=2 `
    -sALLOW_MEMORY_GROWTH=1 `
    "--preload-file=$($assets)@assets" `
    --shell-file (Join-Path $lab "web\shell.html") `
    -o (Join-Path $out "index.html")

if ($LASTEXITCODE -ne 0) { throw "em++ fallo con codigo $LASTEXITCODE" }

Write-Host "Listo: $out\index.html"
