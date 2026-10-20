# TIVIDY Team DevOps & Git Guide
**Goal:** Keep our builds working, avoid merge disasters, and make sure we don't lose marks to memory leaks on demo day.

---

I’ve put together an automated pipeline and a clean Git workflow so we don't break each other's code.

Here is a quick, straightforward guide on how we work together and what commands to run.

---

## 1. The One-Time Setup (Run this in your terminal once)

Before you do anything else, run these three commands on your machine:

```bash
# 1. Automatically stashes your unsaved work if you rebase/pull
git config --global rebase.autoStash true

# 2. Remembers how you resolved merge conflicts so you never have to fix the same conflict twice
git config --global rerere.enabled true

# 3. Keeps your Git pull history clean without messy merge commits
git config --global pull.rebase true
```

That’s it for setup.

---

## 2. Our Daily Workflow (Step-by-Step)

### Rule #1: Nobody pushes directly to `main` or `dev`.
`main` and `dev` are protected. All our work happens in personal feature branches, and we merge via **Pull Requests (PRs)** on GitHub.

### When starting a new feature:
Always create your branch off the latest `dev`:
```bash
git checkout dev
git pull origin dev
git checkout -b feat/your-feature-name
```
*(Example branch names: `feat/builder-pattern`, `feat/cor-handlers`, `feat/composite-rules`)*

### While you're working:
1. Write your code and your unit tests.
2. **ALWAYS commit your work before updating your branch:**
   ```bash
   git add .
   git commit -m "feat(cor): add intake validation handler"
   ```
   *(Committing locks your code into a permanent checkpoint so Git will never overwrite or lose your work).*

### Bringing in your teammates' latest updates:
When you want to pull in what everyone else has merged into `dev`:
```bash
git pull origin dev
```
If there are any merge conflicts, fix the highlighted lines, run `git add .`, and keep going.

### Before you push (Test locally first!):
Run the build and tests on your laptop so you know it compiles:
```bash
cmake --build build
ctest --output-on-failure
valgrind --leak-check=full ./build/bin/tividy_tests
```

### Push and open a Pull Request:
```bash
git push origin feat/your-feature-name
```
Then go to our GitHub repository, open a **Pull Request into `dev`**

---

## 3. What the Pipeline Does When You Push

The moment you open a Pull Request or push code, GitHub automatically boots up a clean Ubuntu cloud machine and runs:

1. **Compilation Check**: Builds the project with `-Wall -Wextra -Werror -std=c++11`.
2. **Automated Unit Tests**: Runs all our GoogleTests (`ctest`).
3. **Valgrind Memory Leak Check**: Runs `valgrind --leak-check=full --error-exitcode=1`.

* **Green Checkmark (✔)**: Everything compiles, all tests pass, zero memory leaks. Someone reviews the PR and it gets merged into `dev`.
* **Red X (❌)**: Something broke (a compiler error, a failed test, or a memory leak). GitHub will show you the exact file and line number. You just fix it locally on your branch, commit, push again, and the check re-runs automatically.

---

## 4. How to Test Your Code Without Waiting for Teammates

If you are working on your pattern (e.g., Builder or State), you don’t have to wait for someone else to finish their handler classes.

Use a simple **Mock Object** in your test file:
```cpp
// In your test file: a quick fake handler to test your own logic
class MockHandler : public Handler {
public:
    bool wasCalled = false;
    void handleRequest(WorkItem* item) override { wasCalled = true; }
    void setNext(Handler* next) override {}
    Handler* clone() override { return new MockHandler(*this); }
};
```
Test your own logic against the mock. Once your teammate finishes the real class, we plug their real class into an integration test. If anything doesn't match up, the test output will tell us exactly what needs adjusting.

---

## 5. Summary Cheat Sheet

| I want to... | What I run in the terminal |
| :--- | :--- |
| Start a new pattern / task | `git checkout dev` <br> `git pull origin dev` <br> `git checkout -b feat/my-pattern` |
| Save my progress | `git add .` <br> `git commit -m "feat: description"` |
| Grab latest updates from dev | `git pull origin dev` |
| Run tests locally | `cmake --build build && ctest` |
| Check for memory leaks | `valgrind --leak-check=full ./build/bin/tividy_tests` |
| Push to GitHub | `git push origin feat/my-pattern` |

