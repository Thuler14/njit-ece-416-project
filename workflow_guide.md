# 🚀 Team Workflow Guide (GitHub + Arduino)

Follow this guide each time you work on an Issue or Milestone.  
It keeps our repo clean, consistent, and easy to review.

---

## 🔑 Rules

- **One branch per milestone** (not per issue).\
  Example: `m1-bringup`, `m2-closedloop`, …
- **Scratch branches per person/issue**, created **from the milestone branch**, then merged back into the milestone branch.\
  Example: `scratch/m1-issue1-blink-test`, `scratch/m2-issue12-pi-loop`.
- **Examples folder** → demo sketches (Blink, sensor scan, servo sweep).
- **Main firmware** → `firmware/control/main/` or `firmware/ui/main/`.
- **Evidence lives in the repo** (`tests/`, `mechanical/photos/`, `docs/`).
- **Commit messages reference the Issue number.**

---

## 🔀 Starting a Milestone

1. Checkout and update main:

   ```bash
   git checkout main
   git pull origin main
   ```

2. Create a branch for the milestone:

   ```bash
   git checkout -b m1-bringup
   ```

3. Assign milestone Issues to yourself in GitHub and move them to In Progress.

---

## 💻 Working on an Issue (single person)

Example: **Issue #1 (Blink Test)**

1. Checkout the milestone branch:

   ```bash
   git checkout m1-bringup
   ```

2. Code location:

   - Demo → `firmware/examples/m1_blink/m1_blink.ino`

3. Test on hardware (Arduino IDE → Upload).

4. Save evidence:

   - Data: `tests/data/m1_blink_test.csv`
   - Reports: `tests/reports/m1_blink_test.png`
   - Photos: `mechanical/photos/m1_blink_test.jpg`

5. Commit:

   ```bash
   git add <files>
   git commit -m "M1: Blink Test (Issue #1)"
   git push origin m1-bringup
   ```

6. Comment on the Issue with evidence paths.

7. Move card → Review/Testing.

---

## 💻 Working on an Issue (multiple members in same milestone)

> Use a **scratch branch** off the milestone branch. Merge it back into the milestone branch via PR, then delete it.

1. **Start from the milestone branch**

```bash
git fetch origin
git checkout m1-bringup
git pull origin m1-bringup
```

2. **Create a scratch branch for your Issue**

```bash
git checkout -b scratch/m1-issue1-blink-test
```

> Naming: `scratch/<milestone>-issue<nr>-<short-name>`

3. **Do the work → commit regularly**

```bash
git add <files>
git commit -m "M1: Blink Test (Issue #1)"
```

4. **Push the scratch branch to GitHub**

```bash
git push -u origin scratch/m1-issue1-blink-test
```

5. **Open a PR** *from* `scratch/m1-issue1-blink-test` *into* `m1-bringup` (not `main`).

   - Title example: `M1: Blink Test (Issue #1)`
   - Body: link evidence paths.

6. **Merge the PR** (recommended: *Squash & merge* for tidy history on milestone branch).

7. **Delete the scratch branch** after merge

```bash
# delete remote branch
git push origin --delete scratch/m1-issue1-blink-test
# delete local branch
git branch -d scratch/m1-issue1-blink-test
```

---

## 🔄 Syncing milestone branches

### Keep milestone branch current with `main`

**Milestone Captain (daily):**

```bash
# Update main
git checkout main && git pull origin main
# Rebase milestone on main
git checkout m1-bringup && git rebase main
# Push safely (force-with-lease preserves others' work)
git push --force-with-lease origin m1-bringup
```

### Re-sync your scratch branch after captain updates

```bash
# switch to your scratch branch
git checkout scratch/m1-issue1-blink-test
# fetch and rebase onto the updated milestone base
git fetch origin
git rebase origin/m1-bringup
# resolve any conflicts, then continue
#   git status  # see conflicted files
#   edit files to fix conflicts
#   git add <fixed files>
#   git rebase --continue
# push updated branch safely
git push --force-with-lease
```

### (Alternative) Merge milestone -> scratch (if you prefer merge commits)

```bash
git checkout scratch/m1-issue1-blink-test
git fetch origin
git merge origin/m1-bringup  # resolves conflicts now
# push
git push
```

---

## 🔗 Completing a Milestone

1. Ensure all Issues for the milestone are merged **into the milestone branch** via scratch PRs.

2. Checkout and push the milestone branch:
   ```bash
   git checkout m1-bringup
   git push origin m1-bringup
   ```

3. Open **one PR** from the milestone branch to `main`:
   - **Base:** `main`
   - **Compare:** `m1-bringup`
   - Use the Milestone PR template
   - **Title:** M1: Bring-up complete (Fixes #1, #2, #3, #4, #5, #6, #7)

4. Choose merge strategy:

- **Squash & merge** (default; clean single commit on `main`)
- or **Rebase & merge** (preserve individual commits)

5. After merge:

```bash
# delete remote milestone branch
git push origin --delete m1-bringup
# delete local milestone branch
git branch -d m1-bringup
```

6. Close the milestone and move cards → **Done**.

---

## ✅ Commit message format

```
M<milestone>: <short description> (Issue #<n>)
```

Examples:

- `M1: Blink Test (Issue #1)`
- `M2: Add PI loop (Issue #12)`
- `M3: E-stop wiring + code (Issue #20)`

---

## 📂 Evidence folders

- **tests/data/** → raw logs (CSV)
- **tests/reports/** → plots, screenshots, photos, videos
- **mechanical/photos/** → build & wiring pictures
- **docs/design/** → diagrams, calibration notes
- **docs/final-report/** → final report and slides

Naming: always start with milestone → `mX_taskname.*`

Examples:

- `tests/data/m2_closedloop.csv`
- `tests/reports/m3_fault_demo.mp4`
- `mechanical/photos/m4_cable_management.jpg`

---

### Milestone PR (milestone → main)

```markdown
### Summary
Milestone M<X> — <name> complete

### Completed Issues
Fixes #<n>, #<n>, #<n>

### Evidence
- tests/data/mX_*.csv
- tests/reports/mX_*.png/mp4
- mechanical/photos/mX_*.jpg
- docs/design/...
```

---

# 🔁 Quick Cycle

1. **Pick Issue** → Assign → Move to **In Progress**.
2. **Create scratch branch** from the milestone branch.
3. **Work** → Code + Evidence → Commit → Push.
4. **PR scratch → milestone** → Review → Merge → Delete scratch branch.
5. Repeat for remaining Issues in the milestone.
6. **PR milestone → main** → Merge → Close Milestone → Cards to **Done**.
