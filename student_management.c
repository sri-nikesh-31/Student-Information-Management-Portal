/*
 * Student Academic Portal
 * Features: semester-wise results, per-subject modification, admin dashboard
 * Compile: gcc student_portal.c -o student_portal
 *
 * File layout
 * -----------
 * Student credentials : <mail>
 *   Line 1 : email
 *   Line 2 : password
 *   Line 3 : yoga answer (1/2)
 *   Line 4 : sports answer (1/2)
 *   Line 5 : lucky number
 *
 * Semester result     : sem_<sem>_<mail>
 *   One line per subject:
 *     SUBJ|<name>|<credits>|<grade>
 *   Last line:
 *     SPI|<value>
 *
 * Admin file          : <adminid>.txt
 *   Line 1 : admin id
 *   Line 2 : password
 *
 * Students list       : studentslist.txt
 *   <name>\t<email>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* ═══════════════════════════ Constants ════════════════════════════ */
#define MAX_MAIL      60
#define MAX_PWD       40
#define MAX_NAME      60
#define MAX_LINE      512
#define MAX_SUBJECTS  30
#define MAX_SEMS       8

#define MAIN_ADMIN_ID  "admin@iiti.ac.in"
#define MAIN_ADMIN_PWD "admin_123"
#define STUDENTS_FILE  "studentslist.txt"

/* ═══════════════════════════ Data types ═══════════════════════════ */
typedef struct {
    char name[MAX_NAME];
    int  credits;
    int  grade;
} Subject;

typedef struct {
    int     sem;
    Subject subjects[MAX_SUBJECTS];
    int     num_subjects;
    float   spi;
} SemResult;

/* ═══════════════════════════ Helpers ══════════════════════════════ */

static void strip_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') s[len - 1] = '\0';
    if (len > 1 && s[len - 2] == '\r') s[len - 2] = '\0';
}

static void read_string(const char *prompt, char *buf, int size) {
    while (1) {
        printf("%s", prompt);
        if (fgets(buf, size, stdin)) {
            strip_newline(buf);
            if (strlen(buf) > 0) return;
        }
        printf("  Input cannot be empty. Try again.\n");
    }
}

static int read_int(const char *prompt) {
    char buf[32];
    while (1) {
        printf("%s", prompt);
        if (fgets(buf, sizeof(buf), stdin)) {
            int val;
            if (sscanf(buf, "%d", &val) == 1) return val;
        }
        printf("  Invalid input. Enter a number.\n");
    }
}

/* Build semester result filename: sem_<N>_<mail> */
static void sem_filename(int sem, const char *mail, char *out, int size) {
    snprintf(out, size, "sem_%d_%s", sem, mail);
}

/* Build admin filename: <adminid>.txt */
static void admin_filename(const char *id, char *out, int size) {
    snprintf(out, size, "%s.txt", id);
}

/* ═══════════════════════════ SemResult I/O ════════════════════════ */

/* Compute SPI from subjects */
static float compute_spi(const SemResult *r) {
    int total_pts = 0, total_cr = 0;
    for (int i = 0; i < r->num_subjects; i++) {
        total_pts += r->subjects[i].grade * r->subjects[i].credits;
        total_cr  += r->subjects[i].credits;
    }
    return (total_cr > 0) ? (float)total_pts / total_cr : 0.0f;
}

/* Save a SemResult to disk */
static bool save_sem_result(int sem, const char *mail, const SemResult *r) {
    char path[MAX_MAIL + 20];
    sem_filename(sem, mail, path, sizeof(path));

    FILE *f = fopen(path, "w");
    if (!f) { printf("Error: cannot write %s\n", path); return false; }

    for (int i = 0; i < r->num_subjects; i++) {
        fprintf(f, "SUBJ|%s|%d|%d\n",
                r->subjects[i].name,
                r->subjects[i].credits,
                r->subjects[i].grade);
    }
    fprintf(f, "SPI|%.2f\n", r->spi);
    fclose(f);
    return true;
}

