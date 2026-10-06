project('zenith-panel', 'c',
    version: '0.1.0',
    license: 'GPL-3.0-or-later',
    default_options: [
        'c_std=c11',
        'warning_level=3',
        'werror=true',
    ],
)

# Dependencies
gtk4 = dependency('gtk4', version: '>= 4.10')
gtk4_layer_shell = dependency('gtk4-layer-shell-0', version: '>= 1.0')

# Source files
src = files(
    'src/main.c',
    'src/panel.c',
    'src/clock.c',
    'src/workspace-indicator.c',
)

# Build
executable('zenith-panel',
    src,
    dependencies: [
        gtk4,
        gtk4_layer_shell,
    ],
    install: true,
    install_dir: get_option('bindir'),
)
