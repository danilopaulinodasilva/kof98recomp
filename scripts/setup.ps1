<#
  setup.ps1 - the Quick start behind Setup.cmd.

  Checks every prerequisite and asks before installing anything, finds your
  KOF98 (kof98h) and Neo Geo ROM zips, verifies them, builds the recompiled game
  (the same commands as README.md's step-by-step route), and leaves a
  "KOF98" launcher in the repo folder. Re-running resumes: finished
  steps are skipped. Details of every step go to setup.log.

  Non-interactive use:
    Setup.cmd -RomZip C:\roms\kof98h.zip -BiosZip C:\roms\neogeo.zip -Yes
#>
param(
    [string]$RomZip,      # MAME "kof98h" set
    [string]$BiosZip,     # MAME "neogeo" set
    [string]$RomDir,      # or: a folder that already holds the unzipped files
    [switch]$Yes,         # answer yes to every install question
    [switch]$NoProfile,   # skip the profiling pass (faster, a little less native code)
    [switch]$NoWindow     # skip SDL2: headless-only build
)

$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$Log = Join-Path $Root 'setup.log'
$Roms = Join-Path $Root 'roms'
$Build = Join-Path $Root 'build'
$Deps = Join-Path $Root 'deps'
$SdlVersion = '2.32.10'
Set-Content -Path $Log -Value "setup started $(Get-Date -Format s)"

function Log([string]$m) { Add-Content -Path $Log -Value $m }
function Say([string]$m) { Write-Host $m; Log $m }
function Step([string]$m) { Write-Host ""; Write-Host "== $m" -ForegroundColor Cyan; Log "== $m" }
function Fail([string]$sentence) {
    Write-Host ""
    Write-Host "Setup stopped: $sentence" -ForegroundColor Red
    Write-Host "Details are in $Log"
    Log "FAILED: $sentence"
    exit 1
}
function Ask([string]$q) {
    if ($Yes) { Log "$q -> yes (-Yes)"; return $true }
    $a = Read-Host "$q [y/N]"
    Log "$q -> $a"
    return $a -match '^(y|yes)$'
}
function Run([string]$exe, [string[]]$argv) {
    # Native tools write progress to stderr; Windows PowerShell would turn
    # that into a terminating error under Stop.
    $ErrorActionPreference = 'Continue'
    Log "> $exe $($argv -join ' ')"
    $out = & $exe @argv 2>&1
    $out | ForEach-Object { Log "$_" }
    return $LASTEXITCODE -eq 0
}
function Has([string]$cmd) { return [bool](Get-Command $cmd -ErrorAction SilentlyContinue) }
function Winget([string]$id, [string]$extra) {
    if (-not (Has 'winget')) { Fail "winget is not available; install $id by hand and run Setup.cmd again." }
    $argv = @('install', '--id', $id, '-e', '--accept-package-agreements', '--accept-source-agreements')
    if ($extra) { $argv += @('--override', $extra) }
    if (-not (Run 'winget' $argv)) { Fail "installing $id failed; install it by hand and run Setup.cmd again." }
}

# ---- 1. prerequisites ----
Step 'Checking prerequisites'

if (-not (Has 'git')) {
    if (-not (Ask 'Git is needed to fetch the neogeorecomp toolkit. Install Git (about 60 MB) with winget?')) {
        Fail 'Git is required; install it from https://git-scm.com and run Setup.cmd again.'
    }
    Winget 'Git.Git' ''
    Fail 'Git was installed. Close this window and run Setup.cmd again so Windows picks it up.'
}
Say "  git:   $((git --version) -join '')"

$cmakeOk = $false
if (Has 'cmake') {
    $v = [regex]::Match(((cmake --version) -join ' '), '(\d+)\.(\d+)')
    $cmakeOk = ([int]$v.Groups[1].Value -gt 3) -or ([int]$v.Groups[1].Value -eq 3 -and [int]$v.Groups[2].Value -ge 21)
}
if (-not $cmakeOk) {
    if (-not (Ask 'CMake 3.21 or newer is needed to build. Install CMake (about 40 MB) with winget?')) {
        Fail 'CMake is required; install it from https://cmake.org and run Setup.cmd again.'
    }
    Winget 'Kitware.CMake' ''
    Fail 'CMake was installed. Close this window and run Setup.cmd again so Windows picks it up.'
}
Say "  cmake: $(((cmake --version) | Select-Object -First 1))"

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$vs = $null
if (Test-Path $vswhere) {
    $vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
}
if (-not $vs) {
    if (-not (Ask 'A C compiler is needed: Visual Studio 2022 Build Tools with C++ (about 3 GB). Install it with winget?')) {
        Fail 'a C compiler is required; install Visual Studio 2022 with "Desktop development with C++" and run Setup.cmd again.'
    }
    Winget 'Microsoft.VisualStudio.2022.BuildTools' '--quiet --wait --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended'
    $vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $vs) { Fail 'the C++ build tools did not install; install Visual Studio 2022 with C++ by hand.' }
}
Say "  C compiler: $vs"

