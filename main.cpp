#include <iostream>
#include <string>
#include <mysqlx/xdevapi.h>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <random>



using namespace std;
using namespace mysqlx;

mysqlx::Session createSession();
mysqlx::Table getUsersTable(mysqlx::Session& sess);
mysqlx::Table getPasswordsTable(mysqlx::Session& sess);

std::string sha256(const std::string& input);
std::string generateSalt(size_t length = 16);

void printMenuOptions();
void Menu(Session& sess,Table& passwords,mysqlx::Table& users);
void Signup(mysqlx::Session& sess, mysqlx::Table& users);
void Signin(Session& sess,Table& passwords,mysqlx::Table& users);
void howstrongismypassword();
void Info();

void EditPasswords(Session& sess, Table& passwords, int userId);

void ListPasswords(Session& sess, Table& passwords,int userId);
void AddPassword(Session& sess, Table& passwords, int userId);
void DeletePassword(Session& sess, Table& passwords, int userId);
void ChangePassword(Session& sess, Table& passwords, int userId);


bool isCommonPassword(const std::string & pwd);
bool hasConsecutivePattern(const std::string & pwd);
bool hasRepeatedChars(const std::string& pwd);
bool isPasswordStrong(const std::string& password);

int CheckCredentials(Session& sess,const std::string& username, const std::string& password);

int main() {
    try {

        mysqlx::Session sess = createSession();
        mysqlx::Table users = getUsersTable(sess);
        mysqlx::Table passwords = getPasswordsTable(sess);
        Menu(sess,passwords,users);
    }
    catch (const mysqlx::Error &err) {
        std::cerr << "MySQL Error: " << err.what() << std::endl;
        return 1;
    }
    catch (const std::exception &ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown error occurred!" << std::endl;
        return 1;
    }
    return 0;
}


void printMenuOptions() {
    cout << "Please enter the number of the operation:\n" << endl;
    cout << "\t\t1 - Sign up\t\t||\t ";
    cout << "\t2 - Sign in\t\t||\t ";
    cout << "\t3 - How strong is my password\t||\t ";
    cout << "4 - Info\t\t||\t ";
    cout << "5 - Exit\n" << endl;
}

void Menu(Session& sess, Table& passwords, mysqlx::Table& users) {
    int f;
    std::string decision;
    cout << "\n\t\t\t-------------------------------------------Welcome -------------------------------------------" << endl;

    do {
        printMenuOptions();

        while (true) {
            cout << "Choice: ";
            cin >> f;

            if (cin.fail()) {
                cin.clear();
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                cout << "\nYanlış tuşladınız, tekrar deneyin.\n";
                printMenuOptions();
                continue;
            }

            if (f >= 1 && f <= 5) {
                break;
            } else {
                cout << "\nYanlış tuşladınız, tekrar deneyin.\n";
                printMenuOptions();
            }
        }

        if (f == 5) {
            cout << "Program sonlandırılıyor...\n";
            return;
        }

        switch (f) {
            case 1:
                Signup(sess, users);
                break;
            case 2:
                Signin(sess, passwords, users);
                break;
            case 3:
                howstrongismypassword();
                break;
            case 4:
                Info();
                break;
        }

        cout << "\nDo you want to continue? (y/n): ";
        cin >> decision;

    } while (decision == "y" || decision == "Y" || decision == "yes" || decision == "YES");

    cout << "Exiting...\n";
}



void Signup(mysqlx::Session& sess, mysqlx::Table& users) {
    std::string username;
    std::string password;
    bool usernameTaken = true;
    bool passwordValid = false;

    try {
        Schema db = sess.getSchema("Mypasswords");
        Table users = db.getTable("users");

        while (usernameTaken) {
            std::cout << "Create your Username: ";
            std::cin >> username;

            RowResult res = users.select("username")
                    .where("username = :username")
                    .bind("username", username)
                    .execute();

            if (res.count() > 0) {
                std::cout << "Username already taken. Please choose a different one." << std::endl;
            } else {
                usernameTaken = false;
            }
        }

        while (!passwordValid) {
            std::cout << "Create your password (Must include uppercase, lowercase, number, special char, and be at least 12 chars): ";
            std::cin >> password;

            if (!isPasswordStrong(password)) {
                std::cout << "Weak password. Must include uppercase, lowercase, number, special char, and be at least 12 chars." << std::endl;
            } else {
                passwordValid = true;
            }
        }

        std::string salt = generateSalt();
        std::string saltedPassword = password + salt;
        std::string hashedPassword = sha256(saltedPassword);


        users.insert("username", "password", "salt")
                .values(username, hashedPassword, salt)
                .execute();

        std::cout << "Signup successful! User added to database." << std::endl;

    } catch (const mysqlx::Error& err) {
        std::cerr << "MySQL Error: " << err.what() << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown error occurred!" << std::endl;
    }
}

