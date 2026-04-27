# Android Frontend Design Optimization Plan

## Goal

Use the `codex-design` standard to move the Android-first QML frontend from
basic visual polish toward a consistent product UI. The focus is typography,
color, border radius, component consistency, mobile density, and screenshot-led
iteration.

## Design Direction

- Product type: Android-first daily-use meal decision and logging tool.
- Tone: practical, calm, information-dense, and easy to scan.
- Avoid: marketing-style hero sections, oversized decorative cards, one-note
  beige/brown palettes, large rounded card stacks, and mixed system/default
  controls beside custom controls.
- Keep: local-first, tool-like workflows where recommendation, feedback, dish
  import, and management actions are immediately reachable.

## Current Observations

- The app already has a clearer green/neutral direction after the latest QML
  polish pass.
- Colors, radii, and font sizes are still spread across `Main.qml`,
  `FoodPage.qml`, `MealLogPage.qml`, and `SchedulePage.qml`.
- `ReadableButton` is partly unified, but many `TextField`, `TextArea`,
  `ComboBox`, `CheckBox`, and `SpinBox` instances still rely on Qt default
  visuals.
- Management pages are complete but still read as stacked forms with many
  explanatory cards.
- Android runtime screenshots are still missing because no emulator/device was
  available in the last session.

## Plan

### 1. Establish the Design Baseline

- Inventory current QML colors, font sizes, border radii, border colors, and
  spacing values.
- Define one token set for:
  - primary action color
  - neutral background and surface colors
  - text primary / secondary / muted colors
  - border and focus colors
  - success / warning / error colors
  - card radius, content radius, and control radius
- Target defaults:
  - large card radius: `16`
  - small card radius: `12`
  - button/input radius: `10-12`
  - page title: `24-26`
  - section title: `19-21`
  - body text: `15`
  - label text: `13`
  - mobile touch target: at least `44` where practical

### 2. Extract Shared Style Components

- Add a shared QML style layer such as `Theme.qml` and/or reusable styled
  controls.
- Prefer central components over repeated inline color/radius declarations.
- Candidate components:
  - `StyledButton`
  - `StyledTextField`
  - `StyledTextArea`
  - `StyledComboBox`
  - `SectionCard`
  - `InfoChip`
  - `StatusPill`
- Keep the first pass QML-only unless component extraction exposes a real C++
  integration need.

### 3. Refine the Main Screen

- Make the bottom composer feel like the primary work surface, not just another
  card.
- Make `推荐 / 反馈 / 菜品 / 日常` read as a coherent segmented control.
- Keep the main action button visually dominant, with secondary buttons quieter.
- Check long Chinese placeholder text at narrow widths.
- Verify no important text is clipped at `340px` and `420px`.

### 4. Unify the Management Drawer

- Apply the shared components across:
  - LLM debug
  - schedule management
  - food management
  - feedback
  - meal records
- Reduce visual mismatch between custom buttons and default Qt controls.
- Reduce nested-card heaviness where a simple section heading and surface is
  enough.
- Keep drawer width rules practical for Android and desktop verification.

### 5. Improve Information Density

- Shorten repeated explanatory copy.
- Separate description text from action rows.
- Give long pages a clearer structure:
  - summary / filters
  - primary form or action area
  - results or recent items
- Prefer scan-friendly labels and compact helper text over full paragraphs in
  every card.

### 6. Converge Color Semantics

- Green: primary actions, success, positive states.
- Amber / warm neutral: food context, light hints, page background.
- Blue-gray: records, system information, comparison details.
- Red: errors, risk, warnings.
- Remove rogue brown/blue/green variants that are not part of the final token
  set.

### 7. Screenshot-Led Iteration

- Desktop narrow-view screenshots:
  - `340px`
  - `420px`
  - `700px`
- Capture and compare:
  - main recommendation page
  - composer modes
  - drawer LLM debug
  - schedule page
  - food page
  - meal log / feedback page
- Iterate in small passes:
  1. typography
  2. colors
  3. radii and spacing
  4. density and text overflow

### 8. Android Runtime Validation

- Build the Android APK after each meaningful pass.
- If a device or AVD is available:
  - install the APK
  - capture screenshots through `adb`
  - check touch target sizing
  - check keyboard behavior for composer and forms
  - check status/navigation bar interactions
  - check drawer open/close behavior
- If no device or AVD is available, state that Android runtime validation is
  still pending and do not claim it passed.

## Suggested Implementation Rounds

### Round 1

- Add shared theme tokens.
- Extract the highest-impact styled controls.
- Refine the main screen and composer.
- Run desktop build and narrow screenshot checks.

### Round 2

- Apply shared controls to drawer and management pages.
- Reduce duplicated colors and large-radius cards.
- Rebuild APK.
- Run Android screenshot validation if a device or AVD is available.

## Validation Commands

Desktop build:

```powershell
& 'C:\Qt\Tools\CMake_64\bin\cmake.exe' --build build\desktop-debug --target MealAdvisor MealAdvisorValidation
```

Validation:

```powershell
& '.\build\desktop-debug\MealAdvisorValidation.exe'
```

Android APK build:

```powershell
& 'C:\Qt\Tools\CMake_64\bin\cmake.exe' --build build\android-arm64-debug --target MealAdvisor_make_apk --config Debug
```

Android device check:

```powershell
& "$env:LOCALAPPDATA\Android\Sdk\platform-tools\adb.exe" devices -l
```

## Boundaries

- Keep changes focused on QML frontend styling and layout unless a real runtime
  bug requires C++ changes.
- Do not rewrite the recommendation engine as part of frontend polish.
- Do not add new LLM capabilities, OCR/image parsing, cloud sync, or direct LLM
  database writes.
- Do not claim Android runtime validation passed without a real device/emulator
  session.