/* Load a SemResult from disk; returns false if file missing */
static bool load_sem_result(int sem, const char *mail, SemResult *r) {
    char path[MAX_MAIL + 20];
    sem_filename(sem, mail, path, sizeof(path));

    FILE *f = fopen(path, "r");
    if (!f) return false;

    r->sem         = sem;
    r->num_subjects = 0;

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), f)) {
        strip_newline(line);
        if (strncmp(line, "SUBJ|", 5) == 0) {
            /* Parse: SUBJ|name|credits|grade */
            char *tok = strtok(line + 5, "|");
            if (!tok) continue;
            Subject *s = &r->subjects[r->num_subjects];
            strncpy(s->name, tok, sizeof(s->name) - 1);
            tok = strtok(NULL, "|");
            s->credits = tok ? atoi(tok) : 0;
            tok = strtok(NULL, "|");
            s->grade   = tok ? atoi(tok) : 0;
            r->num_subjects++;
        } else if (strncmp(line, "SPI|", 4) == 0) {
            r->spi = (float)atof(line + 4);
        }
    }
    fclose(f);
    return true;
}

/* Print a SemResult neatly */
static void print_sem_result(const SemResult *r) {
    printf("\n  ┌─────────────────────────────────────────────────────┐\n");
    printf("  │              Semester %d Result                      │\n", r->sem);
    printf("  ├──────┬──────────────────────┬─────────┬─────────────┤\n");
    printf("  │ S.No │ Subject              │ Credits │ Grade Pts   │\n");
    printf("  ├──────┼──────────────────────┼─────────┼─────────────┤\n");
    for (int i = 0; i < r->num_subjects; i++) {
        printf("  │ %-4d │ %-20s │ %-7d │ %-11d │\n",
               i + 1,
               r->subjects[i].name,
               r->subjects[i].credits,
               r->subjects[i].grade);
    }
    printf("  ├──────┴──────────────────────┴─────────┴─────────────┤\n");
    printf("  │  SPI : %-6.2f                                       │\n", r->spi);
    printf("  └─────────────────────────────────────────────────────┘\n");
}

/* ═══════════════════════════ Admin management ═════════════════════ */

static void manage_admin(void) {
    char admid[MAX_MAIL], admpwd[MAX_PWD], filepath[MAX_MAIL + 8];
    int choice;

    while (1) {
        printf("\n--- Admin Management ---\n");
        printf("1. Add new admin\n");
        printf("2. Update existing admin credentials\n");
        printf("3. Back\n");
        choice = read_int("Choice: ");

        if (choice == 3) return;
        if (choice != 1 && choice != 2) { printf("Invalid choice.\n"); continue; }

        read_string("Admin ID (email): ", admid, sizeof(admid));
        read_string("Admin password  : ", admpwd, sizeof(admpwd));
        admin_filename(admid, filepath, sizeof(filepath));

        if (choice == 2) {
            FILE *chk = fopen(filepath, "r");
            if (!chk) { printf("No admin with that ID found.\n"); continue; }
            fclose(chk);
            remove(filepath);
        }

        FILE *f = fopen(filepath, "w");
        if (!f) { printf("Error: could not create admin file.\n"); continue; }
        fprintf(f, "%s\n%s\n", admid, admpwd);
        fclose(f);
        printf("Admin saved successfully.\n");
    }
}

/* ═══════════════════════════ Result entry & edit ══════════════════ */

/* Enter a brand-new semester result (fresh) */
static void enter_fresh_result(int sem, const char *mail) {
    SemResult r;
    r.sem = sem;

    r.num_subjects = read_int("Number of subjects: ");
    if (r.num_subjects <= 0 || r.num_subjects > MAX_SUBJECTS) {
        printf("Invalid subject count (1-%d).\n", MAX_SUBJECTS);
        return;
    }

    for (int i = 0; i < r.num_subjects; i++) {
        printf("\n-- Subject %d --\n", i + 1);
        read_string("Subject name : ", r.subjects[i].name, MAX_NAME);
        r.subjects[i].grade   = read_int("Grade points : ");
        r.subjects[i].credits = read_int("Credits      : ");
    }

    r.spi = compute_spi(&r);
    if (save_sem_result(sem, mail, &r))
        printf("\nSaved. SPI for Semester %d = %.2f\n", sem, r.spi);
}

