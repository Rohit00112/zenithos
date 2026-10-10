project('zenith-notifications', 'c',
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
gio = dependency('gio-2.0', version: '>= 2.70')

# Source files
src = files(
    'src/main.c',
    'src/notification.c',
    'src/popup.c',
)

# Build
executable('zenith-notifications',
    src,
    dependencies: [
        gtk4,
        gtk4_layer_shell,
        gio,
    ],
    install: true,
    install_dir: get_option('bindir'),
)

# D-Bus service file
dbus_conf = configuration_data()
dbus_conf.set('bindir', get_option('prefix') / get_option('bindir'))

configure_file(
    input: 'data/org.freedesktop.Notifications.service.in',
    output: 'org.freedesktop.Notifications.service',
    configuration: dbus_conf,
    install: true,
    install_dir: get_option('datadir') / 'dbus-1' / 'services',
)
