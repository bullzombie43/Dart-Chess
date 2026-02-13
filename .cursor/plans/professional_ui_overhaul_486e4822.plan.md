---
name: Professional UI Overhaul
overview: Introduce a single app-wide theme (light and dark), replace all hardcoded green/grey with theme-based surfaces, and polish navigation, control panels, and components for a professional, iOS-inspired look while keeping the current popup-based navigation.
todos:
  - id: theme
    content: Add app theme (light + dark ThemeData, ColorScheme, CardTheme, buttons, inputs, AppBar). Remove green backgroundColor from all five scaffold views. Verify all screens use theme background.
    status: completed
  - id: navigation
    content: Update AppBar and popup menu to use theme colors; add theme-mode (light/dark) toggle in AppBar or popup.
    status: completed
  - id: board-controls
    content: Replace grey panel in BoardControls with Card/theme surface; restyle timers (theme-based chips) and Start Match/New Game buttons.
    status: completed
  - id: cards-and-text
    content: Ensure Testing/Replay/Match views use CardTheme and textTheme; remove any remaining hardcoded colors in cards.
    status: completed
  - id: components
    content: Theme EngineDropdownButton (no hardcoded black); theme game-over dialogs and SnackBarTheme in app theme.
    status: completed
  - id: polish
    content: Final spacing, radii, and tweaks; test in both light and dark.
    status: completed
isProject: false
---

# Professional UI and Design Overhaul

## Current state (summary)

- **Scaffold backgrounds**: Mint green `Color(140, 208, 161)` hardcoded in [lib/ui/single_match_view.dart](lib/ui/single_match_view.dart), [lib/ui/game_view.dart](lib/ui/game_view.dart), [lib/ui/match_view.dart](lib/ui/match_view.dart), [lib/ui/testing_view.dart](lib/ui/testing_view.dart), [lib/ui/log_replay_view.dart](lib/ui/log_replay_view.dart).
- **Control panel**: [lib/ui/board_controls.dart](lib/ui/board_controls.dart) uses a flat `Container(color: Colors.grey)` and hardcoded black/white timer boxes.
- **App theme**: [lib/main.dart](lib/main.dart) uses `ThemeData(colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepPurple), useMaterial3: true)` but views override with the green background.
- **Navigation**: [lib/main.dart](lib/main.dart) `HomePage` uses an AppBar with a `PopupMenuButton` (gear icon) to switch between Single Match, Testing, Replay Logs.
- **Cards**: Testing/Replay/Match views use default Material `Card` on the green background; no shared card styling.
- **Components**: [lib/ui/engine_dropdown_button.dart](lib/ui/engine_dropdown_button.dart) uses black underline and black text; dialogs and SnackBars are default Material.

## Current UI (from screenshots)

- **Single Match**: Pastel green canvas; left = brown/tan chessboard with flat 2D pieces; right = **solid flat grey panel** with “Engine” (black rounded timer, white text), oval **lavender “Start Match”** button, “Human” (white rounded timer, black text). High-contrast black/white timer blocks and basic person/computer icons.
- **Replay Logs**: Same green; left = board; right = **three stacked rounded panels** (light purple-grey from theme): PGN Log Selection (dropdown + Browse), Replay Controls (Play / Restart, speed slider with purple thumb), Replay Info. Cards have soft shadows and sit on the green.
- **Testing**: Same green; right = **three stacked panels** (light lavender/off-white): Engine Selection (White/Black dropdowns), **Start Test** button, Statistics (Status: Idle, Total Games 0, etc.). Same Card style as Replay.

Takeaways for implementation: (1) Single Match’s grey panel and stark black/white timers are the biggest “homebrew” cues—replace with a Card-style panel and theme-based timer chips. (2) Replay/Testing Cards already read as “boxes on top”; theme them explicitly so they use neutral surface colors and consistent elevation. (3) Primary buttons (Start Match, Start Test, Play) currently get a lavender tint from the purple seed—define a clear primary in the new theme for a more intentional look.

## Design direction (iOS-inspired, cross-platform)

- **Surfaces**: Neutral backgrounds (e.g. light grey/off-white in light, dark grey in dark). Side panels as elevated “cards” with consistent radius (e.g. 12–16) and subtle shadow/border.
- **Typography**: Single font family (e.g. system-like or Google Fonts such as Inter / Plus Jakarta Sans), clear hierarchy (title, body, caption).
- **Color**: Single source of truth via `ThemeData` and `ColorScheme`; accent for primary actions and key UI; no per-screen hardcoded colors.
- **Light + dark**: Two `ThemeData` (light/dark) and respect `MediaQuery.platformBrightnessOf(context)` or a user toggle that overrides it.