/* Modify an existing semester result subject-by-subject */
static void modify_sem_result(int sem, const char *mail) {
    SemResult r;
    if (!load_sem_result(sem, mail, &r)) {
        printf("No result found for semester %d. Use 'Enter' first.\n", sem);
        return;
    }

    while (1) {
        print_sem_result(&r);
        printf("\nWhat would you like to modify?\n");
        printf("1. Edit a subject's details\n");
        printf("2. Add a new subject\n");
        printf("3. Delete a subject\n");
        printf("4. Done (save & exit)\n");
        int ch = read_int("Choice: ");

        if (ch == 4) break;

        if (ch == 1) {
            int sno = read_int("Enter subject S.No to edit: ");
            if (sno < 1 || sno > r.num_subjects) { printf("Invalid S.No.\n"); continue; }
            Subject *s = &r.subjects[sno - 1];
            printf("Current: %s | Credits:%d | Grade:%d\n",
                   s->name, s->credits, s->grade);
            printf("(Press Enter to keep current value for name)\n");

            /* Name */
            printf("New subject name [%s]: ", s->name);
            char tmp[MAX_NAME];
            if (fgets(tmp, sizeof(tmp), stdin)) {
                strip_newline(tmp);
                if (strlen(tmp) > 0) strncpy(s->name, tmp, MAX_NAME - 1);
            }
            s->grade   = read_int("New grade points : ");
            s->credits = read_int("New credits      : ");

        } else if (ch == 2) {
            if (r.num_subjects >= MAX_SUBJECTS) {
                printf("Maximum subjects reached.\n"); continue;
            }
            Subject *s = &r.subjects[r.num_subjects];
            read_string("Subject name : ", s->name, MAX_NAME);
            s->grade   = read_int("Grade points : ");
            s->credits = read_int("Credits      : ");
            r.num_subjects++;

        } else if (ch == 3) {
            int sno = read_int("Enter subject S.No to delete: ");
            if (sno < 1 || sno > r.num_subjects) { printf("Invalid S.No.\n"); continue; }
            /* Shift subjects left */
            for (int i = sno - 1; i < r.num_subjects - 1; i++)
                r.subjects[i] = r.subjects[i + 1];
            r.num_subjects--;
            printf("Subject removed.\n");

        } else {
            printf("Invalid choice.\n");
        }
    }

    /* Recalculate and save */
    r.spi = compute_spi(&r);
    char path[MAX_MAIL + 20];
    sem_filename(sem, mail, path, sizeof(path));
    remove(path);
    if (save_sem_result(sem, mail, &r))
        printf("Changes saved. Updated SPI = %.2f\n", r.spi);
}

/* Show all semesters for a student */
static void view_all_sems(const char *mail) {
    bool any = false;
    for (int s = 1; s <= MAX_SEMS; s++) {
        SemResult r;
        if (load_sem_result(s, mail, &r)) {
            print_sem_result(&r);
            any = true;
        }
    }
    if (!any) printf("No results found for %s.\n", mail);
}

/* ═══════════════════════════ Admin dashboard ══════════════════════ */

static void admin_dashboard(void) {
    while (1) {
        printf("\n╔══════════════════════════╗\n");
        printf("║     Admin Dashboard      ║\n");
        printf("╚══════════════════════════╝\n");

        /* List students */
        printf("\n-- Registered Students --\n");
        FILE *sl = fopen(STUDENTS_FILE, "r");
        if (!sl) {
            printf("(No students registered yet)\n");
        } else {
            char line[MAX_LINE];
            int idx = 1;
            while (fgets(line, sizeof(line), sl)) {
                strip_newline(line);
                printf("  %d. %s\n", idx++, line);
            }
            fclose(sl);
        }

        printf("\n1. Enter result for a semester\n");
        printf("2. Modify an existing semester result\n");
        printf("3. View all semesters of a student\n");
        printf("4. View one semester of a student\n");
        printf("5. Logout\n");
        int ch = read_int("Choice: ");

        if (ch == 5) return;
        if (ch < 1 || ch > 4) { printf("Invalid choice.\n"); continue; }

        char mail[MAX_MAIL];
        read_string("Student email: ", mail, sizeof(mail));

        if (ch == 1) {
            int sem = read_int("Semester number (1-8): ");
            if (sem < 1 || sem > MAX_SEMS) { printf("Invalid semester.\n"); continue; }

            /* Check if result already exists */
            SemResult existing;
            if (load_sem_result(sem, mail, &existing)) {
                printf("Result for Semester %d already exists.\n", sem);
                printf("1. Overwrite with fresh entry\n");
                printf("2. Modify existing (recommended)\n");
                printf("3. Cancel\n");
                int ov = read_int("Choice: ");
                if (ov == 1) {
                    char path[MAX_MAIL + 20];
                    sem_filename(sem, mail, path, sizeof(path));
                    remove(path);
                    enter_fresh_result(sem, mail);
                } else if (ov == 2) {
                    modify_sem_result(sem, mail);
                }
            } else {
                enter_fresh_result(sem, mail);
            }

        } else if (ch == 2) {
            int sem = read_int("Semester number (1-8): ");
            if (sem < 1 || sem > MAX_SEMS) { printf("Invalid semester.\n"); continue; }
            modify_sem_result(sem, mail);

        } else if (ch == 3) {
            view_all_sems(mail);

        } else if (ch == 4) {
            int sem = read_int("Semester number (1-8): ");
            if (sem < 1 || sem > MAX_SEMS) { printf("Invalid semester.\n"); continue; }
            SemResult r;
            if (load_sem_result(sem, mail, &r))
                print_sem_result(&r);
            else
                printf("No result found for Semester %d.\n", sem);
        }
    }
}

