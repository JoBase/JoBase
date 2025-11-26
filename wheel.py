import subprocess, sysconfig, pathlib, sys, zipfile, hashlib, base64, shutil, os

VERSION = "3.1"

def build_wheel(wheel_directory, config_settings = None, metadata_directory = None):
    def write(src, path):
        file.write(src, pathlib.Path("JoBase") / path)

        hash = hashlib.sha256()
        item = open(src, "rb")

        while chunk := item.read(8192):
            hash.update(chunk)

        code = base64.urlsafe_b64encode(hash.digest()).rstrip(b"=").decode("ascii")
        lines.append(f"JoBase/{path},sha256={code},{pathlib.Path(src).stat().st_size}")

    def writestr(path, list):
        text = "\n".join(list)
        bytes = text.encode("utf-8")
        hash = hashlib.sha256()

        file.writestr(path, text)
        hash.update(bytes)

        code = base64.urlsafe_b64encode(hash.digest()).rstrip(b"=").decode("ascii")
        lines.append(f"{path},sha256={code},{len(bytes)}")

    build, flags, include, ext, ver, abi = sysconfig.get_config_vars("BLDSHARED", "OPT", "INCLUDEPY", "EXT_SUFFIX", "py_version_nodot", "abiflags")
    tag = f"cp{ver}-cp{ver}{abi}-{os.environ.get("PLAT", sysconfig.get_platform().replace("-", "_").replace(".", "_"))}"
    wheel = f"JoBase-{VERSION}-{tag}.whl"
    file = zipfile.ZipFile(pathlib.Path(wheel_directory) / wheel, "w")
    out = "__init__" + ext

    lines = []
    source = list(pathlib.Path("src").glob("*.c")) + list(pathlib.Path("libtess2/Source").glob("*.c"))
    arch = [] if sys.maxsize > 2 ** 32 or sys.platform != "win32" else ["-A", "Win32"]

    if not pathlib.Path("sdl/build").exists():
        if sys.platform == "linux":
            if shutil.which("apk"):
                subprocess.run(["apk", "add", "--no-cache", "libxkbcommon-dev", "wayland-dev", "wayland-protocols", "mesa-dev", "libdrm-dev"])

            elif shutil.which("dnf"):
                subprocess.run(["dnf", "install", "-y", "libxkbcommon-devel", "wayland-devel", "wayland-protocols-devel", "mesa-libEGL-devel"])

        subprocess.run([
            "cmake", "-S", "sdl", "-B", "sdl/build", *arch,
            "-DBUILD_SHARED_LIBS=OFF",
            "-DSDL_SHARED=OFF",
            "-DCMAKE_OSX_ARCHITECTURES=x86_64;arm64",
            "-DCMAKE_OSX_DEPLOYMENT_TARGET=10.13",
            "-DCMAKE_POSITION_INDEPENDENT_CODE=ON",
            "-DSDL_AUDIO=OFF",
            "-DSDL_CAMERA=OFF",
            "-DSDL_JOYSTICK=OFF",
            "-DSDL_HAPTIC=OFF",
            "-DSDL_HIDAPI=OFF",
            "-DSDL_POWER=OFF",
            "-DSDL_SENSOR=OFF",
            "-DSDL_DIALOG=OFF",
            "-DSDL_X11=OFF"
        ])

        subprocess.run(["cmake", "--build", "sdl/build", "--config", "Release"])

    subprocess.run([
        "cl", *source,
        "/Fodist\\", "/LD", "/MD",
        "/I", include, "/I", "include", "/I", "stb",
        "/I", "libtess2\\Include", "/I", "sdl\\include", "/I", "freetype\\include",
        "/link",
        "/LIBPATH:sdl\\build\\Release", "SDL3-static.lib",
        "/LIBPATH:" + sysconfig.get_config_var("LIBDIR"),
        "user32.lib", "winmm.lib", "advapi32.lib", "ole32.lib", "gdi32.lib",
        "shell32.lib", "setupapi.lib", "version.lib", "imm32.lib",
        "/OUT:" + str(pathlib.Path(wheel_directory) / out)
    ] if sys.platform == "win32" else [
        *build.split(), *flags.split(), *([
        "-framework", "GameController",
        "-framework", "ForceFeedback",
        "-framework", "AppKit",
        "-g0",
        "-Wstrict-prototypes", "-Wsign-compare"
    ] if sys.platform == "darwin" else []),
        *source,
        "-I" + include, "-Iinclude", "-Istb",
        "-Ilibtess2/Include", "-Isdl/include",
        "-Lsdl/build", "-lSDL3",
        "-fPIC",
        "-o", pathlib.Path(wheel_directory) / out
    ])

    for path in pathlib.Path("module").rglob("*"):
        if path.suffix in (".pyi", ".png", ".ttf", ".bin"):
            write(path, pathlib.Path(*path.parts[1:]))

    writestr(f"JoBase-{VERSION}.dist-info/METADATA", [
        "Metadata-Version: 2.1",
        "Name: JoBase",
        "Version: " + VERSION,
        "Summary: JoBase is a fast Python game library for beginner coders",
        "Keywords: game engine learn beginner",
        "Home-page: https://jobase.org",
        "Author: Reuben Grey Ford",
        "Author-email: <hello@jobase.org>"
    ])

    writestr(f"JoBase-{VERSION}.dist-info/WHEEL", [
        "Wheel-Version: 1.0",
        "Generator: JoBase " + VERSION,
        "Root-Is-Purelib: false",
        "Tag: " + tag
    ])

    write(pathlib.Path(wheel_directory) / out, out)
    lines.append(f"JoBase-{VERSION}.dist-info/RECORD,,")
    file.writestr(f"JoBase-{VERSION}.dist-info/RECORD", "\n".join(lines))

    return wheel