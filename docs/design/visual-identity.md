# Zenith OS — Visual Identity

## Brand

- **Name:** Zenith OS
- **Tagline:** "Reach the peak."
- **Codename v0.1:** Summit
- **Logo concept:** Stylized mountain peak / upward chevron forming a "Z"

## Color System

### Dark Mode (Default)

| Token | Hex | Usage |
|-------|-----|-------|
| bg-primary | `#0D1117` | Main background |
| bg-secondary | `#161B22` | Panel, sidebar |
| bg-elevated | `#1C2128` | Cards, dialogs |
| text-primary | `#E6EDF3` | Primary text |
| text-secondary | `#8B949E` | Secondary text |
| accent | `#58A6FF` | Links, focus rings, active |
| accent-hover | `#79C0FF` | Accent hover state |
| success | `#3FB950` | OK, running, connected |
| warning | `#D29922` | Warnings, attention |
| danger | `#F85149` | Errors, destructive actions |
| border | `#30363D` | Borders, dividers |

### Light Mode

| Token | Hex | Usage |
|-------|-----|-------|
| bg-primary | `#FAFBFC` | Main background |
| bg-secondary | `#F0F2F5` | Panel, sidebar |
| bg-elevated | `#FFFFFF` | Cards, dialogs |
| text-primary | `#1A1D23` | Primary text |
| text-secondary | `#57606A` | Secondary text |
| accent | `#2F81F7` | Links, focus rings |
| success | `#1A7F37` | Success indicators |
| warning | `#9A6700` | Warnings |
| danger | `#CF222E` | Errors |
| border | `#D0D7DE` | Borders |

## Typography

- **UI font:** Inter (variable weight)
- **Monospace:** JetBrains Mono
- **Display:** Inter Display
- **Base size:** 14px
- **Scale ratio:** 1.25

## Design Principles

1. **Depth through shadows, not borders** — Use elevation/shadow to create hierarchy
2. **Motion with purpose** — 200ms ease-out, no gratuitous animation
3. **Information density control** — Simple mode vs Advanced mode
4. **Consistent iconography** — Outlined icons, 24px grid
5. **8px grid system** — All spacing in multiples of 8px

## Panel Design

```
┌──────────────────────────────────────────────────────┐
│ [1] [2] [3] [4]  │                    │  ⬢  🔊  🔋  14:30 │
│  workspaces       │    (center)        │    indicators      │
└──────────────────────────────────────────────────────┘
```

- Height: 32px
- Background: `bg-secondary` at 92% opacity
- Font: Inter 13px Medium