/* ═══════════════════════════ Admin login ══════════════════════════ */

static bool verify_secondary_admin(const char *id, const char *pwd) {
    char filepath[MAX_MAIL + 8];
    admin_filename(id, filepath, sizeof(filepath));
    FILE *f = fopen(filepath, "r");
    if (!f) return false;

    char fid[MAX_MAIL], fpwd[MAX_PWD];
    bool ok = false;
    if (fgets(fid,  sizeof(fid),  f) &&
        fgets(fpwd, sizeof(fpwd), f)) {
        strip_newline(fid); strip_newline(fpwd);
        ok = (strcmp(fid, id) == 0 && strcmp(fpwd, pwd) == 0);
    }
    fclose(f);
    return ok;
}

static void admin_login(void) {
    char id[MAX_MAIL], pwd[MAX_PWD];

    while (1) {
        printf("\n=== Admin Login ===\n");
        printf("1. Main admin\n");
        printf("2. Secondary admin\n");
        printf("3. Back\n");
        int type = read_int("Choice: ");

        if (type == 3) return;
        if (type != 1 && type != 2) { printf("Invalid choice.\n"); continue; }

        read_string("Admin ID : ", id,  sizeof(id));
        read_string("Password : ", pwd, sizeof(pwd));

        if (type == 1) {
            if (strcmp(id, MAIN_ADMIN_ID) == 0 && strcmp(pwd, MAIN_ADMIN_PWD) == 0) {
                printf("Main admin login successful.\n");
                printf("1. Manage secondary admins\n");
                printf("2. Go to dashboard\n");
                int sub = read_int("Choice: ");
                if (sub == 1) manage_admin();
                admin_dashboard();
                return;
            }
            printf("Incorrect main admin credentials.\n");
        } else {
            if (verify_secondary_admin(id, pwd)) {
                printf("Secondary admin login successful.\n");
                admin_dashboard();
                return;
            }
            printf("Incorrect credentials or no access. Contact main admin.\n");
        }
    }
}

/* ═══════════════════════════ Student register ═════════════════════ */

static void student_register(void) {
    char mail[MAX_MAIL], pwd1[MAX_PWD], pwd2[MAX_PWD], name[MAX_NAME];
    int  yoga, sports, lucky;

    printf("\n=== Student Registration ===\n");

    while (1) {
        read_string("Institute email : ", mail, sizeof(mail));
        FILE *chk = fopen(mail, "r");
        if (chk) { fclose(chk); printf("Already registered. Please login.\n"); return; }

        read_string("Password        : ", pwd1, sizeof(pwd1));
        read_string("Confirm password: ", pwd2, sizeof(pwd2));
        if (strcmp(pwd1, pwd2) != 0) { printf("Passwords don't match.\n"); continue; }
        break;
    }

    printf("\nSecurity questions (1 = Yes, 2 = No):\n");
    yoga   = read_int("Do you like yoga?      ");
    sports = read_int("Do you play any sport? ");
    lucky  = read_int("Your lucky number      ");

    FILE *f = fopen(mail, "w");
    if (!f) { printf("Error creating account file.\n"); return; }
    fprintf(f, "%s\n%s\n%d\n%d\n%d\n", mail, pwd1, yoga, sports, lucky);
    fclose(f);

    read_string("\nYour name (use _ for spaces): ", name, sizeof(name));
    FILE *sl = fopen(STUDENTS_FILE, "a");
    if (sl) { fprintf(sl, "%s\t%s\n", name, mail); fclose(sl); }

    printf("Registration successful!\n");
}

/* ═══════════════════════════ Student login ════════════════════════ */

static bool student_authenticate(const char *mail, const char *pwd) {
    FILE *f = fopen(mail, "r");
    if (!f) { printf("No account found for this email.\n"); return false; }

    char fm[MAX_MAIL], fp[MAX_PWD];
    bool ok = false;
    if (fgets(fm, sizeof(fm), f) && fgets(fp, sizeof(fp), f)) {
        strip_newline(fm); strip_newline(fp);
        ok = (strcmp(fm, mail) == 0 && strcmp(fp, pwd) == 0);
    }
    fclose(f);
    return ok;
}

