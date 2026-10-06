// Zenith OS — osctl
//
// main.rs — Entry point for the Zenith OS CLI tool
//
// Usage:
//   osctl system status     — Show system information
//   osctl system update     — Update the system
//   osctl network status    — Show network information
//   osctl hardware info     — Show hardware information
//   osctl service list      — List services
//   osctl package search    — Search packages
//   osctl snapshot create   — Create system snapshot
//
// SPDX-License-Identifier: GPL-3.0-or-later

mod commands;

use clap::{Parser, Subcommand};

/// Zenith OS — System Management Tool
#[derive(Parser)]
#[command(
    name = "osctl",
    about = "Zenith OS system management CLI",
    version = env!("CARGO_PKG_VERSION"),
    author = "Zenith OS Project",
    propagate_version = true,
)]
struct Cli {
    #[command(subcommand)]
    command: Commands,
}

#[derive(Subcommand)]
enum Commands {
    /// System management
    System {
        #[command(subcommand)]
        action: commands::system::SystemAction,
    },

    /// Network management
    Network {
        #[command(subcommand)]
        action: commands::network::NetworkAction,
    },

    /// Hardware information
    Hardware {
        #[command(subcommand)]
        action: commands::hardware::HardwareAction,
    },

    /// Service management
    Service {
        #[command(subcommand)]
        action: commands::service::ServiceAction,
    },

    /// Package management
    Package {
        #[command(subcommand)]
        action: commands::package::PackageAction,
    },

    /// Snapshot management
    Snapshot {
        #[command(subcommand)]
        action: commands::snapshot::SnapshotAction,
    },

    /// Security management
    Security {
        #[command(subcommand)]
        action: commands::security::SecurityAction,
    },

    /// System troubleshooting
    Troubleshoot,
}

fn main() -> anyhow::Result<()> {
    let cli = Cli::parse();

    match cli.command {
        Commands::System { action } => commands::system::handle(action),
        Commands::Network { action } => commands::network::handle(action),
        Commands::Hardware { action } => commands::hardware::handle(action),
        Commands::Service { action } => commands::service::handle(action),
        Commands::Package { action } => commands::package::handle(action),
        Commands::Snapshot { action } => commands::snapshot::handle(action),
        Commands::Security { action } => commands::security::handle(action),
        Commands::Troubleshoot => commands::troubleshoot::handle(),
    }
}
