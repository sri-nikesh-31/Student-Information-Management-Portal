# Student Information Management Portal

A console-based Student Academic Portal developed in **C** that allows students to securely access their academic records while providing administrators with tools to manage semester-wise results.

---

## Features

### Student Features

* Student Registration
* Student Login
* Password Recovery using Security Questions
* View Semester-wise Results
* View Complete Academic History

### Admin Features

* Main Admin and Secondary Admin Login
* Add and Manage Secondary Admins
* Enter Student Results
* Modify Existing Results
* Add, Edit, and Delete Subjects
* View Student Academic Records

### Academic Management

* Semester-wise Result Storage
* Subject-wise Credits and Grades
* Automatic SPI Calculation
* Support for up to 8 Semesters

---

## Technologies Used

* C Programming Language
* File Handling
* Structures
* Arrays
* String Manipulation
* Authentication System

---

## Project Structure

```text
.
├── student_portal.c
├── studentslist.txt
├── sem_1_student@iiti.ac.in
├── sem_2_student@iiti.ac.in
├── admin@iiti.ac.in.txt
└── README.md
```

---

## File Formats

### Student Account File

Filename:

```text
student@iiti.ac.in
```

Contents:

```text
Email
Password
Yoga Answer
Sports Answer
Lucky Number
```

### Semester Result File

Filename:

```text
sem_<semester>_<email>
```

Example:

```text
sem_1_student@iiti.ac.in
```

Contents:

```text
SUBJ|Mathematics|4|10
SUBJ|Physics|3|9
SUBJ|Programming|4|10
SPI|9.55
```

### Admin File

Filename:

```text
<admin_id>.txt
```

Contents:

```text
Admin ID
Password
```

---

## Compilation

```bash
gcc student_portal.c -o student_portal
```

---

## Execution

### Linux / macOS

```bash
./student_portal
```

### Windows

```bash
student_portal.exe
```

---

## Default Main Admin Credentials

```text
Admin ID : admin@iiti.ac.in
Password : admin_123
```

---

## Concepts Demonstrated

* File Handling
* Structures
* Functions
* Authentication System
* CRUD Operations
* Data Persistence
* Menu-Driven Programming
* Modular Design

---

## Future Enhancements

* CGPA Calculation
* Attendance Management
* Student Profile Management
* Database Integration (MySQL)
* GUI Version
* Course Registration System

---

## Author

**Sri Nikesh Ayinala**
Mathematics and Computing
Indian Institute of Technology Indore
