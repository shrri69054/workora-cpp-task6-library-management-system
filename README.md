# Task 6 – Library Management System (C++)

Demonstrates **inheritance, polymorphism, and safe dynamic memory**.

## Design
```
MediaItem (abstract base, virtual destructor)
 ├── Book     – 14-day loan, fine Rs. 2/day
 └── Journal  –  7-day loan, fine Rs. 5/day
```
- Virtual methods: `type()`, `loanDays()`, `finePerDay()`, `extra()`, `display()`
- The catalog is `std::vector<std::unique_ptr<MediaItem>>` – objects are owned and freed
  automatically, so there are no leaks and no manual `delete`
- `Library` class handles members, checkout/return, search, overdue report

## Features
- Add books / journals / members
- Check out (max 3 loans per member) and return
- Fine calculation for overdue items (type-specific rate)
- **"Fast-forward days"** menu option to demo fines without waiting
- Catalog and members saved to `library.txt` and `members.txt`

## File formats
```
library.txt : type|id|title|author-or-publisher|isbn-or-issue|borrowerId(0=none)|dueDay
members.txt : id|name
```
Default sample data is created on first run. To use the portal's "Library Seed Data",
convert it to these formats (or edit the `load()` function).

## Build & Run
```bash
g++ -std=c++17 -Wall -Wextra library.cpp -o library
./library
```

## Files
| File | Purpose |
|------|---------|
| `library.cpp` | Source code |
| `sample_output.txt` | Captured output (includes an overdue-fine demo) |