bool isPasswordStrong(const std::string & password) {
    if (password.length() < 12) return false;

    bool has_upper = false, has_lower = false, has_digit = false, has_special = false;

    for (char ch : password) {
        if (isupper(ch)) has_upper = true;
        else if (islower(ch)) has_lower = true;
        else if (isdigit(ch)) has_digit = true;
        else has_special = true;
    }

    return has_upper && has_lower && has_digit && has_special;
}


void Signin(Session& sess, Table& passwords, mysqlx::Table& users) {
    std::string username, password;
    int attempts = 3;

    while (attempts > 0) {
        std::cout << "Enter your username: ";
        std::cin >> username;
        std::cout << "Enter your password: ";
        std::cin >> password;

        int userId = CheckCredentials(sess, username, password);

        if (userId != -1) {
            std::cout << "Welcome " << username << "!" << std::endl;
            EditPasswords(sess, passwords, userId);
            return;
        }

        attempts--;
        std::cout << "Invalid username or password. Attempts left: " << attempts << std::endl;
    }

    std::cout << "Too many failed attempts. Try again later." << std::endl;
}




void howstrongismypassword() {
    cout << "Now type a password to see if it is secure:\n";
    std::string password;
    cin >> password;

    int score = 0;

    bool has_upper = false;
    bool has_lower = false;
    bool has_digit = false;
    bool has_symbol = false;

    for (char ch : password) {
        if (isupper(ch)) has_upper = true;
        else if (islower(ch)) has_lower = true;
        else if (isdigit(ch)) has_digit = true;
        else has_symbol = true;
    }

    if (password.length() >= 8) score++;
    if (password.length() >= 12) score++;
    if (has_upper) score++;
    if (has_lower) score++;
    if (has_digit) score++;
    if (has_symbol) score++;
    if (!isCommonPassword(password)) score++;
    if (!hasRepeatedChars(password)) score++;
    if (!hasConsecutivePattern(password)) score++;

    if (has_upper && has_lower && has_digit && has_symbol)
        score++; // bonus

    // Result
    cout << "Password security score: " << score << "/10" << endl;

    if (score <= 4)
        cout << " Weak password. Consider changing it.\n";
    else if (score <= 7)
        cout << "Moderate password. Can be improved.\n";
    else
        cout << "Strong password. Well done!\n";
}


void Info(){
    cout<<"This is my solo project where I built MyPasswords.\n"
          "My influence was to get better and gain experience while I work on this project.\n"
          "I tried to link my coding skills with database skills and mix them with \n"
          "security essentials/applications to finally showcase them in a steady environment \n "<<endl;
}
bool isCommonPassword(const std::string & pwd) {
    std::string commons[] = {"password", "123456", "qwerty", "admin", "abc123"};
    for (std::string word : commons) {
        if (pwd == word) return true;
    }
    return false;
}

bool hasConsecutivePattern(const std::string & pwd) {
    for (size_t i = 0; i < pwd.length() - 2; ++i) {
        if (pwd[i+1] == pwd[i] + 1 && pwd[i+2] == pwd[i] + 2)
            return true;
    }
    return false;
}

bool hasRepeatedChars(const std::string& pwd) {
    for (size_t i = 0; i < pwd.length() - 1; ++i) {
        if (pwd[i] == pwd[i+1])
            return true;
    }
    return false;
}

int CheckCredentials(mysqlx::Session& sess, const std::string& username, const std::string& password) {
    try {
        Schema db = sess.getSchema("Mypasswords");
        Table users = db.getTable("users");


        RowResult res = users.select("id", "password", "salt")
                .where("username = :username")
                .bind("username", username)
                .execute();

        if (res.count() == 0) {
            return -1;
        }

        Row row = res.fetchOne();
        int userId = row[0];
        std::string storedHash = row[1].get<std::string>();
        std::string storedSalt = row[2].get<std::string>();



        std::string hashedInput = sha256(password + storedSalt);


        if (hashedInput == storedHash) {
            return userId;
        } else {
            return -1;
        }

    } catch (const mysqlx::Error& err) {
        std::cerr << "MySQL Error: " << err.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "Unknown error occurred!" << std::endl;
        return -1;
    }
}



