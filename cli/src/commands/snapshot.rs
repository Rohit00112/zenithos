// Zenith OS — osctl
//
// commands/snapshot.rs — Snapshot management (stub)
//
// SPDX-License-Identifier: GPL-3.0-or-later

use anyhow::Result;
use clap::Subcommand;
use colored::Colorize;

#[derive(Subcommand)]
pub enum SnapshotAction {
    /// List snapshots
    List,
    /// Create a new snapshot
    Create {
        /// Description for the snapshot
        #[arg(short, long)]
        description: Option<String>,
    },
    /// Restore a snapshot
    Restore { id: String },
    /// Delete a snapshot
    Delete { id: String },
}

pub fn handle(action: SnapshotAction) -> Result<()> {
    let msg = match action {
        SnapshotAction::List => "snapshot list",
        SnapshotAction::Create { .. } => "snapshot create",
        SnapshotAction::Restore { .. } => "snapshot restore",
        SnapshotAction::Delete { .. } => "snapshot delete",
    };
    println!();
    println!("  {} {} is not implemented yet.", "⚠".bright_yellow(), msg);
    println!("  This feature will be available in Phase 4.");
    println!();
    Ok(())
}
