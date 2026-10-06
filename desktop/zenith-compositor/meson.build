project('zenith-compositor', 'c',
    version: '0.1.0',
    license: 'GPL-3.0-or-later',
    default_options: [
        'c_std=c11',
        'warning_level=3',
        'werror=true',
    ],
)

# Dependencies
wlroots = dependency('wlroots', version: '>= 0.17.0')
wayland_server = dependency('wayland-server')
wayland_protos = dependency('wayland-protocols')
xkbcommon = dependency('xkbcommon')
pixman = dependency('pixman-1')

# Source files
src = files(
    'src/main.c',
    'src/compositor.c',
    'src/output.c',
    'src/input.c',
    'src/view.c',
    'src/workspace.c',
    'src/keybindings.c',
)

# Build
executable('zenith-compositor',
    src,
    dependencies: [
        wlroots,
        wayland_server,
        wayland_protos,
        xkbcommon,
        pixman,
    ],
    install: true,
    install_dir: get_option('bindir'),
)