---

## 1. App-wide theme (light and dark)

**File:** [lib/main.dart](lib/main.dart) (and optionally a new `lib/ui/theme/app_theme.dart`).

- Define a **light** and a **dark** `ColorScheme` (or use `ColorScheme.fromSeed(..., brightness: Brightness.light/dark)` with a chosen seed and ensure good contrast).
- Set **surface** colors: e.g. light = off-white or very light grey for scaffold; dark = dark grey (not pure black). Use these for `scaffoldBackgroundColor` and for panel backgrounds.
- Set **primary** (and optional secondary) for buttons, links, selected state; ensure they work on both backgrounds.
- In `MaterialApp`, assign:
  - `theme: ThemeData(... light theme ...)`
  - `darkTheme: ThemeData(... dark theme ...)`
  - `themeMode: ThemeMode.system` (or drive from a simple “use dark mode” toggle stored in memory or preference).
- In the same `ThemeData`, set:
  - **Typography**: e.g. `textTheme` from a chosen font (add `google_fonts` if you want Inter/Plus Jakarta Sans, or use default with adjusted sizes/weights).
  - **CardTheme**: `elevation`, `shape` (e.g. `RoundedRectangleBorder(borderRadius: 12)`), `color`/`surfaceContainerHigh` so Cards use theme surface.
  - **ElevatedButtonTheme**: shape (rounded corners), padding, text style.
  - **InputDecorationTheme**: rounded borders, label style, so dropdowns and text fields look consistent.
  - **AppBarTheme**: background and foreground colors from theme; optional transparent or blended look.

Remove any `backgroundColor` overrides on `Scaffold` in the five view files above and rely on `Theme.of(context).scaffoldBackgroundColor` (default once theme is set).

**Optional:** Extract theme into `lib/ui/theme/app_theme.dart` (e.g. `static ThemeData get light`, `static ThemeData get dark`) and reference from `main.dart` to keep `main.dart` shorter.

---

## 2. Navigation (keep popup, polish it)

**File:** [lib/main.dart](lib/main.dart) (`HomePage`).

- Keep the **PopupMenuButton** as the main navigation, but:
  - Use **theme colors** for AppBar (background, title text, icon) so it fits light/dark.
  - Style the **popup menu**: ensure menu items use theme text/icon colors; consider slightly larger tap targets and consistent icon + label alignment (already present). Optionally use a **Card** or custom shape for the menu so it looks like a floating panel (rounded, subtle shadow) rather than the default system popup.
  - If you add a **theme toggle** (light/dark), place it in the AppBar (e.g. icon button) or inside the same popup so navigation and theme stay in one place.

No need to add tabs or rail; the plan is to make the existing menu visually consistent with the new theme.

---

## 3. Control panels and “boxes” (replace green and grey)

**Files:** [lib/ui/board_controls.dart](lib/ui/board_controls.dart), [lib/ui/single_match_view.dart](lib/ui/single_match_view.dart), [lib/ui/testing_view.dart](lib/ui/testing_view.dart), [lib/ui/log_replay_view.dart](lib/ui/log_replay_view.dart), [lib/ui/match_view.dart](lib/ui/match_view.dart).

- **Scaffold**: Remove `backgroundColor: Color.fromARGB(255, 140, 208, 161)` from all five views. Use default (theme) scaffold background.
- **Board controls panel** (right side in Single Match):
  - Replace the grey `Container` with a **Card** (or a `Container` with `decoration: BoxDecoration(..., color: Theme.of(context).colorScheme.surfaceContainerHigh)` and rounded corners). This gives a clear “panel” that fits light and dark.
  - **Timers**: Replace solid black/white boxes with theme-aware surfaces: e.g. use `colorScheme.surfaceContainerHigh` or `surface` with a border or light fill, and use `colorScheme.onSurface` / `onSurfaceVariant` for text; for “black/white” identity keep a subtle indicator (e.g. small icon or label) rather than full black/white blocks.
  - **Buttons** (“Start Match”, “New Game”): Use `ElevatedButton` with theme (no hardcoded colors); ensure they use the new `ElevatedButtonTheme` from step 1.
  - **Player selectors**: Use theme text and icon colors.
