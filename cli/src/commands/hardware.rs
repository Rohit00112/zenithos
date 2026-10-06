// Zenith OS — osctl
//
// commands/hardware.rs — Hardware information commands
//
// SPDX-License-Identifier: GPL-3.0-or-later

use anyhow::Result;
use clap::Subcommand;
use colored::Colorize;
use sysinfo::{Components, Disks, System};

#[derive(Subcommand)]
pub enum HardwareAction {
    /// Show hardware information
    Info,
}

pub fn handle(action: HardwareAction) -> Result<()> {
    match action {
        HardwareAction::Info => show_info(),
    }
}

fn show_info() -> Result<()> {
    let mut sys = System::new_all();
    sys.refresh_all();

    println!();
    println!(
        "  {} {}",
        "⬢".bright_blue(),
        "Hardware Information".bold().bright_white()
    );
    println!("  {}", "─".repeat(50).dimmed());

    // CPU
    println!();
    println!("  {}", "CPU".bold());
    if !sys.cpus().is_empty() {
        println!(
            "    {} {}",
            "Model:".dimmed(),
            sys.cpus()[0].brand().bright_white()
        );
        println!(
            "    {} {}",
            "Cores:".dimmed(),
            sys.cpus().len().to_string().bright_white()
        );
        println!(
            "    {} {} MHz",
            "Frequency:".dimmed(),
            sys.cpus()[0].frequency().to_string().bright_white()
        );
    }

    // Memory
    println!();
    println!("  {}", "Memory".bold());
    println!(
        "    {} {}",
        "Total:".dimmed(),
        format_bytes(sys.total_memory()).bright_white()
    );
    println!(
        "    {} {}",
        "Used:".dimmed(),
        format_bytes(sys.used_memory()).bright_white()
    );
    println!(
        "    {} {}",
        "Available:".dimmed(),
        format_bytes(sys.total_memory() - sys.used_memory()).bright_white()
    );

    // Disks
    let disks = Disks::new_with_refreshed_list();
    println!();
    println!("  {}", "Storage".bold());
    for disk in disks.list() {
        let mount = disk.mount_point().display();
        let total = disk.total_space();
        let avail = disk.available_space();
        let used = total - avail;
        let fs = disk.file_system().to_string_lossy();

        println!(
            "    {} {} ({}) — {} / {} used",
            "●".bright_blue(),
            disk.name().to_string_lossy().bright_white(),
            mount,
            format_bytes(used),
            format_bytes(total)
        );
        println!("      {} {}", "Filesystem:".dimmed(), fs);
    }

    // Sensors / Temperatures
    let components = Components::new_with_refreshed_list();
    if !components.list().is_empty() {
        println!();
        println!("  {}", "Sensors".bold());
        for comp in components.list() {
            println!(
                "    {} {} — {:.1}°C",
                "🌡".dimmed(),
                comp.label().bright_white(),
                comp.temperature()
            );
        }
    }

    println!();
    Ok(())
}

fn format_bytes(bytes: u64) -> String {
    const GB: u64 = 1024 * 1024 * 1024;
    const MB: u64 = 1024 * 1024;

    if bytes >= GB {
        format!("{:.1} GB", bytes as f64 / GB as f64)
    } else {
        format!("{:.0} MB", bytes as f64 / MB as f64)
    }
}