static void student_dashboard(const char *mail) {
    while (1) {
        printf("\n╔══════════════════════════╗\n");
        printf("║    Student Dashboard     ║\n");
        printf("╚══════════════════════════╝\n");
        printf("1. View all semester results\n");
        printf("2. View specific semester result\n");
        printf("3. Logout\n");
        int ch = read_int("Choice: ");

        if (ch == 3) return;

        if (ch == 1) {
            view_all_sems(mail);
        } else if (ch == 2) {
            int sem = read_int("Semester number (1-8): ");
            if (sem < 1 || sem > MAX_SEMS) { printf("Invalid semester.\n"); continue; }
            SemResult r;
            if (load_sem_result(sem, mail, &r))
                print_sem_result(&r);
            else
                printf("No result found for Semester %d. Contact admin.\n", sem);
        } else {
            printf("Invalid choice.\n");
        }
    }
}

static void student_login(void) {
    char mail[MAX_MAIL], pwd[MAX_PWD];
    printf("\n=== Student Login ===\n");
    read_string("Institute email: ", mail, sizeof(mail));
    read_string("Password       : ", pwd,  sizeof(pwd));

    if (!student_authenticate(mail, pwd)) {
        printf("Invalid email or password.\n");
        return;
    }
    printf("Login successful! Welcome.\n");
    student_dashboard(mail);
}

/* ═══════════════════════════ Forgot password ══════════════════════ */

static void student_forgot_password(void) {
    char mail[MAX_MAIL];
    printf("\n=== Password Reset ===\n");
    read_string("Institute email: ", mail, sizeof(mail));

    FILE *f = fopen(mail, "r");
    if (!f) { printf("No account found.\n"); return; }

    char sm[MAX_MAIL], sp[MAX_PWD], line[MAX_LINE];
    int  sy, ss, sl_val;

    bool ok =
        fgets(sm,   sizeof(sm),   f) &&
        fgets(sp,   sizeof(sp),   f) &&
        fgets(line, sizeof(line), f) && sscanf(line, "%d", &sy)     == 1 &&
        fgets(line, sizeof(line), f) && sscanf(line, "%d", &ss)     == 1 &&
        fgets(line, sizeof(line), f) && sscanf(line, "%d", &sl_val) == 1;
    fclose(f);

    if (!ok) { printf("Error reading account data.\n"); return; }
    strip_newline(sm); strip_newline(sp);

    printf("Answer your security questions:\n");
    int yoga   = read_int("Do you like yoga?      (1=Yes/2=No): ");
    int sports = read_int("Do you play any sport? (1=Yes/2=No): ");
    int lucky  = read_int("Your lucky number                  : ");

    if (yoga != sy || sports != ss || lucky != sl_val) {
        printf("Security answers do not match.\n");
        return;
    }

    char pwd1[MAX_PWD], pwd2[MAX_PWD];
    while (1) {
        read_string("New password    : ", pwd1, sizeof(pwd1));
        read_string("Confirm password: ", pwd2, sizeof(pwd2));
        if (strcmp(pwd1, pwd2) == 0) break;
        printf("Passwords don't match.\n");
    }

    FILE *fw = fopen(mail, "w");
    if (!fw) { printf("Error updating password.\n"); return; }
    fprintf(fw, "%s\n%s\n%d\n%d\n%d\n", sm, pwd1, sy, ss, sl_val);
    fclose(fw);
    printf("Password reset successful!\n");
}

/* ═══════════════════════════ Main ═════════════════════════════════ */

int main(void) {
    printf("╔══════════════════════════════════════════╗\n");
    printf("║   IIT Indore Student Academic Portal     ║\n");
    printf("╚══════════════════════════════════════════╝\n");

    while (1) {
        printf("\n--- Main Menu ---\n");
        printf("1. Student Login\n");
        printf("2. Student Register\n");
        printf("3. Forgot Password\n");
        printf("4. Admin Login\n");
        printf("5. Exit\n");
        int choice = read_int("Your choice: ");

        switch (choice) {
            case 1: student_login();           break;
            case 2: student_register();        break;
            case 3: student_forgot_password(); break;
            case 4: admin_login();             break;
            case 5: printf("Goodbye!\n"); return 0;
            default: printf("Invalid choice (1-5).\n");
        }
    }
}