$SdlDir = Join-Path $Deps "SDL2-$SdlVersion"
if (-not $NoWindow -and -not (Test-Path (Join-Path $SdlDir 'cmake\sdl2-config.cmake'))) {
    if (Ask "SDL2 draws the game window. Download SDL2 $SdlVersion (about 7 MB) from github.com/libsdl-org?") {
        New-Item -ItemType Directory -Force -Path $Deps | Out-Null
        $zip = Join-Path $Deps "SDL2-devel-$SdlVersion-VC.zip"
        $url = "https://github.com/libsdl-org/SDL/releases/download/release-$SdlVersion/SDL2-devel-$SdlVersion-VC.zip"
        Log "download $url"
        try { Invoke-WebRequest -Uri $url -OutFile $zip -UseBasicParsing } catch { Fail "downloading SDL2 failed ($($_.Exception.Message)); check your connection and run Setup.cmd again." }
        Expand-Archive -Path $zip -DestinationPath $Deps -Force
        Remove-Item $zip
    } else {
        Say '  SDL2 skipped: the build will be headless-only (no window).'
        $NoWindow = $true
    }
}
if (-not $NoWindow) { Say "  SDL2:  $SdlDir" }
if (-not (Has 'ffmpeg')) { Say '  ffmpeg: not found (only needed for --record)' }

# ---- 2. the toolkit ----
Step 'Fetching the neogeorecomp toolkit'
if (-not (Test-Path (Join-Path $Root 'neogeorecomp\ext\musashi\m68k_in.c'))) {
    if (Test-Path (Join-Path $Root '.git')) {
        if (-not (Run 'git' @('-C', $Root, 'submodule', 'update', '--init', '--recursive'))) { Fail 'git could not fetch the toolkit submodule; check your connection.' }
    } else {
        # A plain download (no .git): clone the toolkit at the pinned commit.
        $ref = (Get-Content (Join-Path $Root 'toolkit.ref') -ErrorAction Stop | Select-Object -First 1).Trim()
        $dst = Join-Path $Root 'neogeorecomp'
        if (Test-Path $dst) { Remove-Item -Recurse -Force $dst }
        if (-not (Run 'git' @('clone', 'https://github.com/danilopaulinodasilva/neogeorecomp.git', $dst))) { Fail 'git could not clone the toolkit; check your connection.' }
        if (-not (Run 'git' @('-C', $dst, 'checkout', $ref))) { Fail "the toolkit has no commit $ref; download a newer copy of this repo." }
        if (-not (Run 'git' @('-C', $dst, 'submodule', 'update', '--init', '--recursive'))) { Fail 'git could not fetch the toolkit''s submodules.' }
    }
}
Say '  toolkit ready'

# ---- 3. ROMs ----
Step 'Checking your ROM files'
Add-Type -TypeDefinition @'
public static class Crc32 {
    static readonly uint[] T = Make();
    static uint[] Make() {
        var t = new uint[256];
        for (uint i = 0; i < 256; i++) { uint c = i; for (int k = 0; k < 8; k++) c = (c & 1) != 0 ? 0xEDB88320u ^ (c >> 1) : c >> 1; t[i] = c; }
        return t;
    }
    public static uint Of(System.IO.Stream s) {
        uint c = 0xFFFFFFFFu; var buf = new byte[65536]; int n;
        while ((n = s.Read(buf, 0, buf.Length)) > 0) for (int i = 0; i < n; i++) c = T[(c ^ buf[i]) & 0xFF] ^ (c >> 8);
        return ~c;
    }
}
'@
Add-Type -AssemblyName System.IO.Compression.FileSystem

# MAME sets "kof98h" and "neogeo" (the files used from it).
$Game = [ordered]@{
    '242-pn1.bin' = '61ac868a'; '242-p2.bin' = '980aba4c'; '242-s1.bin' = '7f7b4805'; '242-mg1.bin' = '4e7a6b1b'
    '242-c1.bin' = 'e564ecd6'; '242-c2.bin' = 'bd959b60'; '242-c3.bin' = '22127b4f'; '242-c4.bin' = '0b4fa044'
    '242-c5.bin' = '9d10bed3'; '242-c6.bin' = 'da07b6a2'; '242-c7.bin' = 'f6d7a38a'; '242-c8.bin' = 'c823e045'
    '242-v1.bin' = 'b9ea8051'; '242-v2.bin' = 'cc11106e'; '242-v3.bin' = '044ea4e1'; '242-v4.bin' = '7985ea30'
}
$Bios = [ordered]@{ 'neo-po.bin' = '16d0c132'; 'sfix.sfix' = 'c2ea0cfd'; 'sm1.sm1' = '94416d67'; '000-lo.lo' = '5a86cff2' }

