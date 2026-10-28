// Task 6: Library Management System
// Demonstrates: inheritance, virtual methods / polymorphism, smart pointers, file persistence
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <string>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <cstdlib>
using namespace std;

// ---------- helpers ----------
vector<string> split(const string& s, char d) {
    vector<string> out; string cur;
    for (char c : s) { if (c == d) { out.push_back(cur); cur.clear(); } else cur += c; }
    out.push_back(cur);
    return out;
}
string clean(string s) { replace(s.begin(), s.end(), '|', '/'); return s; }
string lower(string s) { transform(s.begin(), s.end(), s.begin(), ::tolower); return s; }
string readLine(const string& prompt) {
    string s; cout << prompt;
    if (!getline(cin, s)) exit(0);
    return s;
}
int readInt(const string& prompt, int lo, int hi) {
    while (true) {
        string s = readLine(prompt);
        try { size_t p; int v = stoi(s, &p); if (p == s.size() && v >= lo && v <= hi) return v; } catch (...) {}
        cout << "  [Error] Enter a number between " << lo << " and " << hi << ".\n";
    }
}

long dayOffset = 0;                                   // used to simulate days passing
long today() { return static_cast<long>(time(nullptr) / 86400) + dayOffset; }

// ---------- class hierarchy ----------
class MediaItem {
protected:
    int id;
    string title, creator;
    int borrowerId = 0;                               // 0 = available
    long dueDay = 0;
public:
    MediaItem(int i, string t, string c) : id(i), title(t), creator(c) {}
    virtual ~MediaItem() = default;                   // virtual destructor (safe delete via base pointer)

    virtual string type() const = 0;
    virtual int loanDays() const = 0;
    virtual double finePerDay() const = 0;
    virtual string extra() const = 0;                 // ISBN for Book, issue no. for Journal

    int getId() const { return id; }
    string getTitle() const { return title; }
    string getCreator() const { return creator; }
    int getBorrower() const { return borrowerId; }
    long getDue() const { return dueDay; }
    bool isBorrowed() const { return borrowerId != 0; }

    void checkout(int memberId, long now) { borrowerId = memberId; dueDay = now + loanDays(); }
    void giveBack() { borrowerId = 0; dueDay = 0; }
    void restore(int b, long d) { borrowerId = b; dueDay = d; }

    double fine(long now) const {
        if (!isBorrowed() || now <= dueDay) return 0;
        return (now - dueDay) * finePerDay();
    }

    virtual void display() const {
        cout << left << setw(5) << id << setw(9) << type() << setw(30) << title.substr(0, 28)
             << setw(20) << creator.substr(0, 18)
             << (isBorrowed() ? "Borrowed (member " + to_string(borrowerId) + ")" : "Available") << "\n";
    }
    string serialize() const {
        ostringstream o;
        o << type() << '|' << id << '|' << title << '|' << creator << '|' << extra() << '|' << borrowerId << '|' << dueDay;
        return o.str();
    }
};

class Book : public MediaItem {
    string isbn;
public:
    Book(int i, string t, string a, string isbn) : MediaItem(i, t, a), isbn(isbn) {}
    string type() const override { return "Book"; }
    int loanDays() const override { return 14; }
    double finePerDay() const override { return 2.0; }
    string extra() const override { return isbn; }
};

class Journal : public MediaItem {
    string issue;
public:
    Journal(int i, string t, string p, string issue) : MediaItem(i, t, p), issue(issue) {}
    string type() const override { return "Journal"; }
    int loanDays() const override { return 7; }
    double finePerDay() const override { return 5.0; }
    string extra() const override { return issue; }
};

struct Member { int id; string name; };

// ---------- library ----------
class Library {
    vector<unique_ptr<MediaItem>> items;              // owns the objects (no manual delete)
    vector<Member> members;
    const string itemFile = "library.txt", memberFile = "members.txt";
    const int MAX_LOANS = 3;

    MediaItem* findItem(int id) { for (auto& p : items) if (p->getId() == id) return p.get(); return nullptr; }
    Member* findMember(int id) { for (auto& m : members) if (m.id == id) return &m; return nullptr; }
    int nextItemId() const { int m = 100; for (auto& p : items) m = max(m, p->getId()); return m + 1; }
    int nextMemberId() const { int m = 0; for (auto& x : members) m = max(m, x.id); return m + 1; }
    int loansOf(int memberId) const { int n = 0; for (auto& p : items) if (p->getBorrower() == memberId) n++; return n; }

    void seed() {
        items.push_back(make_unique<Book>(101, "The C++ Programming Language", "Bjarne Stroustrup", "9780321563842"));
        items.push_back(make_unique<Book>(102, "Effective Modern C++", "Scott Meyers", "9781491903995"));
        items.push_back(make_unique<Book>(103, "Clean Code", "Robert C. Martin", "9780132350884"));
        items.push_back(make_unique<Journal>(104, "ACM Computing Surveys", "ACM", "Vol 58 No 2"));
        items.push_back(make_unique<Journal>(105, "IEEE Software", "IEEE", "Issue 5"));
        members.push_back({1, "Arun Kumar"});
        members.push_back({2, "Priya Nair"});
    }
public:
    Library() { load(); }

