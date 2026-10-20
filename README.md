# TIVIDY: Generic Workflow Management System

## CI workflow setup(Github)
## 1. Quick Start & Build Instructions 

This project uses modern **CMake** with automatic dependency management. You do not need to install GoogleTest manually; CMake fetches and builds it automatically.

### Build and Run:
```bash
# 1. Configure the build
cmake -B build

# 2. Compile the project and test suite
cmake --build build

# 3. Run the main application
./build/bin/tividy_app

# 4. Run the unit test suite
./build/bin/tividy_tests

# 5. Check for memory leaks with Valgrind
valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./build/bin/tividy_tests
```

---

## 2. One-Time Git Setup (Run Once on Your Laptop)

Before starting work, configure these three Git settings to prevent accidental merge overwrites and messy commit histories:

```bash
# Automatically save uncommitted work when rebasing or pulling
git config --global rebase.autoStash true

# Remember and auto-apply conflict resolutions
git config --global rerere.enabled true

# Keep git pull clean via rebase rather than creating extra merge commits
git config --global pull.rebase true
```

---

## 3. Team Git & Branching Strategy

To ensure nobody breaks each other's code, we follow a strict **feature-branch workflow**:

* **`main`**: Protected production branch. Tagged milestone releases only.
* **`dev`**: Protected integration branch. All feature branches merge here via Pull Request.
* **`feat/<feature-name>`**: Individual branches where each member implements their assigned design pattern.

### Daily Workflow:
1. **Create your feature branch off `dev`**:
   ```bash
   git checkout dev
   git pull origin dev
   git checkout -b feat/your-pattern-name
   ```
2. **Work and commit locally**:
   Always commit your work before pulling updates from `dev`:
   ```bash
   git add .
   git commit -m "feat(pattern): add initial classes and tests"
   ```
3. **Pull latest changes from `dev`**:
   ```bash
   git pull origin dev
   ```
4. **Verify tests pass locally before pushing**:
   ```bash
   cmake --build build && ./build/bin/tividy_tests
   ```
5. **Push and open a Pull Request**:
   ```bash
   git push origin feat/your-pattern-name
   ```
   Open a **Pull Request into `dev`** on GitHub.

---

## 4. How the Automated CI/CD Pipeline Works

Every push to any branch and every Pull Request into `dev` or `main` automatically triggers our **GitHub Actions** cloud runner.

### Automated Checks (`build-and-test`):
1. **Compilation**: Built with `-Wall -Wextra -Werror -std=c++11`. Any compiler warning will fail the build.
2. **Unit Tests**: Runs all GoogleTest test suites via `ctest`.
3. **Memory Leak Inspection**: Runs Valgrind on the test binary with `--error-exitcode=1`. Any memory leak blocks the merge.

* **Green Checkmark (✔)**: All checks passed. Pull Request can be merged.
* **Red X (❌)**: Build, test, or memory leak failure. The PR is physically blocked from merging until resolved.

---

## 5. How to Write Unit Tests (GoogleTest)

Our CMake setup features **automatic test discovery**:
```cmake
file(GLOB TEST_SOURCES "tests/*.cpp")
```
Whenever you add a new `.cpp` file inside the `tests/` directory (e.g., `tests/test_builder.cpp`), CMake discovers, compiles, and registers it automatically.

### Writing a Test (Arrange - Act - Assert):
Create a file like `tests/test_my_pattern.cpp`:

```cpp
#include <gtest/gtest.h>

TEST(MyTestSuite, VerifiesExpectedBehavior) {
    // 1. Arrange: Set up objects
    int initialValue = 10;

    // 2. Act: Perform operation
    int result = initialValue * 2;

    // 3. Assert: Verify outcome
    EXPECT_EQ(result, 20);
    EXPECT_TRUE(result > 0);
    ASSERT_NE(result, 0); // Stops test immediately if false
}
```

---

## 6. Project Layout

```text
├── .github/
│   └── workflows/
│       └── ci.yml             # GitHub Actions CI workflow (job: build-and-test)
├── include/                   # Public header files (.h)
├── src/                       # Production source files (.cpp)
│   └── main.cpp               # Main application entry point
├── tests/                     # Unit test suites (.cpp)
│   └── test_smoke.cpp         # Smoke test verifying test runner and CI
├── docs/                      # Architecture research, diagrams, and proposals
├── CMakeLists.txt             # Root build system
├── .gitignore                 # Excluded build files and artifacts
└── README.md                  # This documentation
```
