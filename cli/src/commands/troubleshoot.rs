// Zenith OS — osctl
//
// commands/troubleshoot.rs — System troubleshooting
//
// Collects system diagnostics: failed services, disk space,
// memory pressure, recent kernel messages.
//
// SPDX-License-Identifier: GPL-3.0-or-later

use anyhow::Result;
use colored::Colorize;
use std::process::Command;
use sysinfo::{System, Disks};

pub fn handle() -> Result<()> {
    let mut sys = System::new_all();
    sys.refresh_all();

    println!();
    println!("  {} {}", "⬢".bright_blue(), "System Diagnostics".bold().bright_white());
    println!("  {}", "─".repeat(60).dimmed());

    let mut issues = 0;

    // Memory pressure
    let total = sys.total_memory();
    let used = sys.used_memory();
    let mem_pct = if total > 0 { used * 100 / total } else { 0 };
    if mem_pct >= 90 {
        println!("  {} Memory critically high: {}% used", "✗".bright_red(), mem_pct);
        issues += 1;
    } else if mem_pct >= 75 {
        println!("  {} Memory high: {}% used", "⚠".bright_yellow(), mem_pct);
        issues += 1;
    }

    // Swap pressure
    let total_swap = sys.total_swap();
    let used_swap = sys.used_swap();
    if total_swap > 0 {
        let swap_pct = used_swap * 100 / total_swap;
        if swap_pct >= 80 {
            println!("  {} Swap critically high: {}% used", "✗".bright_red(), swap_pct);
            issues += 1;
        }
    }

    // Disk space
    let disks = Disks::new_with_refreshed_list();
    for disk in disks.list() {
        let total = disk.total_space();
        if total == 0 {
            continue;
        }
        let avail = disk.available_space();
        let used_pct = (total - avail) * 100 / total;
        let mount = disk.mount_point().display().to_string();

        if used_pct >= 95 {
            println!("  {} Disk {} is critically full: {}% used",
                "✗".bright_red(), mount, used_pct);
            issues += 1;
        } else if used_pct >= 85 {
            println!("  {} Disk {} is nearly full: {}% used",
                "⚠".bright_yellow(), mount, used_pct);
            issues += 1;
        }
    }

    // Failed systemd services
    if let Ok(output) = Command::new("systemctl")
        .args(["--failed", "--plain", "--no-legend", "--no-pager"])
        .output()
    {
        let stdout = String::from_utf8_lossy(&output.stdout);
        let failed: Vec<&str> = stdout.lines()
            .filter(|l| !l.is_empty())
            .collect();

        if !failed.is_empty() {
            println!("  {} Failed services:", "✗".bright_red());
            for line in failed {
                let name = line.split_whitespace().next().unwrap_or(line);
                println!("      {}", name.bright_white());
                issues += 1;
            }
        }
    }

    // Recent kernel errors
    if let Ok(output) = Command::new("journalctl")
        .args(["-k", "-p", "err", "-n", "5", "--no-pager", "--plain"])
        .output()
    {
        let stdout = String::from_utf8_lossy(&output.stdout);
        let lines: Vec<&str> = stdout.lines()
            .filter(|l| !l.is_empty())
            .collect();

        if !lines.is_empty() {
            println!("  {} Recent kernel errors:", "⚠".bright_yellow());
            for line in lines {
                println!("      {}", line.dimmed());
            }
            issues += 1;
        }
    }

    println!("  {}", "─".repeat(60).dimmed());

    if issues == 0 {
        println!("  {} No issues detected", "✓".bright_green());
    } else {
        println!("  {} {} issue(s) found", "⚠".bright_yellow(), issues);
    }

    println!();
    Ok(())
}
