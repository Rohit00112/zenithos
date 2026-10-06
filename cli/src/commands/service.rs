// Zenith OS — osctl
//
// commands/service.rs — Service management via systemctl
//
// SPDX-License-Identifier: GPL-3.0-or-later

use anyhow::{Context, Result};
use clap::Subcommand;
use colored::Colorize;
use std::process::Command;

#[derive(Subcommand)]
pub enum ServiceAction {
    /// List all services and their status
    List,
    /// Start a service
    Start { name: String },
    /// Stop a service
    Stop { name: String },
    /// Restart a service
    Restart { name: String },
    /// Show service logs
    Logs { name: String },
}

pub fn handle(action: ServiceAction) -> Result<()> {
    match action {
        ServiceAction::List => list_services(),
        ServiceAction::Start { name } => control_service("start", &name),
        ServiceAction::Stop { name } => control_service("stop", &name),
        ServiceAction::Restart { name } => control_service("restart", &name),
        ServiceAction::Logs { name } => show_logs(&name),
    }
}

fn list_services() -> Result<()> {
    let output = Command::new("systemctl")
        .args([
            "list-units",
            "--type=service",
            "--no-pager",
            "--plain",
            "--no-legend",
        ])
        .output()
        .context("Failed to run systemctl — is this a systemd system?")?;

    let stdout = String::from_utf8_lossy(&output.stdout);

    println!();
    println!(
        "  {} {}",
        "⬢".bright_blue(),
        "Services".bold().bright_white()
    );
    println!("  {}", "─".repeat(60).dimmed());
    println!(
        "  {:<40} {:<10} {}",
        "NAME".dimmed(),
        "STATE".dimmed(),
        "DESCRIPTION".dimmed()
    );
    println!("  {}", "─".repeat(60).dimmed());

    for line in stdout.lines().take(30) {
        let parts: Vec<&str> = line.split_whitespace().collect();
        if parts.len() < 4 {
            continue;
        }
        let name = parts[0].trim_end_matches(".service");
        let active = parts[2];
        let desc = parts[4..].join(" ");

        let state_colored = match active {
            "running" => active.bright_green(),
            "failed" => active.bright_red(),
            "exited" => active.dimmed(),
            _ => active.bright_yellow(),
        };

        println!(
            "  {:<40} {:<10} {}",
            name.bright_white(),
            state_colored,
            desc.dimmed()
        );
    }

    println!();
    Ok(())
}

fn control_service(action: &str, name: &str) -> Result<()> {
    println!();
    println!(
        "  {} {} {}...",
        "→".bright_blue(),
        action,
        name.bright_white()
    );

    let output = Command::new("systemctl")
        .args([action, name])
        .output()
        .context("Failed to run systemctl")?;

    if output.status.success() {
        println!(
            "  {} {} {}",
            "✓".bright_green(),
            name.bright_white(),
            format!("{action}ed").dimmed()
        );
    } else {
        let stderr = String::from_utf8_lossy(&output.stderr);
        println!(
            "  {} Failed to {} {}",
            "✗".bright_red(),
            action,
            name.bright_white()
        );
        if !stderr.is_empty() {
            println!("  {}", stderr.trim().dimmed());
        }
    }

    println!();
    Ok(())
}

fn show_logs(name: &str) -> Result<()> {
    let output = Command::new("journalctl")
        .args(["-u", name, "-n", "50", "--no-pager"])
        .output()
        .context("Failed to run journalctl")?;

    println!();
    println!(
        "  {} {} {}",
        "⬢".bright_blue(),
        "Logs:".dimmed(),
        name.bold().bright_white()
    );
    println!("  {}", "─".repeat(60).dimmed());

    let stdout = String::from_utf8_lossy(&output.stdout);
    for line in stdout.lines() {
        println!("  {}", line);
    }

    println!();
    Ok(())
}
