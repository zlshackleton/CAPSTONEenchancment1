#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <sqlite3.h>

using namespace std;

class CourseDatabase {
private:
    sqlite3* db;

public:
    // Constructor: Runs automatically when an object is created
    CourseDatabase() {
        int rc = sqlite3_open("courses.db", &db);
        if (rc) {
            cerr << "Error opening database: " << sqlite3_errmsg(db) << endl;
        }
        else {
            cout << "Database connected successfully!" << endl;
        }
    }

    /*
     *
     * Method to create new table upon program start
     *
    */
    void createTables() {
        char* errMsg = 0;
        const char* sql = "CREATE TABLE IF NOT EXISTS courses (id INTEGER PRIMARY KEY, name TEXT);";

        int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            cerr << "Table creation failed: " << errMsg << endl;
        }
        else {
            cout << "Table created successfully!" << endl;
        }
    }

    /*
     *
     * Method to insert course data into database. Expirience Required
     *
    */
    void insertCourse() {
        int prerequisites;
        string newCourseNum, newCourseTitle, newCoursePrereq;
        char* errMsg = 0;

        // User Input
        cout << "Enter Course Number: " << endl;
        cin >> newCourseNum;

        cout << "Enter Course Title: " << endl;
        cin.ignore();   // Allows for multple words that wont be broken by whitespace
        getline(cin, newCourseTitle);

        // Handle Prerequsites input
        cout << "Enter the number of Prerequisites the course has (0, 1, or 2): " << endl;
        cin >> prerequisites;

        // Handle number of prereqs 
        if (prerequisites == 0) {
            newCoursePrereq = ""; // SQL stores as NULL
        }
        else if (prerequisites == 1) {
            cout << "Enter Prerequisite: " << endl;
            cin >> newCoursePrereq;
        }
        else if (prerequisites == 2) {
            string prereq1, prereq2;
            cout << "Enter first Prerequisite: " << endl;
            cin >> prereq1;
            cout << "Enter second Prerequisite: " << endl;
            cin >> prereq2;

            newCoursePrereq = prereq1 + ", " + prereq2;
        }

        // Prepared SQL placeholders "?"
        const char* sql = "INSERT INTO courses (courseNumber, courseTitle, prerequisite) VALUES (?, ?, ?)";
        sqlite3_stmt* stmt;

        // Prepare SQL statement
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            cerr << "Prepared Statement failed" << sqlite3_errmsg(db) << endl;
            return;
        }

        // Bind values to the placeholders "?"
        sqlite3_bind_text(stmt, 1, newCourseNum.c_str(), -1, SQLITE_STATIC); // Bind newCourseNum
        sqlite3_bind_text(stmt, 2, newCourseTitle.c_str(), -1, SQLITE_STATIC); // Bind newCourseTitle
        sqlite3_bind_text(stmt, 3, newCoursePrereq.c_str(), -1, SQLITE_STATIC); // Bind newCoursePrereq

        // Execute stmt
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            cerr << "Error inserting course: " << sqlite3_errmsg(db) << endl;
        }
        else {
            cout << "Course inserted successfully!" << endl;
        }

        // CLEAN
        sqlite3_finalize(stmt);
    }

    /*
     *
     * Method to lookup a singluar course by its Course Number [ex. CS101]
     *
    */
    void lookupCourse() {
        string newCourseNum;
        char* errMsg = 0;

        // User Input
        cout << "Enter Course Number: " << endl;
        cin >> newCourseNum;

        const char* sql = "SELECT * FROM courses WHERE courseNumber = ?";
        sqlite3_stmt* stmt;

        // Prepare SQL statement
        int prep_rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (prep_rc != SQLITE_OK) {
            cerr << "Prepared Statement failed" << sqlite3_errmsg(db) << endl;
            return;
        }

        sqlite3_bind_text(stmt, 1, newCourseNum.c_str(), -1, SQLITE_STATIC); // Bind newCourseNum

        // Execute stmt
        int step_rc;
        step_rc = sqlite3_step(stmt);
        if (step_rc == SQLITE_ROW) {
            // Extract and display course data
            const char* courseNumber = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            const char* courseTitle = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            const char* prerequisites = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

            string result = string(courseNumber) + ", " + string(courseTitle) + ", " + string(prerequisites);
            cout << result << endl;  // Print the result as a whole line
        }
        else if (step_rc == SQLITE_DONE) {
            cout << "No course found with that course number." << endl;
        }
        else {
            cerr << "Error executing statement: " << sqlite3_errmsg(db) << endl;
        }

        // Finalize the statement
        sqlite3_finalize(stmt);
    }

    /*
     *
     * Method to showcase all data in database
     *
    */
    void displayCourses() {
        const char* sql = "SELECT * FROM courses;";
        sqlite3_stmt* stmt;

        // Prepare SQL Statement
        int prep_rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (prep_rc != SQLITE_OK) {
            cerr << "Prepared Statement failed" << sqlite3_errmsg(db) << endl;
            return;
        }

        //Loop to step through each row
        int step_rc = sqlite3_step(stmt);
        while (step_rc == SQLITE_ROW) {

            // Obtain course data through column indicies
            const char* courseNumber = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            const char* courseTitle = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            const char* prerequisites = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

            // Print out row data in cout format
            cout << courseNumber << " | " << courseTitle << " | " << prerequisites << endl;
        }

        // CLEAN
        sqlite3_finalize(stmt);
    }

    /*
     *
     * Method used to run our menu for the user
     *
    */
    void runMenu(CourseDatabase& db) {
        db.createTables();   // Create the tables automatically

        int choice;
        do {
            cout << "\n===== Advisor Course Program =====" << endl;
            cout << "1. Insert a new course" << endl;
            cout << "2. Show one course" << endl;
            cout << "3. Show all courses" << endl;
            cout << "4. Exit Program" << endl;
            cout << "Enter your choice (1-4): " << endl;
            cin >> choice;

            switch (choice) {
            case 1:
                db.insertCourse();  // Call insertCourse as a method on the db object
                break;
            case 2:
                db.lookupCourse();  // Add lookupCourse here as needed
                break;
            case 3:
                db.displayCourses();  // Call displayCourses as a method on the db object
                break;
            case 4:
                cout << "Exiting Program..." << endl;
                break;
            default:
                cout << "Invalid choice. Please enter numbers 1 though 4." << endl;
                break;
            }
        } while (choice != 4);
    }
};

int main() {
    CourseDatabase database;
    database.runMenu(database);
    return 0;
}

        