- **Testing / Replay / Match views**: They already use `Card` for sections. After step 1, Cards will pick up `CardTheme` (elevation, shape, color). Ensure no hardcoded colors inside those cards (e.g. text should use `Theme.of(context).textTheme` or `colorScheme.onSurface`). Adjust padding/spacing if needed for a more consistent “panel” look.

Result: No green or grey boxes; everything is driven by the app theme and looks like a single, cohesive layout.

---

## 4. Shared components (dropdowns, dialogs, SnackBars)

**Files:** [lib/ui/engine_dropdown_button.dart](lib/ui/engine_dropdown_button.dart), game-over dialogs in [lib/ui/single_match_view.dart](lib/ui/single_match_view.dart) and [lib/ui/game_view.dart](lib/ui/game_view.dart).

- **EngineDropdownButton**: Remove hardcoded `Colors.black` for icon and underline. Use `Theme.of(context).colorScheme.onSurface` (and `onSurfaceVariant` if needed) and theme-based `InputDecoration` if you switch to `DropdownButtonFormField` or style the underline from theme. Prefer theme `borderRadius` and colors so it fits light and dark.
- **Game-over dialog**: Replace raw `AlertDialog` styling (if any) with theme: use `theme.colorScheme` and `theme.textTheme` for title, content, and buttons. Optionally use a rounded shape consistent with `CardTheme` so the dialog feels part of the app.
- **SnackBars**: Use `SnackBarThemeData` in the app theme (background, content text style, behavior) so all `ScaffoldMessenger.showSnackBar` calls look consistent without changing call sites.

---

## 5. Board and window chrome (optional refinements)

- **Board**: [lib/ui/board_background.dart](lib/ui/board_background.dart) already has `BoardTheme` (brown, blueGrey, etc.). Optionally add a theme that matches the new app palette (e.g. light grey squares for light mode) or keep current default; no requirement to change board logic.
- **Window**: [lib/main.dart](lib/main.dart) uses `window_manager` with `backgroundColor: Colors.transparent` and `TitleBarStyle.hidden`. Ensure the new scaffold background works with this (no visual glitches). If you add a title bar later, style it from theme.

---

## 6. Implementation todos

1. **Theme** – Add app theme (light + dark `ThemeData`, `ColorScheme`, `CardTheme`, button/input/AppBar themes). Remove `backgroundColor` from all five scaffold views. Verify all screens use theme background.
2. **Navigation** – Update AppBar and popup menu to use theme colors; add theme-mode (light/dark) toggle in AppBar or popup.
3. **BoardControls** – Replace grey panel with Card/theme surface; restyle timers (theme-based chips) and Start Match/New Game buttons.
4. **Cards and text** – Ensure Testing/Replay/Match views use `CardTheme` and `textTheme`; remove any remaining hardcoded colors in cards.
5. **Components** – Theme EngineDropdownButton (no hardcoded black); theme game-over dialogs and `SnackBarTheme` in app theme.
6. **Polish** – Final spacing, radii, and tweaks; test in both light and dark.

---

## Dependency

- **google_fonts** (optional): Add to `pubspec.yaml` if you want a specific font (e.g. Inter or Plus Jakarta Sans) for the “professional” typography. Otherwise, use Flutter’s default font with a clear `textTheme` definition in theme.

---

## Summary of files to touch


| Area                | Files                                                                                                           |
| ------------------- | --------------------------------------------------------------------------------------------------------------- |
| Theme               | `lib/main.dart`, optionally `lib/ui/theme/app_theme.dart`                                                       |
| Navigation          | `lib/main.dart` (HomePage AppBar + popup)                                                                       |
| Scaffold background | `single_match_view.dart`, `game_view.dart`, `match_view.dart`, `testing_view.dart`, `log_replay_view.dart`      |
| Control panel       | `board_controls.dart`                                                                                           |
| Cards / panels      | `testing_view.dart`, `log_replay_view.dart`, `match_view.dart` (theme-driven; minimal code if CardTheme is set) |
| Components          | `engine_dropdown_button.dart`; `single_match_view.dart`, `game_view.dart` (dialogs)                             |
| Optional            | `pubspec.yaml` (google_fonts); `board_background.dart` (optional board theme)                                   |


No change to game logic, state, or routing structure—only visual and theme updates to achieve a professional, iOS-inspired look with light and dark support and a polished popup menu.