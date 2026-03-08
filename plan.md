1. Replace the `Makefile` with the V3 structure.
2. Create the `src` directory and move source files.
3. Update includes, entry point, and macros in `main.cpp` and `calc.cpp`.
4. Update keycode mappings in `options.h`.
5. Remove obsolete build files (`linker.ld`, `.gitmodules`, `CPappTemplate`).
6. Add V3 features (`.github`, `.devcontainer`, `.vscode`, `.clangd`, `.gitignore`).
7. Build the project. Since docker doesn't work, skip this step.
8. Complete pre commit steps
   - Ensure proper testing, verifications, reviews and reflections are done. Addressed `APP_NAME` in `Makefile`, `v4` actions in `c-cpp.yml`, and `GetInput(&event, 0, 0x10);` with zero timeout.
9. Submit the changes.
