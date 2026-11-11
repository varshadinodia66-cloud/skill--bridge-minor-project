#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>

using namespace std;

// A simple skill record.
class Skill {
private:
    string name;
    string level;

public:
    Skill() = default;
    Skill(const string& skillName, const string& skillLevel)
        : name(skillName), level(skillLevel) {}

    string getName() const { return name; }
    string getLevel() const { return level; }

    void display() const {
        cout << name << " (" << level << ")";
    }
};

// Base class: common information for every student.
class Student {
protected:
    int id;
    string name;
    string course;
    string email;
    vector<Skill> skillsOffered;
    vector<string> skillsWanted;

public:
    Student() : id(0) {}

    Student(int studentId, const string& studentName,
            const string& studentCourse, const string& studentEmail)
        : id(studentId), name(studentName),
          course(studentCourse), email(studentEmail) {}

    virtual ~Student() = default;

    int getId() const { return id; }
    string getName() const { return name; }
    string getEmail() const { return email; }
    const vector<Skill>& getSkillsOffered() const { return skillsOffered; }
    const vector<string>& getSkillsWanted() const { return skillsWanted; }

    void addOfferedSkill(const Skill& skill) {
        skillsOffered.push_back(skill);
    }

    void addWantedSkill(const string& skill) {
        skillsWanted.push_back(skill);
    }

    virtual string getRole() const {
        return "Student";
    }

    // Virtual function demonstrates runtime polymorphism.
    virtual void displayProfile() const {
        cout << "\n----------------------------------------\n";
        cout << "Student ID : " << id << "\n";
        cout << "Name       : " << name << "\n";
        cout << "Course     : " << course << "\n";
        cout << "Email      : " << email << "\n";
        cout << "Profile    : " << getRole() << "\n";
        cout << "Skills offered: ";
        if (skillsOffered.empty()) cout << "None";
        for (size_t i = 0; i < skillsOffered.size(); ++i) {
            if (i) cout << ", ";
            skillsOffered[i].display();
        }
        cout << "\nSkills wanted : ";
        if (skillsWanted.empty()) cout << "None";
        for (size_t i = 0; i < skillsWanted.size(); ++i) {
            if (i) cout << ", ";
            cout << skillsWanted[i];
        }
        cout << "\n----------------------------------------\n";
    }

    string serialize() const {
        ostringstream out;
        out << id << "|" << name << "|" << course << "|" << email << "|";
        for (size_t i = 0; i < skillsOffered.size(); ++i) {
            if (i) out << ",";
            out << skillsOffered[i].getName() << "~" << skillsOffered[i].getLevel();
        }
        out << "|";
        for (size_t i = 0; i < skillsWanted.size(); ++i) {
            if (i) out << ",";
            out << skillsWanted[i];
        }
        return out.str();
    }

    void loadSkills(const string& offered, const string& wanted) {
        string item;
        stringstream offeredStream(offered);
        while (getline(offeredStream, item, ',')) {
            size_t splitAt = item.find('~');
            if (splitAt != string::npos) {
                skillsOffered.emplace_back(item.substr(0, splitAt),
                                           item.substr(splitAt + 1));
            }
        }

        stringstream wantedStream(wanted);
        while (getline(wantedStream, item, ',')) {
            if (!item.empty()) skillsWanted.push_back(item);
        }
    }
};

// Derived class: a student who is willing to mentor others.
class MentorStudent : public Student {
public:
    MentorStudent() = default;
    MentorStudent(int studentId, const string& studentName,
                  const string& studentCourse, const string& studentEmail)
        : Student(studentId, studentName, studentCourse, studentEmail) {}

    string getRole() const override {
        return "Peer Mentor";
    }

    void displayProfile() const override {
        Student::displayProfile();
        cout << "This student is open to helping peers learn.\n";
    }
};

// Stores one request sent by a student to a peer mentor.
class MentorshipRequest {
private:
    int requestId;
    int requesterId;
    int mentorId;
    string skill;
    string status;

public:
    MentorshipRequest() : requestId(0), requesterId(0), mentorId(0) {}

    MentorshipRequest(int rid, int from, int to,
                      const string& skillName, const string& requestStatus)
        : requestId(rid), requesterId(from), mentorId(to),
          skill(skillName), status(requestStatus) {}

    int getRequestId() const { return requestId; }

    void display() const {
        cout << "Request #" << requestId
             << " | From student ID: " << requesterId
             << " | Mentor ID: " << mentorId
             << " | Skill: " << skill
             << " | Status: " << status << "\n";
    }

    string serialize() const {
        return to_string(requestId) + "|" + to_string(requesterId) + "|" +
               to_string(mentorId) + "|" + skill + "|" + status;
    }
};

// Main application class. It manages students, requests and file storage.
class SkillBridge {
private:
    vector<Student> students;
    vector<MentorshipRequest> requests;
    const string STUDENT_FILE = "students.txt";
    const string REQUEST_FILE = "requests.txt";

    int nextStudentId() const {
        int largest = 0;
        for (const auto& s : students) largest = max(largest, s.getId());
        return largest + 1;
    }

    int nextRequestId() const {
        int largest = 0;
        for (const auto& r : requests) largest = max(largest, r.getRequestId());
        return largest + 1;
    }

    Student* findStudent(int studentId) {
        for (auto& s : students)
            if (s.getId() == studentId) return &s;
        return nullptr;
    }

    void saveStudents() const {
        ofstream file(STUDENT_FILE);
        for (const auto& s : students) file << s.serialize() << "\n";
    }

    void saveRequests() const {
        ofstream file(REQUEST_FILE);
        for (const auto& r : requests) file << r.serialize() << "\n";
    }

