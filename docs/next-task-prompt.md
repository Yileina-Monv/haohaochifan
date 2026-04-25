# Next Task Prompt

Use the prompt below in the next new Codex window.

```text
The project path is D:\Codex\2026-04-18-qt-c-app.

Before doing work, first scan the current changed files plus these docs:
1. docs/memory.md
2. docs/next-task-prompt.md
3. docs/handoff.md

Then read the core context if needed:
4. README.md
5. docs/project-plan.md
6. docs/data-model.md
7. docs/product-rules.md
8. docs/chat-style-frontend-refactor-plan.md
9. docs/llm-supplement-rules.md
10. docs/llm-supplement-prompt.md

Task-specific files:
11. CMakeLists.txt
12. src/main.cpp
13. app/qml/Main.qml
14. app/qml/FoodPage.qml
15. app/qml/SchedulePage.qml
16. app/qml/MealLogPage.qml
17. src/recommendation/recommendationengine.h
18. src/recommendation/recommendationengine.cpp
19. tools/validation/main.cpp

Current status:
- The main page is recommendation-first, with the right Drawer handling LLM
  debug, schedule, food configuration, feedback, and feedback/history.
- Drawer `LLM 调试` has a `测试连接` button. It tests the current API Key / URL /
  Model field values without forcing a save first; blank fields still fall back
  through the existing AppConfig / environment / default chain.
- A real-device screenshot showed Drawer `LLM 调试` failing with
  `TLS initialization failed` when testing DeepSeek over HTTPS. The Android APK
  now has a targeted OpenSSL packaging fix:
  - `CMakeLists.txt` pulls pinned `KDAB/android_openssl` commit
    `b71f1470962019bd89534a2919f5925f93bc5779` during Android configure.
  - `CMakeLists.txt` calls `add_android_openssl_libraries(MealAdvisor)`.
  - `src/main.cpp` sets `ANDROID_OPENSSL_SUFFIX=_3` before Qt starts and logs
    `QSslSocket::supportsSsl()` plus SSL library versions.
  - `aapt list` confirms the APK includes
    `lib/arm64-v8a/libplugins_tls_qopensslbackend_arm64-v8a.so`,
    `lib/arm64-v8a/libssl_3.so`, and
    `lib/arm64-v8a/libcrypto_3.so`.
- All QML text inputs hide placeholder hints whenever the field is focused or
  already has text.
- FoodPage's repeated low/medium/high dish attributes remain labeled fields:
  碳水、脂肪、蛋白、维生素、纤维、饱腹、消化负担、犯困风险、口味、气味.
- Drawer `饭后反馈` remains natural-language-first. It uses strict
  `feedback_parser_v1` JSON parsing through existing AppConfig settings,
  saves successful parses through `mealLogManager.saveMealFeedback(...)`, and
  reveals manual 1-5 scoring on parser/config/network failure.
- No schema change, recommendation-core rewrite, LLM rerank, OCR, or dish
  enrichment was added.
- Latest validation: `MealAdvisorValidation.exe` reports `48/50`. The only
  failures are the known non-blocking baseline cases:
  - non-LLM relaxed high-budget dinner does not lift hotpot into top-3
  - sparse seeded feedback does not emit sleepiness-watch / stable-favorites /
    low-repeat insight types
- Latest desktop smoke: 5-second launch passed with empty stderr.
- Latest Android arm64 debug APK:
  `C:\Users\Administrator\Desktop\MealAdvisor-arm64-debug.apk`
- Source/desktop APK SHA256:
  `6157107CD27478ECC716CC630378465B12E7B885D30A96906EF0768F701500F2`
- APK size: `68925434` bytes, timestamp `2026-04-26 01:53:20`
- `aapt dump badging` confirms package `org.qtproject.example.MealAdvisor`,
  `minSdkVersion 28`, `targetSdkVersion 36`, launchable activity
  `org.qtproject.qt.android.bindings.QtActivity`, launcher icons under
  `res/mipmap-*-v4/ic_launcher.png`, and native-code `arm64-v8a`.
- Android runtime validation is still blocked in the current shell:
  `adb devices -l` has no attached devices, and `emulator -list-avds` returns
  no configured AVD.

Current goal:
Run the Test Android Apps validation workflow if a real device or AVD is
available. Focus first on retesting Drawer `LLM 调试` -> `测试连接` with DeepSeek;
the previous phone screenshot failed with `TLS initialization failed`, and the
current APK packages Android OpenSSL. Then check placeholder behavior with the
Android soft keyboard, launcher icon, beige main shell, Food labels, and
Feedback LLM/manual fallback.

DeepSeek test values:
- API URL: `https://api.deepseek.com/chat/completions`
- Model: `deepseek-v4-flash`
- API Key: use a valid user-provided key; do not commit or document the key.

