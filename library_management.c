/*
 * LIBRARY BOOK MANAGEMENT SYSTEM
 * Mini Project - C Backend
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
//#include <emscripten/emscripten.h>

// Add EMSCRIPTEN_KEEPALIVE before each function you want JS to call
//EMSCRIPTEN_KEEPALIVE
//void add_book_wasm(const char *title, const char *author, int copies) {
    // your add_book logic here
//}

//EMSCRIPTEN_KEEPALIVE
//void issue_book_wasm(int book_id, const char *reg, const char *name) {
    // your issue_book logic here
//}
#define MAX_BOOKS    200
#define MAX_ISSUED   100
#define TITLE_LEN     80
#define AUTHOR_LEN    50
#define REG_LEN       20
#define NAME_LEN      50
#define DATE_LEN      20
#define BOOKS_FILE   "books.dat"
#define ISSUED_FILE  "issued.dat"

typedef struct {
    int  book_id;
    char title[TITLE_LEN];
    char author[AUTHOR_LEN];
    int  total_copies;
    int  available_copies;
} Book;

typedef struct {
    int  issue_id;
    int  book_id;
    char book_title[TITLE_LEN];
    char student_reg[REG_LEN];
    char student_name[NAME_LEN];
    char issue_date[DATE_LEN];
    char due_date[DATE_LEN];
    int  returned;
    char return_date[DATE_LEN];
} IssuedRecord;

Book         books[MAX_BOOKS];
IssuedRecord issued[MAX_ISSUED];
int          book_count   = 0;
int          issued_count = 0;

void get_today(char *buf);
void get_due_date(char *buf);
void to_lower(char *dest, const char *src);
void trim(char *s);
int  validate_reg(const char *reg);
void clear_input_buffer(void);
void save_books(void);
void load_books(void);
void save_issued(void);
void load_issued(void);
void add_book(void);
void issue_book(void);
void return_book(void);
void search_book(void);
void view_issued_books(void);
void view_all_books(void);
void display_menu(void);

int main(void)
{
    load_books();
    load_issued();

    int choice;

    printf("\n  LIBRARY BOOK MANAGEMENT SYSTEM\n\n");

    do {
        display_menu();
        printf("Enter your choice : ");
        if (scanf("%d", &choice) != 1) {
            clear_input_buffer();
            printf("ERROR: Invalid input. Please enter a number.\n");
            choice = -1;
            continue;
        }
        clear_input_buffer();

        switch (choice) {
            case 1: add_book();          break;
            case 2: issue_book();        break;
            case 3: return_book();       break;
            case 4: search_book();       break;
            case 5: view_issued_books(); break;
            case 6: view_all_books();    break;
            case 0:
                save_books();
                save_issued();
                printf("\nRecords saved. Goodbye!\n\n");
                break;
            default:
                printf("ERROR: Invalid choice. Try again.\n");
        }
    } while (choice != 0);

    return 0;
}

void display_menu(void)
{
    printf("\n  -------- MAIN MENU --------\n");
    printf("  1. Add Book\n");
    printf("  2. Issue Book\n");
    printf("  3. Return Book\n");
    printf("  4. Search Book\n");
    printf("  5. View Issued Books\n");
    printf("  6. View All Books\n");
    printf("  0. Save and Exit\n");
    printf("  ---------------------------\n");
}

void add_book(void)
{
    if (book_count >= MAX_BOOKS) {
        printf("ERROR: Book storage is full.\n");
        return;
    }

    Book b;
    b.book_id = (book_count == 0) ? 1001 : books[book_count - 1].book_id + 1;

    printf("\n--- ADD NEW BOOK ---\n");
    printf("Auto-assigned Book ID : %d\n", b.book_id);

    printf("Title  : ");
    fgets(b.title, TITLE_LEN, stdin);
    trim(b.title);
    if (strlen(b.title) == 0) {
        printf("ERROR: Title cannot be empty.\n");
        return;
    }

    printf("Author : ");
    fgets(b.author, AUTHOR_LEN, stdin);
    trim(b.author);
    if (strlen(b.author) == 0) {
        printf("ERROR: Author cannot be empty.\n");
        return;
    }

    printf("Total Copies : ");
    if (scanf("%d", &b.total_copies) != 1 || b.total_copies <= 0) {
        clear_input_buffer();
        printf("ERROR: Copies must be a positive number.\n");
        return;
    }
    clear_input_buffer();

    b.available_copies = b.total_copies;
    books[book_count++] = b;
    save_books();

    printf("SUCCESS: Book added! (ID: %d)\n", b.book_id);
}

void issue_book(void)
{
    if (issued_count >= MAX_ISSUED) {
        printf("ERROR: Issue records are full.\n");
        return;
    }

    printf("\n--- ISSUE BOOK ---\n");

    int book_id;
    printf("Enter Book ID : ");
    if (scanf("%d", &book_id) != 1) {
        clear_input_buffer();
        printf("ERROR: Invalid Book ID.\n");
        return;
    }
    clear_input_buffer();

    int bi = -1;
    for (int i = 0; i < book_count; i++) {
        if (books[i].book_id == book_id) { bi = i; break; }
    }
    if (bi == -1) {
        printf("ERROR: Book ID %d not found.\n", book_id);
        return;
    }
    if (books[bi].available_copies <= 0) {
        printf("ERROR: No available copies of \"%s\". All copies are issued.\n", books[bi].title);
        return;
    }

    IssuedRecord rec;
    memset(&rec, 0, sizeof(rec));

    printf("Student Register Number : ");
    fgets(rec.student_reg, REG_LEN, stdin);
    trim(rec.student_reg);

    if (!validate_reg(rec.student_reg)) {
        printf("ERROR: Invalid register number \"%s\".\n", rec.student_reg);
        printf("       Must be exactly 9 alphanumeric characters.\n");
        return;
    }

    for (int i = 0; i < issued_count; i++) {
        if (issued[i].returned == 0 &&
            issued[i].book_id == book_id &&
            strcmp(issued[i].student_reg, rec.student_reg) == 0) {
            printf("ERROR: Student %s has already issued this book (Issue ID: %d).\n",
                   rec.student_reg, issued[i].issue_id);
            return;
        }
    }

    printf("Student Name : ");
    fgets(rec.student_name, NAME_LEN, stdin);
    trim(rec.student_name);
    if (strlen(rec.student_name) == 0) {
        printf("ERROR: Student name cannot be empty.\n");
        return;
    }

    rec.issue_id = (issued_count == 0) ? 1 : issued[issued_count - 1].issue_id + 1;
    rec.book_id  = book_id;
    strncpy(rec.book_title, books[bi].title, TITLE_LEN - 1);
    get_today(rec.issue_date);
    get_due_date(rec.due_date);
    rec.returned = 0;
    strcpy(rec.return_date, "N/A");

    books[bi].available_copies--;
    issued[issued_count++] = rec;
    save_books();
    save_issued();

    printf("\nSUCCESS: Book Issued!\n");
    printf("Issue ID     : %d\n", rec.issue_id);
    printf("Book         : %s\n", rec.book_title);
    printf("Student      : %s\n", rec.student_name);
    printf("Register No  : %s\n", rec.student_reg);
    printf("Issue Date   : %s\n", rec.issue_date);
    printf("Due Date     : %s\n", rec.due_date);
}

void return_book(void)
{
    printf("\n--- RETURN BOOK ---\n");

    int issue_id;
    printf("Enter Issue ID : ");
    if (scanf("%d", &issue_id) != 1) {
        clear_input_buffer();
        printf("ERROR: Invalid Issue ID.\n");
        return;
    }
    clear_input_buffer();

    int ri = -1;
    for (int i = 0; i < issued_count; i++) {
        if (issued[i].issue_id == issue_id) { ri = i; break; }
    }
    if (ri == -1) {
        printf("ERROR: Issue ID %d not found.\n", issue_id);
        return;
    }
    if (issued[ri].returned == 1) {
        printf("ERROR: Issue ID %d was already returned on %s.\n",
               issue_id, issued[ri].return_date);
        return;
    }

    issued[ri].returned = 1;
    get_today(issued[ri].return_date);

    for (int i = 0; i < book_count; i++) {
        if (books[i].book_id == issued[ri].book_id) {
            books[i].available_copies++;
            break;
        }
    }

    save_books();
    save_issued();

    printf("\nSUCCESS: Book returned.\n");
    printf("Book        : %s\n", issued[ri].book_title);
    printf("Student     : %s (%s)\n", issued[ri].student_name, issued[ri].student_reg);
    printf("Returned on : %s\n", issued[ri].return_date);
}

void search_book(void)
{
    printf("\n--- SEARCH BOOK ---\n");
    printf("1. Search by Book ID\n");
    printf("2. Search by Title Keyword\n");
    printf("Choice : ");

    int opt;
    if (scanf("%d", &opt) != 1) {
        clear_input_buffer();
        printf("ERROR: Invalid option.\n");
        return;
    }
    clear_input_buffer();

    if (opt == 1) {
        int book_id;
        printf("Enter Book ID : ");
        if (scanf("%d", &book_id) != 1) {
            clear_input_buffer();
            printf("ERROR: Invalid Book ID.\n");
            return;
        }
        clear_input_buffer();

        int found = 0;
        for (int i = 0; i < book_count; i++) {
            if (books[i].book_id == book_id) {
                printf("\nBook Found:\n");
                printf("ID        : %d\n", books[i].book_id);
                printf("Title     : %s\n", books[i].title);
                printf("Author    : %s\n", books[i].author);
                printf("Total     : %d\n", books[i].total_copies);
                printf("Available : %d\n", books[i].available_copies);
                printf("Status    : %s\n",
                       books[i].available_copies > 0 ? "AVAILABLE" : "ALL COPIES ISSUED");
                found = 1;
                break;
            }
        }
        if (!found)
            printf("ERROR: No book found with ID %d.\n", book_id);

    } else if (opt == 2) {
        char keyword[TITLE_LEN];
        char kw_lower[TITLE_LEN], title_lower[TITLE_LEN];

        printf("Enter Keyword : ");
        fgets(keyword, TITLE_LEN, stdin);
        trim(keyword);

        if (strlen(keyword) == 0) {
            printf("ERROR: Keyword cannot be empty.\n");
            return;
        }

        to_lower(kw_lower, keyword);

        int found = 0;
        printf("\nSearch results for \"%s\":\n", keyword);
        for (int i = 0; i < book_count; i++) {
            to_lower(title_lower, books[i].title);
            if (strstr(title_lower, kw_lower) != NULL) {
                printf("  ID: %d | %s | Author: %s | Available: %d/%d\n",
                       books[i].book_id,
                       books[i].title,
                       books[i].author,
                       books[i].available_copies,
                       books[i].total_copies);
                found = 1;
            }
        }
        if (!found)
            printf("ERROR: No books found matching \"%s\".\n", keyword);
    } else {
        printf("ERROR: Invalid search option.\n");
    }
}

void view_issued_books(void)
{
    printf("\n--- CURRENTLY ISSUED BOOKS ---\n");
    printf("%-8s %-6s %-25s %-12s %-18s %-12s\n",
           "IssueID", "BkID", "Title", "RegNo", "Student", "DueDate");
    printf("----------------------------------------------------------------------\n");

    int count = 0;
    for (int i = 0; i < issued_count; i++) {
        if (issued[i].returned == 0) {
            printf("%-8d %-6d %-25s %-12s %-18s %-12s\n",
                   issued[i].issue_id,
                   issued[i].book_id,
                   issued[i].book_title,
                   issued[i].student_reg,
                   issued[i].student_name,
                   issued[i].due_date);
            count++;
        }
    }

    if (count == 0)
        printf("No books are currently issued.\n");
    else
        printf("\nTotal currently issued: %d\n", count);
}

void view_all_books(void)
{
    printf("\n--- ALL BOOKS ---\n");
    printf("%-6s %-35s %-20s %-7s %-9s\n",
           "ID", "Title", "Author", "Total", "Available");
    printf("----------------------------------------------------------------------\n");

    if (book_count == 0) {
        printf("No books in the system yet.\n");
        return;
    }

    for (int i = 0; i < book_count; i++) {
        printf("%-6d %-35s %-20s %-7d %-9d\n",
               books[i].book_id,
               books[i].title,
               books[i].author,
               books[i].total_copies,
               books[i].available_copies);
    }
    printf("\nTotal books in catalogue: %d\n", book_count);
}

void save_books(void)
{
    FILE *f = fopen(BOOKS_FILE, "wb");
    if (!f) { printf("WARNING: Could not save books file.\n"); return; }
    fwrite(&book_count, sizeof(int), 1, f);
    fwrite(books, sizeof(Book), book_count, f);
    fclose(f);
}

void load_books(void)
{
    FILE *f = fopen(BOOKS_FILE, "rb");
    if (!f) return;
    fread(&book_count, sizeof(int), 1, f);
    fread(books, sizeof(Book), book_count, f);
    fclose(f);
    printf("Loaded %d book(s) from records.\n", book_count);
}

void save_issued(void)
{
    FILE *f = fopen(ISSUED_FILE, "wb");
    if (!f) { printf("WARNING: Could not save issued file.\n"); return; }
    fwrite(&issued_count, sizeof(int), 1, f);
    fwrite(issued, sizeof(IssuedRecord), issued_count, f);
    fclose(f);
}

void load_issued(void)
{
    FILE *f = fopen(ISSUED_FILE, "rb");
    if (!f) return;
    fread(&issued_count, sizeof(int), 1, f);
    fread(issued, sizeof(IssuedRecord), issued_count, f);
    fclose(f);
    printf("Loaded %d issue record(s) from records.\n", issued_count);
}

void get_today(char *buf)
{
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, DATE_LEN, "%d-%m-%Y", tm_info);
}

void get_due_date(char *buf)
{
    time_t t = time(NULL) + (14 * 24 * 3600);
    struct tm *tm_info = localtime(&t);
    strftime(buf, DATE_LEN, "%d-%m-%Y", tm_info);
}

void to_lower(char *dest, const char *src)
{
    int i = 0;
    while (src[i]) { dest[i] = (char)tolower((unsigned char)src[i]); i++; }
    dest[i] = '\0';
}

void trim(char *s)
{
    int len = (int)strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r' || s[len-1] == ' '))
        s[--len] = '\0';
}

int validate_reg(const char *reg)
{
    int len = (int)strlen(reg);
    if (len != 9) return 0;
    for (int i = 0; i < len; i++)
        if (!isalnum((unsigned char)reg[i])) return 0;
    return 1;
}

void clear_input_buffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}
