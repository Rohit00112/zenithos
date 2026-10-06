project('zenith-session', 'c',
    version: '0.1.0',
    license: 'GPL-3.0-or-later',
    default_options: [
        'c_std=c11',
        'warning_level=3',
        'werror=true',
    ],
)

src = files(
    'src/main.c',
    'src/session.c',
)

executable('zenith-session',
    src,
    install: true,
    install_dir: get_option('bindir'),
)
