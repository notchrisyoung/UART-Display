# PlatformIO extra script: adds the screen tester as project tasks.
#
#   pio run -t screentest    interactive tester (scripts/screen_test.py)
#   pio run -t screendemo    show every screen once and report pass/fail
#
# Both also appear in VS Code under PlatformIO > Project Tasks > Custom.
# They use PlatformIO's own Python, which already includes pyserial.

import os
import subprocess

Import("env")  # noqa: F821  (provided by PlatformIO/SCons)

SCRIPT = os.path.join(env.subst("$PROJECT_DIR"), "scripts", "screen_test.py")  # noqa: F821


def _port():
    # Same port the monitor/upload use, if one is set in platformio.ini or on the command line
    for option in ("monitor_port", "upload_port"):
        value = env.GetProjectOption(option, "")  # noqa: F821
        if value:
            return value
    return env.subst("$UPLOAD_PORT")  # noqa: F821


def _run(extra_args):
    def action(target, source, env):
        cmd = [env.subst("$PYTHONEXE"), SCRIPT]
        port = _port()
        if port:
            cmd += ["--port", port]
        cmd += extra_args
        # Run in the foreground so the interactive menu gets the keyboard
        return subprocess.call(cmd)
    return action


env.AddCustomTarget(  # noqa: F821
    name="screentest",
    dependencies=None,
    actions=[_run([])],
    title="Screen Test",
    description="Interactive screen tester for the display",
    always_build=True,
)

env.AddCustomTarget(  # noqa: F821
    name="screendemo",
    dependencies=None,
    actions=[_run(["--demo"])],
    title="Screen Demo",
    description="Show every screen once and report pass/fail",
    always_build=True,
)