function FileOk([string]$path, [string]$crc) {
    if (-not (Test-Path $path)) { return $false }
    $s = [System.IO.File]::OpenRead($path)
    try { return ('{0:x8}' -f [Crc32]::Of($s)) -eq $crc } finally { $s.Close() }
}
function Missing($set) {
    return @($set.Keys | Where-Object { -not (FileOk (Join-Path $Roms $_) $set[$_]) })
}
function FromZip([string]$zip, $set) {
    $z = [System.IO.Compression.ZipFile]::OpenRead($zip)
    try {
        foreach ($name in $set.Keys) {
            $e = $z.Entries | Where-Object { $_.Name -ieq $name } | Select-Object -First 1
            if (-not $e) { continue }
            $out = Join-Path $Roms $name
            [System.IO.Compression.ZipFileExtensions]::ExtractToFile($e, $out, $true)
            if (-not (FileOk $out $set[$name])) { Remove-Item $out; Log "bad checksum: $name in $zip" }
        }
    } finally { $z.Dispose() }
}
function FindZip([string]$name, [string]$given) {
    if ($given) { return $given }
    $places = @($Root, $Roms, (Join-Path $env:USERPROFILE 'Downloads'), (Join-Path $env:USERPROFILE 'Desktop'))
    foreach ($p in $places) { $f = Join-Path $p $name; if (Test-Path $f) { Say "  found $f"; return $f } }
    $a = Read-Host "  Where is your $name? (full path, or Enter to skip)"
    Log "asked for $name -> $a"
    return $a.Trim('"')
}

New-Item -ItemType Directory -Force -Path $Roms | Out-Null
if ($RomDir) {
    foreach ($name in @($Game.Keys) + @($Bios.Keys)) {
        $src = Join-Path $RomDir $name
        if (Test-Path $src) { Copy-Item $src (Join-Path $Roms $name) -Force }
    }
}
if ((Missing $Game).Count) {
    $zip = FindZip 'kof98h.zip' $RomZip
    # a non-merged set carries the system ROMs too
    if ($zip -and (Test-Path $zip)) { FromZip $zip $Game; FromZip $zip $Bios }
}
if ((Missing $Bios).Count) {
    $zip = FindZip 'neogeo.zip' $BiosZip
    if ($zip -and (Test-Path $zip)) { FromZip $zip $Bios }
}
$gone = @(Missing $Game) + @(Missing $Bios)
if ($gone.Count) {
    Log "missing or wrong: $($gone -join ', ')"
    Fail "these ROM files are missing or don't match the MAME sets: $($gone -join ', '). Point Setup.cmd at your kof98h.zip and neogeo.zip."
}
Say '  all 20 ROM files present and verified'

# ---- 4. build ----
Step 'Building (the recompiler runs on your ROMs; a few minutes)'
$cfg = @('-S', $Root, '-B', $Build, '-G', 'Visual Studio 17 2022', '-A', 'x64', "-DKOF98_ROM_DIR=$Roms")
if (-not $NoWindow) { $cfg += "-DSDL2_DIR=$SdlDir\cmake" }
if (-not (Run 'cmake' $cfg)) { Fail 'CMake could not configure the build.' }
if (-not (Run 'cmake' @('--build', $Build, '--config', 'Release', '--parallel'))) { Fail 'the build failed.' }
Say '  built'

if (-not $NoProfile) {
    Step 'Profiling (finds code only seen at runtime; about a minute)'
    if (-not (Run 'cmake' @('--build', $Build, '--config', 'Release', '--target', 'profile'))) { Fail 'the profiling run failed.' }
    if (-not (Run 'cmake' @('--build', $Build, '--config', 'Release', '--parallel'))) { Fail 'the rebuild after profiling failed.' }
    Say '  profiled and rebuilt'
}

# ---- 5. launcher ----
Step 'Creating the launcher'
$exe = Join-Path $Build 'Release\kof98.exe'
if (-not (Test-Path $exe)) { Fail "the build did not produce $exe." }
$cmdArgs = if ($NoWindow) { '--headless' } else { '' }
Set-Content -Path (Join-Path $Root 'KOF98.cmd') -Encoding ASCII -Value @(
    '@echo off',
    "start `"`" `"%~dp0build\Release\kof98.exe`" --rom-path `"%~dp0roms`" $cmdArgs"
)
try {
    $sh = New-Object -ComObject WScript.Shell
    $lnk = $sh.CreateShortcut((Join-Path $Root 'KOF98.lnk'))
    $lnk.TargetPath = $exe
    $lnk.Arguments = "--rom-path `"$Roms`" $cmdArgs"
    $lnk.WorkingDirectory = $Root
    $lnk.Save()
} catch { Log "shortcut not created: $($_.Exception.Message)" }

Write-Host ""
Write-Host 'Done. Double-click "KOF98" in this folder to play.' -ForegroundColor Green
Write-Host 'Keys: arrows move, Z/X/C/V buttons A-D, 1 start (no coin: it runs as an AES), Esc quit. No sound yet.'
Log 'setup finished'