    void loadData() {
        ifstream studentInput(STUDENT_FILE);
        string line;
        while (getline(studentInput, line)) {
            stringstream ss(line);
            string idText, studentName, course, email, offered, wanted;
            if (!getline(ss, idText, '|') || !getline(ss, studentName, '|') ||
                !getline(ss, course, '|') || !getline(ss, email, '|') ||
                !getline(ss, offered, '|') || !getline(ss, wanted)) continue;

            try {
                Student s(stoi(idText), studentName, course, email);
                s.loadSkills(offered, wanted);
                students.push_back(s);
            } catch (...) {
                // Ignore an incomplete or damaged record.
            }
        }

        ifstream requestInput(REQUEST_FILE);
        while (getline(requestInput, line)) {
            stringstream ss(line);
            string rid, from, to, skill, status;
            if (getline(ss, rid, '|') && getline(ss, from, '|') &&
                getline(ss, to, '|') && getline(ss, skill, '|') &&
                getline(ss, status)) {
                try {
                    requests.emplace_back(stoi(rid), stoi(from), stoi(to),
                                           skill, status);
                } catch (...) {
                    // Ignore an incomplete or damaged record.
                }
            }
        }
    }

    static string readLine(const string& prompt) {
        string value;
        cout << prompt;
        getline(cin, value);
        return value;
    }

    static int readInt(const string& prompt) {
        int value;
        while (true) {
            cout << prompt;
            if (cin >> value) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return value;
            }
            cout << "Please enter a valid number.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }

public:
    SkillBridge() {
        loadData();
    }

    void registerStudent() {
        cout << "\n--- Student Registration ---\n";
        string name = readLine("Full name: ");
        string course = readLine("Course / semester: ");
        string email = readLine("Email: ");

        if (name.empty() || course.empty() || email.empty()) {
            cout << "Please fill in all the basic details.\n";
            return;
        }

        int id = nextStudentId();
        Student student(id, name, course, email);

        cout << "\nAdd skills you can teach (leave blank to stop).\n";
        while (true) {
            string skillName = readLine("Skill name: ");
            if (skillName.empty()) break;
            string level = readLine("Your level (Beginner/Intermediate/Advanced): ");
            if (level.empty()) level = "Not specified";
            student.addOfferedSkill(Skill(skillName, level));
        }

        cout << "\nAdd skills you want to learn (leave blank to stop).\n";
        while (true) {
            string skill = readLine("Skill wanted: ");
            if (skill.empty()) break;
            student.addWantedSkill(skill);
        }

        students.push_back(student);
        saveStudents();
        cout << "\nRegistration successful. Your student ID is " << id << ".\n";
        cout << "Keep this ID for future requests.\n";
    }

    void showAllStudents() const {
        cout << "\n--- Registered Students ---\n";
        if (students.empty()) {
            cout << "No students registered yet.\n";
            return;
        }
        for (const auto& student : students) student.displayProfile();
    }

    void searchSkill() const {
        string keyword = readLine("\nEnter the skill you want to find: ");
        if (keyword.empty()) return;

        string search = keyword;
        transform(search.begin(), search.end(), search.begin(),
                  [](unsigned char c) { return static_cast<char>(tolower(c)); });

        bool found = false;
        for (const auto& student : students) {
            for (const auto& skill : student.getSkillsOffered()) {
                string skillName = skill.getName();
                transform(skillName.begin(), skillName.end(), skillName.begin(),
                          [](unsigned char c) { return static_cast<char>(tolower(c)); });
                if (skillName.find(search) != string::npos) {
                    cout << "\nStudent ID: " << student.getId()
                         << " | Name: " << student.getName()
                         << " | Email: " << student.getEmail()
                         << " | Skill: ";
                    skill.display();
                    cout << "\n";
                    found = true;
                }
            }
        }
        if (!found) cout << "No matching skill was found.\n";
    }

    void sendRequest() {
        cout << "\n--- Send a Peer Learning Request ---\n";
        int from = readInt("Your student ID: ");
        int to = readInt("Mentor's student ID: ");

        if (from == to) {
            cout << "You cannot send a request to your own profile.\n";
            return;
        }
        if (!findStudent(from) || !findStudent(to)) {
            cout << "One of the student IDs does not exist. Please check again.\n";
            return;
        }

        string skill = readLine("Which skill would you like to learn? ");
        if (skill.empty()) {
            cout << "Skill name cannot be empty.\n";
            return;
        }

        int id = nextRequestId();
        requests.emplace_back(id, from, to, skill, "Pending");
        saveRequests();
        cout << "Request sent successfully. Request ID: " << id << "\n";
    }

    void showRequests() const {
        cout << "\n--- Peer Learning Requests ---\n";
        if (requests.empty()) {
            cout << "There are no requests yet.\n";
            return;
        }
        for (const auto& request : requests) request.display();
    }

    void run() {
        cout << "========================================\n";
        cout << "       SKILLBRIDGE - SKILL EXCHANGE     \n";
        cout << "========================================\n";

        int choice;
        do {
            cout << "\n1. Register student\n";
            cout << "2. View all student profiles\n";
            cout << "3. Search for a skill\n";
            cout << "4. Send a peer learning request\n";
            cout << "5. View all requests\n";
            cout << "0. Exit\n";

            choice = readInt("Choose an option: ");

            switch (choice) {
                case 1: registerStudent(); break;
                case 2: showAllStudents(); break;
                case 3: searchSkill(); break;
                case 4: sendRequest(); break;
                case 5: showRequests(); break;
                case 0: cout << "Thank you for using SkillBridge. Goodbye!\n"; break;
                default: cout << "Invalid option. Please choose again.\n";
            }
        } while (choice != 0);
    }
};

int main() {
    SkillBridge app;
    app.run();
    return 0;
}
