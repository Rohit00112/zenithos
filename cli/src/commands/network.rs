// Zenith OS — osctl
//
// commands/network.rs — Network management commands
//
// SPDX-License-Identifier: GPL-3.0-or-later

use anyhow::Result;
use clap::Subcommand;
use colored::Colorize;
use sysinfo::Networks;

#[derive(Subcommand)]
pub enum NetworkAction {
    /// Show network status
    Status,
}

pub fn handle(action: NetworkAction) -> Result<()> {
    match action {
        NetworkAction::Status => show_status(),
    }
}

fn show_status() -> Result<()> {
    let networks = Networks::new_with_refreshed_list();

    println!();
    println!(
        "  {} {}",
        "⬢".bright_blue(),
        "Network Status".bold().bright_white()
    );
    println!("  {}", "─".repeat(50).dimmed());

    for (name, data) in &networks {
        let rx = data.total_received();
        let tx = data.total_transmitted();

        println!("  {} {}", "●".bright_green(), name.bold().bright_white());
        println!(
            "    {} {}  {} {}",
            "↓".dimmed(),
            format_bytes(rx),
            "↑".dimmed(),
            format_bytes(tx)
        );
    }

    if networks.list().is_empty() {
        println!(
            "  {} {}",
            "✗".bright_red(),
            "No network interfaces found".dimmed()
        );
    }

    println!();
    Ok(())
}

fn format_bytes(bytes: u64) -> String {
    const GB: u64 = 1024 * 1024 * 1024;
    const MB: u64 = 1024 * 1024;
    const KB: u64 = 1024;

    if bytes >= GB {
        format!("{:.1} GB", bytes as f64 / GB as f64)
    } else if bytes >= MB {
        format!("{:.1} MB", bytes as f64 / MB as f64)
    } else if bytes >= KB {
        format!("{:.1} KB", bytes as f64 / KB as f64)
    } else {
        format!("{} B", bytes)
    }
}
