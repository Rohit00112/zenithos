// Zenith OS — osctl
//
// commands/package.rs — Package management (stub)
//
// SPDX-License-Identifier: GPL-3.0-or-later

use anyhow::Result;
use clap::Subcommand;
use colored::Colorize;

#[derive(Subcommand)]
pub enum PackageAction {
    /// Search for packages
    Search { query: String },
    /// Install a package
    Install { name: String },
    /// Remove a package
    Remove { name: String },
    /// Update all packages
    Update,
}

pub fn handle(action: PackageAction) -> Result<()> {
    let msg = match action {
        PackageAction::Search { .. } => "package search",
        PackageAction::Install { .. } => "package install",
        PackageAction::Remove { .. } => "package remove",
        PackageAction::Update => "package update",
    };
    println!();
    println!("  {} {} is not implemented yet.", "⚠".bright_yellow(), msg);
    println!("  This feature will be available in Phase 4.");
    println!();
    Ok(())
}
