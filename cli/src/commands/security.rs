// Zenith OS — osctl
//
// commands/security.rs — Security management (stub)
//
// SPDX-License-Identifier: GPL-3.0-or-later

use anyhow::Result;
use clap::Subcommand;
use colored::Colorize;

#[derive(Subcommand)]
pub enum SecurityAction {
    /// Show security status
    Status,
    /// Run a security scan
    Scan,
}

pub fn handle(action: SecurityAction) -> Result<()> {
    let msg = match action {
        SecurityAction::Status => "security status",
        SecurityAction::Scan => "security scan",
    };
    println!();
    println!("  {} {} is not implemented yet.", "⚠".bright_yellow(), msg);
    println!("  This feature will be available in Phase 5.");
    println!();
    Ok(())
}
