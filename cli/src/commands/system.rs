// Zenith OS — osctl
//
// commands/system.rs — System management commands
//
// Implements:
//   osctl system status   — Show system overview
//   osctl system update   — Update the system (placeholder)
//   osctl system rollback — Rollback to previous state (placeholder)
//
// SPDX-License-Identifier: GPL-3.0-or-later

use anyhow::Result;
use chrono::Local;
use clap::Subcommand;
use colored::Colorize;
use std::fs;
use sysinfo::System;

#[derive(Subcommand)]
pub enum SystemAction {
    /// Show system status and information
    Status,

    /// Update the system
    Update,

    /// Rollback to previous system state
    Rollback,
}

pub fn handle(action: SystemAction) -> Result<()> {
    match action {
        SystemAction::Status => show_status(),
        SystemAction::Update => show_not_implemented("system update"),
        SystemAction::Rollback => show_not_implemented("system rollback"),
    }
}

fn show_status() -> Result<()> {
    let mut sys = System::new_all();
    sys.refresh_all();

    // Header
    println!();
    println!(
        "  {} {}",
        "⬢".bright_blue(),
        "Zenith OS".bold().bright_white()
    );
    println!("  {}", "─".repeat(40).dimmed());

    // Hostname
    let hostname = System::host_name().unwrap_or_else(|| "unknown".to_string());
    println!("  {}  {}", "Hostname:".dimmed(), hostname.bright_white());

    // Kernel
    let kernel = System::kernel_version().unwrap_or_else(|| "unknown".to_string());
    println!("  {}    {}", "Kernel:".dimmed(), kernel.bright_white());

    // OS
    let os_name = System::long_os_version().unwrap_or_else(|| "Zenith OS".to_string());
    println!("  {}        {}", "OS:".dimmed(), os_name.bright_white());

    // Uptime
    let uptime_secs = System::uptime();
    let hours = uptime_secs / 3600;
    let minutes = (uptime_secs % 3600) / 60;
    let uptime_str = if hours > 0 {
        format!("{}h {}m", hours, minutes)
    } else {
        format!("{}m", minutes)
    };
    println!("  {}    {}", "Uptime:".dimmed(), uptime_str.bright_white());

    // Date/time
    let now = Local::now();
    println!(
        "  {}      {}",
        "Time:".dimmed(),
        now.format("%Y-%m-%d %H:%M:%S").to_string().bright_white()
    );

    println!();
    println!("  {}", "─".repeat(40).dimmed());

    // CPU
    let cpu_count = sys.cpus().len();
    let cpu_name = if !sys.cpus().is_empty() {
        sys.cpus()[0].brand().to_string()
    } else {
        "unknown".to_string()
    };
    println!(
        "  {}       {} ({})",
        "CPU:".dimmed(),
        cpu_name.bright_white(),
        format!("{} cores", cpu_count).dimmed()
    );

    // Memory
    let total_mem = sys.total_memory();
    let used_mem = sys.used_memory();
    let mem_percent = if total_mem > 0 {
        (used_mem as f64 / total_mem as f64 * 100.0) as u64
    } else {
        0
    };
    println!(
        "  {}    {} / {} ({}%)",
        "Memory:".dimmed(),
        format_bytes(used_mem).bright_white(),
        format_bytes(total_mem).dimmed(),
        colorize_percent(mem_percent)
    );

    // Swap
    let total_swap = sys.total_swap();
    let used_swap = sys.used_swap();
    if total_swap > 0 {
        let swap_percent = (used_swap as f64 / total_swap as f64 * 100.0) as u64;
        println!(
            "  {}      {} / {} ({}%)",
            "Swap:".dimmed(),
            format_bytes(used_swap).bright_white(),
            format_bytes(total_swap).dimmed(),
            colorize_percent(swap_percent)
        );
    }

    // Load average
    let load = System::load_average();
    println!(
        "  {}      {:.2} {:.2} {:.2}",
        "Load:".dimmed(),
        load.one,
        load.five,
        load.fifteen
    );

    // Disk usage (root)
    let disks = sysinfo::Disks::new_with_refreshed_list();
    for disk in disks.list() {
        if disk.mount_point() == std::path::Path::new("/") {
            let total = disk.total_space();
            let avail = disk.available_space();
            let used = total - avail;
            let percent = if total > 0 {
                (used as f64 / total as f64 * 100.0) as u64
            } else {
                0
            };
            println!(
                "  {}  {} / {} ({}%) [{}]",
                "Disk (/):".dimmed(),
                format_bytes(used).bright_white(),
                format_bytes(total).dimmed(),
                colorize_percent(percent),
                disk.file_system().to_string_lossy().dimmed()
            );
            break;
        }
    }

    // Processes
    let proc_count = sys.processes().len();
    println!(
        "  {} {}",
        "Processes:".dimmed(),
        proc_count.to_string().bright_white()
    );

    println!();

    // Status indicators
    println!("  {}", "Services".bold());
    print_status(
        "Compositor",
        check_process_running(&sys, "zenith-compositor"),
    );
    print_status("Panel", check_process_running(&sys, "zenith-panel"));
    print_status(
        "NetworkManager",
        check_process_running(&sys, "NetworkManager"),
    );
    print_status("PipeWire", check_process_running(&sys, "pipewire"));

    println!();

    Ok(())
}

fn check_process_running(sys: &System, name: &str) -> bool {
    sys.processes()
        .values()
        .any(|p| p.name().to_string_lossy().contains(name))
}

fn print_status(name: &str, running: bool) {
    if running {
        println!("  {} {} {}", "✓".bright_green(), name, "running".dimmed());
    } else {
        println!("  {} {} {}", "✗".bright_red(), name, "not running".dimmed());
    }
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

fn colorize_percent(percent: u64) -> colored::ColoredString {
    let s = percent.to_string();
    if percent >= 90 {
        s.bright_red()
    } else if percent >= 70 {
        s.bright_yellow()
    } else {
        s.bright_green()
    }
}

fn show_not_implemented(feature: &str) -> Result<()> {
    println!();
    println!(
        "  {} {} is not implemented yet.",
        "⚠".bright_yellow(),
        feature
    );
    println!("  This feature will be available in a future phase.");
    println!();
    Ok(())
}
