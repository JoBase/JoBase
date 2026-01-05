import packaging.tags, subprocess, sysconfig, pathlib, sys, zipfile, shutil, hashlib, base64, platform

VERSION = "3.1"
NAME = "JoBase"

def build_wheel(wheel_directory, config_settings = None, metadata_directory = None):
    def write(src, path):
        file.write(src, pathlib.Path("JoBase") / path)

        hash = hashlib.sha256()
        item = open(src, "rb")

        while chunk := item.read(8192):
            hash.update(chunk)

        code = base64.urlsafe_b64encode(hash.digest()).rstrip(b"=").decode("ascii")
        lines.append(f"{NAME}/{path},sha256={code},{pathlib.Path(src).stat().st_size}")

    def writestr(path, list):
        text = "\n".join(list)
        bytes = text.encode("utf-8")
        hash = hashlib.sha256()

        file.writestr(path, text)
        hash.update(bytes)

        code = base64.urlsafe_b64encode(hash.digest()).rstrip(b"=").decode("ascii")
        lines.append(f"{path},sha256={code},{len(bytes)}")

    def clone(url, name):
        if not pathlib.Path("lib/" + name).exists():
            subprocess.run(["git", "clone", f"https://github.com/{url}.git", "lib/" + name, "--depth", "1", "--recursive"])

    base, ext = sysconfig.get_config_vars("installed_base", "EXT_SUFFIX")
    tag = next(packaging.tags.sys_tags())

    wheel = f"{NAME.lower()}-{VERSION}-{tag}.whl"
    build = "build/" + tag.platform
    file = zipfile.ZipFile(pathlib.Path(wheel_directory) / wheel, "w")
    lines = []

    cmake = [] if sys.platform != "win32" else [
        "-A",
        "Win32" if sys.maxsize < 2 ** 32 else "ARM64" if platform.machine() == "ARM64" else "x64"
    ]

    clone("memononen/libtess2", "libtess2")
    clone("libsdl-org/SDL_mixer", "mix")
    clone("nothings/stb", "stb")
    clone("JoBase/SDL", "sdl")

    if sys.platform == "linux":
        if shutil.which("apk"): subprocess.run([
            "apk", "add", "--no-cache",
            "libxi-dev",
            "libxrandr-dev",
            "libxkbcommon-dev",
            "wayland-dev",
            "wayland-protocols",
            "mesa-dev",
            "libdrm-dev"
        ])

        elif shutil.which("dnf"): subprocess.run([
            "dnf", "install", "-y",
            "libXi-devel",
            "libXrandr-devel",
            "libxkbcommon-devel",
            "wayland-devel",
            "wayland-protocols-devel",
            "mesa-libEGL-devel"
        ])

    subprocess.run(["cmake", "-S", ".", "-B", build, *cmake,
        "-DPython3_ROOT_DIR=" + base,
        "-DJOBASE_EXT=" + ext,
        "-DJOBASE_DIR=" + str(pathlib.Path(wheel_directory))
    ])

    subprocess.run(["cmake", "--build", build, "--config", "Release"])
    subprocess.run(["ls", str(pathlib.Path(wheel_directory))])

    for path in pathlib.Path("module").rglob("*"):
        if path.suffix in (".pyi", ".png", ".bin", ".wav"):
            write(path, pathlib.Path(*path.parts[1:]))

    writestr(f"{NAME.lower()}-{VERSION}.dist-info/METADATA", [
        "Metadata-Version: 2.1",
        "Name: " + NAME,
        "Version: " + VERSION,
        "Description-Content-Type: text/x-rst",
        "Summary: JoBase is a fast Python game library for beginner coders",
        "Keywords: game engine learn beginner",
        "Home-page: https://jobase.org",
        "Author: Reuben Grey Ford",
        "Author-email: <hello@jobase.org>",
        "",
        "Welcome to JoBase, the fastest Python game framework.",
        "",
        "Features",
        "========",
        "",
        "* Beginner friendly",
        "* Fast",
        "* Works on Windows, Linux, MacOS, and all modern browsers",
        "",
        "Installation",
        "============",
        "",
        "If you already have Python installed, you can install JoBase with pip.::",
        "",
        "    pip install JoBase",
        "",
        "Run Python Online",
        "=================",
        "",
        "If you don't want to install Python, and you're happy to code online, you can run the examples on `the website <https://jobase.org>`__."
    ])

    writestr(f"{NAME.lower()}-{VERSION}.dist-info/WHEEL", [
        "Wheel-Version: 1.0",
        f"Generator: {NAME} {VERSION}",
        "Root-Is-Purelib: false",
        "Tag: " + str(tag)
    ])

    write(pathlib.Path(wheel_directory) / ("__init__" + ext), "__init__" + ext)
    lines.append(f"{NAME.lower()}-{VERSION}.dist-info/RECORD,,")
    file.writestr(f"{NAME.lower()}-{VERSION}.dist-info/RECORD", "\n".join(lines))

    print("\n".join(lines))

    return wheel