Validation workflow:
1. Use the test-android-apps android-emulator-qa skill.
2. Run:
   & 'C:\Users\Administrator\AppData\Local\Android\Sdk\platform-tools\adb.exe' devices -l
   & 'C:\Users\Administrator\AppData\Local\Android\Sdk\emulator\emulator.exe' -list-avds
3. If a device is available:
   - install `C:\Users\Administrator\Desktop\MealAdvisor-arm64-debug.apk`
   - resolve and launch `org.qtproject.example.MealAdvisor`
   - validate Drawer `LLM 调试` can test DeepSeek without
     `TLS initialization failed`
   - if connection testing still fails, capture logcat around
     `Device supports SSL:` and `qt.network.ssl`
   - capture UI tree and screenshots
   - validate launcher and recents icon show the meal-bowl icon
   - validate the main header no longer shows the planning/budget line
   - validate placeholder hints disappear while input fields are focused or
     populated instead of floating above the field
   - validate Drawer open/close and section switching
   - validate FoodPage dish tags: no bare repeated low/medium/high selectors,
     labels remain visible, and no label/control overlap
   - validate Drawer `饭后反馈`: natural-language field is primary, manual score
     controls remain reachable, unconfigured/failure fallback reveals manual
     scoring, and saving works
   - check keyboard entry, scrolling, portrait, and landscape
   - capture logcat if the app crashes or QML errors surface
4. If no device or AVD is available, do not pretend Android runtime validation
   passed. Report it as blocked and preserve the APK metadata evidence.
5. If screenshots reveal a concrete app-side issue, make the smallest targeted
   QML/C++ fix, rebuild desktop, rerun validation/smoke, rebuild/copy the
   Android APK, inspect badging, and repeat the screenshot check when possible.

Fast local regression commands:
1. Desktop build:
   $env:PATH='C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.10.3\mingw_64\bin;C:\Qt\Tools\Ninja;' + $env:PATH
   & 'C:\Qt\Tools\CMake_64\bin\cmake.exe' --build build\desktop-debug --target MealAdvisor MealAdvisorValidation
2. Validation:
   & '.\build\desktop-debug\MealAdvisorValidation.exe'
3. Desktop smoke:
   launch `build\desktop-debug\MealAdvisor.exe` for 5 seconds and confirm stderr is empty
4. Android build:
   & 'C:\Qt\Tools\CMake_64\bin\cmake.exe' --build build\android-arm64-debug --target MealAdvisor_make_apk
5. Copy APK:
   Copy-Item build\android-arm64-debug\android-build\build\outputs\apk\debug\android-build-debug.apk C:\Users\Administrator\Desktop\MealAdvisor-arm64-debug.apk -Force
6. APK library check:
   & 'C:\Users\Administrator\AppData\Local\Android\Sdk\build-tools\36.0.0\aapt.exe' list build\android-arm64-debug\android-build\build\outputs\apk\debug\android-build-debug.apk | Select-String -Pattern 'libssl|libcrypto|qopensslbackend'
7. Badging:
   & 'C:\Users\Administrator\AppData\Local\Android\Sdk\build-tools\36.0.0\aapt.exe' dump badging build\android-arm64-debug\android-build\build\outputs\apk\debug\android-build-debug.apk

Before finishing:
- Update docs/memory.md.
- Update docs/handoff.md if assumptions changed.
- Replace docs/next-task-prompt.md with the next concrete prompt.
```