void EditPasswords(Session& sess, Table& passwords, int userId) {
    while (true) {
        std::cout << "\nPassword Menu:\n";
        std::cout << "1. List Passwords\n";
        std::cout << "2. Add Password\n";
        std::cout << "3. Delete Password\n";
        std::cout << "4. Change Password\n";
        std::cout << "5. Exit\n";
        std::cout << "Choice: ";

        int choice;
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
            case 1:
                ListPasswords(sess, passwords, userId);
                break;
            case 2:
                AddPassword(sess, passwords, userId);
                break;
            case 3:
                DeletePassword(sess, passwords, userId);
                break;
            case 4:
                ChangePassword(sess, passwords, userId);
                break;
            case 5:
                return;
            default:
                std::cout << "Invalid choice.\n";
        }
    }
}


void AddPassword(Session& sess, Table& passwords, int userId) {
    std::string name, mail, password;

    std::cout << "Enter name: ";
    std::cin>>name;
    std::cout << "Enter mail: ";
    std::cin>>mail;
    std::cout << "Enter password: ";
    std::cin>>password;

    try {
        passwords.insert("user_id", "name", "mail", "password")
                .values(userId, name, mail, password)
                .execute();

        std::cout << "Password added successfully.\n";
    } catch (const mysqlx::Error& err) {
        std::cerr << "MySQL Error: " << err.what() << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown error occurred!\n";
    }
}


void ListPasswords(Session& sess, Table& passwords,int userId) {
    try {
        auto session = createSession();
        auto passwords = session.getSchema("MyPasswords").getTable("Passwords");

        auto res = passwords.select("name", "mail", "password")
                .where("user_id = :uid")
                .bind("uid", userId)
                .execute();

        std::cout << "\n--- Saved Passwords ---\n";
        for (const auto& row : res)
            std::cout << "Name: " << row[0]
                      << " | Email: " << row[1]
                      << " | Password: " << row[2] << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }
}


void DeletePassword(Session& sess, Table& passwords, int userId) {
    std::string nameToDelete;
    std::cout << "Enter the name of the password you want to delete: ";
    std::cin >> nameToDelete;
    std::cin.ignore();  // newline temizleme

    try {

        Result res = passwords.remove()
                .where("name = :name AND user_id = :uid")
                .bind("name", nameToDelete)
                .bind("uid", userId)
                .execute();

        if (res.getAffectedItemsCount() > 0) {
            std::cout << "Password "<<nameToDelete<<" deleted  successfully.\n";
        } else {
            std::cout << "No matching password found for deletion.\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error deleting password: " << e.what() << std::endl;
    }
}


void ChangePassword(Session& sess, Table& passwords, int userId) {
    std::string nameToUpdate;
    std::string newPassword;

    std::cout << "Enter the name of the password you want to change: ";
    std::cin >> nameToUpdate;
    std::cout << "Enter the new password: ";
    std::cin >> newPassword;
    std::cin.ignore();



    try {
        Result res = passwords.update()
                .set("password", newPassword)
                .where("name = :name AND user_id = :uid")
                .bind("name", nameToUpdate)
                .bind("uid", userId)
                .execute();

        if (res.getAffectedItemsCount() > 0) {
            std::cout << "Password updated successfully.\n";
        } else {
            std::cout << "Password not found or you don't have permission.\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error updating password: " << e.what() << std::endl;
    }
}



mysqlx::Session createSession() {
    return mysqlx::Session("localhost", 33060, "root", "B.isnotmyname30");
}
mysqlx::Table getUsersTable(mysqlx::Session& sess) {
    return sess.getSchema("Mypasswords").getTable("users");
}
mysqlx::Table getPasswordsTable(mysqlx::Session& sess) {
    return sess.getSchema("Mypasswords").getTable("passwords");
}

std::string generateSalt(size_t length ) {
    const std::string chars =
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, chars.size() - 1);

    std::string salt;
    for (size_t i = 0; i < length; ++i)
        salt += chars[dist(gen)];

    return salt;
}

std::string sha256(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];

    return ss.str();
}