    void load() {
        ifstream mf(memberFile), inf(itemFile);
        string line;
        while (mf && getline(mf, line)) {
            auto f = split(line, '|');
            if (f.size() >= 2) { try { members.push_back({stoi(f[0]), f[1]}); } catch (...) {} }
        }
        while (inf && getline(inf, line)) {
            auto f = split(line, '|');
            if (f.size() < 7) continue;
            try {
                unique_ptr<MediaItem> p;
                if (f[0] == "Book") p = make_unique<Book>(stoi(f[1]), f[2], f[3], f[4]);
                else if (f[0] == "Journal") p = make_unique<Journal>(stoi(f[1]), f[2], f[3], f[4]);
                else continue;
                p->restore(stoi(f[5]), stol(f[6]));
                items.push_back(move(p));
            } catch (...) { cerr << "Skipping bad catalog line.\n"; }
        }
        if (items.empty() && members.empty()) seed();
        save();
    }
    void save() const {
        ofstream inf(itemFile), mf(memberFile);
        for (auto& p : items) inf << p->serialize() << '\n';
        for (auto& m : members) mf << m.id << '|' << m.name << '\n';
    }

    void listItems() const {
        cout << "\n" << left << setw(5) << "ID" << setw(9) << "Type" << setw(30) << "Title"
             << setw(20) << "Author/Publisher" << "Status\n" << string(85, '-') << "\n";
        for (auto& p : items) p->display();          // polymorphic call
    }
    void listMembers() const {
        for (auto& m : members) cout << "  Member " << m.id << ": " << m.name << " (loans: " << loansOf(m.id) << ")\n";
    }

    void addItem(bool isBook) {
        string t = clean(readLine("Title: ")), a = clean(readLine(isBook ? "Author: " : "Publisher: "));
        string e = clean(readLine(isBook ? "ISBN: " : "Issue: "));
        if (t.empty()) { cout << "  [Error] Title is required.\n"; return; }
        int id = nextItemId();
        if (isBook) items.push_back(make_unique<Book>(id, t, a, e));
        else items.push_back(make_unique<Journal>(id, t, a, e));
        save();
        cout << "  Added with ID " << id << ".\n";
    }
    void addMember() {
        string n = clean(readLine("Member name: "));
        if (n.empty()) { cout << "  [Error] Name required.\n"; return; }
        int id = nextMemberId();
        members.push_back({id, n});
        save();
        cout << "  Member created with ID " << id << ".\n";
    }

    void checkoutItem() {
        listMembers();
        int mid = readInt("Member ID: ", 1, 99999);
        if (!findMember(mid)) { cout << "  [Error] Member not found.\n"; return; }
        if (loansOf(mid) >= MAX_LOANS) { cout << "  [Error] Loan limit (" << MAX_LOANS << ") reached.\n"; return; }
        int iid = readInt("Item ID: ", 1, 99999);
        MediaItem* it = findItem(iid);
        if (!it) { cout << "  [Error] Item not found.\n"; return; }
        if (it->isBorrowed()) { cout << "  [Error] Item is already borrowed.\n"; return; }
        it->checkout(mid, today());
        save();
        cout << "  Checked out '" << it->getTitle() << "'. Loan period: " << it->loanDays()
             << " days. Fine: Rs. " << it->finePerDay() << "/day after due date.\n";
    }
    void returnItem() {
        int iid = readInt("Item ID to return: ", 1, 99999);
        MediaItem* it = findItem(iid);
        if (!it || !it->isBorrowed()) { cout << "  [Error] That item is not currently borrowed.\n"; return; }
        double f = it->fine(today());
        it->giveBack();
        save();
        cout << "  Returned. ";
        if (f > 0) cout << "OVERDUE! Fine due: Rs. " << fixed << setprecision(2) << f << "\n";
        else cout << "No fine.\n";
    }
    void search() const {
        string q = lower(readLine("Search title/author: "));
        bool found = false;
        for (auto& p : items)
            if (lower(p->getTitle()).find(q) != string::npos || lower(p->getCreator()).find(q) != string::npos) {
                p->display(); found = true;
            }
        if (!found) cout << "  No matches.\n";
    }
    void overdueReport() const {
        bool any = false;
        for (auto& p : items) {
            double f = p->fine(today());
            if (f > 0) {
                cout << "  " << p->getTitle() << " (member " << p->getBorrower() << ") - fine Rs. "
                     << fixed << setprecision(2) << f << "\n";
                any = true;
            }
        }
        if (!any) cout << "  No overdue items.\n";
    }
    void simulateDays() {
        int d = readInt("Fast-forward how many days (for testing fines)? ", 0, 3650);
        dayOffset += d;
        cout << "  Clock moved forward by " << d << " day(s) (total simulated: " << dayOffset << ").\n";
    }
};

int main() {
    Library lib;
    while (true) {
        cout << "\n===== LIBRARY MANAGEMENT SYSTEM =====\n"
             << "1. List catalog\n2. Add book\n3. Add journal\n4. Add member\n5. List members\n"
             << "6. Check out item\n7. Return item\n8. Search\n9. Overdue report\n"
             << "10. Fast-forward days (test fines)\n11. Save & Exit\n";
        switch (readInt("Choice: ", 1, 11)) {
            case 1: lib.listItems(); break;
            case 2: lib.addItem(true); break;
            case 3: lib.addItem(false); break;
            case 4: lib.addMember(); break;
            case 5: lib.listMembers(); break;
            case 6: lib.checkoutItem(); break;
            case 7: lib.returnItem(); break;
            case 8: lib.search(); break;
            case 9: lib.overdueReport(); break;
            case 10: lib.simulateDays(); break;
            case 11: lib.save(); cout << "Catalog saved. Goodbye!\n"; return 0;
        }
    }
}
