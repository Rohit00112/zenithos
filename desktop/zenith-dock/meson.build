project('zenith-dock', 'c',
    version: '0.1.0',
    license: 'GPL-3.0-or-later',
    default_options: [
        'c_std=c11',
        'warning_level=3',
        'werror=true',
    ],
)

# Dependencies
gtk = dependency('gtk4', version: '>= 4.0.0')
layer_shell = dependency('gtk4-layer-shell-0', version: '>= 1.0.0')

# Source files
src = files(
    'src/main.c',
    'src/dock.c',
)

# Build
executable('zenith-dock',
    src,
    dependencies: [
        gtk,
        layer_shell,
    ],
    install: true,
    install_dir: get_option('bindir'),
)
