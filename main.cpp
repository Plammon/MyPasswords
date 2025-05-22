#include <iostream>
#include <string>
#include <mysqlx/xdevapi.h>
using namespace std;
using namespace mysqlx;

// Fonksiyon prototipleri
void Menu();
void Signup();
void Signin();
void howstrongismypassword();
void Info();

void EditPasswords(int userId);
void ListPasswords(int userId);
void AddPassword(int userId);
void DeletePassword(int userId);
void ChangePassword(int userId);


bool isCommonPassword(const std::string & pwd);
bool hasConsecutivePattern(const std::string & pwd);
bool hasRepeatedChars(const std::string& pwd);
bool isPasswordStrong(const std::string& password);

int CheckCredentials(const std::string& username, const std::string& password);


int main() {
   try{
       Menu();
    }
    catch (const mysqlx::Error &err) {
        cerr << "Error: " << err.what() << endl;
        return 1;
    }
    catch (exception &ex) {
        cerr << "STD Exception: " << ex.what() << endl;
        return 1;
    }
    catch (...) {
        cerr << "Unknown exception caught" << endl;
        return 1;
    }
    return 0;
}

void Menu() {
    int f;
    cout << "Welcome to my website." << endl;
    cout << "Please choose what you want to do ?:" << endl;
    cout << "1 - Sign up" << endl;
    cout << "2 - Sign in" << endl;
    cout << "3 - How strong is my password"<<endl;
    cout << "4 - Info"<<endl;


    cin >> f;
    if (f == 1) {
        Signup();
    } else if (f == 2) {
       Signin();
    }
    else if (f==3){
        howstrongismypassword();
    }
    else if(f==4){
        Info();
    }
}

void Signup() {
    std::string username;
    std::string password;
    bool usernameTaken = true;
    bool passwordValid = false;

    try {
        // MySQL bağlantısını kur
        Session sess("localhost", 33060, "root", "B.isnotmyname30"); // Şifrenizi buraya ekleyin.
        Schema db = sess.getSchema("Mypasswords"); // Veritabanı adını buraya yazın.
        Table users = db.getTable("users");

        // Kullanıcı adı kontrolü
        while (usernameTaken) {
            cout << "Create your Username: ";
            cin >> username;

            RowResult res = users.select("username")
                    .where("username = :username")
                    .bind("username", username)
                    .execute();

            if (res.count() > 0) {
                cout << "Username already taken. Please choose a different one." << endl;
            } else {
                usernameTaken = false; // Kullanıcı adı alınmamış, döngüden çık
            }
        }

        // Şifre kontrolü
        while (!passwordValid) {
            cout << "Create your password  (Must include uppercase, lowercase, number, special char, and be at least 12 chars.)  : ";
            cin >> password;

            if (!isPasswordStrong(password)) {
                cout << "Weak password. Must include uppercase, lowercase, number, special char, and be at least 12 chars." << endl;
            } else {
                passwordValid = true; // Şifre güçlü, döngüden çık
            }
        }

        // Kullanıcıyı veritabanına ekle
        users.insert("username", "password")
                .values(username, password)
                .execute();

        cout << "Signup successful! User added to database." << endl;

    } catch (const mysqlx::Error &err) {
        cerr << "MySQL Error: " << err.what() << endl;
    } catch (const std::exception &ex) {
        cerr << "Exception: " << ex.what() << endl;
    } catch (...) {
        cerr << "Unknown error occurred!" << endl;
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


void Signin() {
    std::string username, password;
    std::cout << "Enter your username: ";
    std::cin >> username;
    std::cout << "Enter your password: ";
    std::cin >> password;

    int userId = CheckCredentials(username, password);

    if (userId == -1) {
        std::cout << "Invalid username or password." << std::endl;
        return;
    }

    std::cout << "Welcome " << username << "!" << std::endl;

    // Kullanıcının ID'sini EditPasswords fonksiyonuna gönder
    EditPasswords(userId);
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

int CheckCredentials(const std::string& username, const std::string& password) {
    try {
        mysqlx::Session session("localhost", 33060, "root", "B.isnotmyname30");
        mysqlx::Schema db = session.getSchema("MyPasswords");
        mysqlx::Table users = db.getTable("users");

        mysqlx::RowResult res = users.select("id")
                .where("username = :user AND password = :pass")
                .bind("user", username)
                .bind("pass", password)
                .execute();

        if (res.count() == 1) {
            return res.fetchOne().get(0).get<int>();
        }
    } catch (const std::exception& e) {
        std::cerr << "Hata: " << e.what() << std::endl;
    }

    return -1;
}



void EditPasswords(int userId) {
    int i;
    std::cout << "What exactly do you wanna do ?" << std::endl;
    std::cout << "1-List all the passwords" << std::endl;
    std::cout << "2-Add a new password" << std::endl;
    std::cout << "3-Delete a password" << std::endl;
    std::cout << "4-Change password" << std::endl;
    std::cin >> i;

    if (i == 1) {
        ListPasswords(userId);
    } else if (i == 2) {
        AddPassword(userId);
    } else if (i == 3) {
        DeletePassword(userId);
    } else if (i == 4) {
        ChangePassword(userId);
    } else {
        std::cout << "Invalid choice" << std::endl;
    }
}

void AddPassword(int userId) {
    std::string name, email, password;
    std::cout << "Enter name: ";
    std::cin >> name;
    std::cout << "Enter mail: ";
    std::cin >> email;
    std::cout << "Enter password: ";
    std::cin >> password;

    try {
        mysqlx::Session session("localhost", 33060, "root", "B.isnotmyname30");
        mysqlx::Schema db = session.getSchema("MyPasswords");
        mysqlx::Table passwords = db.getTable("passwords");

        passwords.insert("user_id", "name", "mail", "password")
                .values(userId, name, email, password)
                .execute();

        std::cout << "Password added successfully!" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void ListPasswords(int userId) {
    try {
        mysqlx::Session session("localhost", 33060, "root", "B.isnotmyname30");
        mysqlx::Schema db = session.getSchema("MyPasswords");
        mysqlx::Table passwords = db.getTable("Passwords");

        mysqlx::RowResult res = passwords.select("name", "mail", "password")
                .where("user_id = :uid")
                .bind("uid", userId)
                .execute();

        std::cout << "Saved passwords:\n";
        for (mysqlx::Row row : res) {
            std::cout << "Name: " << row[0]
                      << " | Email: " << row[1]
                      << " | Password: " << row[2] << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error listing passwords: " << e.what() << std::endl;
    }
}

void DeletePassword(int userId) {
    std::string nameToDelete;
    std::cout << "Enter the name of the password you want to delete: ";
    std::cin >> nameToDelete;

    try {
        mysqlx::Session session("localhost", 33060, "root", "B.isnotmyname30");
        mysqlx::Schema db = session.getSchema("MyPasswords");
        mysqlx::Table passwords = db.getTable("Passwords");

        // Kullanıcının sadece kendi şifrelerinden, belirttiği isimde olanı sil
        mysqlx::Result res = passwords.remove()
                .where("name = :name AND user_id = :uid")
                .bind("name", nameToDelete)
                .bind("uid", userId)
                .execute();

        if (res.getAffectedItemsCount() > 0) {
            std::cout << "Password with name '" << nameToDelete << "' deleted successfully.\n";
        } else {
            std::cout << "No such password found under your account.\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error deleting password: " << e.what() << std::endl;
    }
}

void ChangePassword(int userId) {
    std::string nameToUpdate;
    std::string newPassword;

    std::cout << "Enter the name of the password you want to change: ";
    std::cin >> nameToUpdate;
    std::cout << "Enter the new password: ";
    std::cin >> newPassword;

    try {
        mysqlx::Session session("localhost", 33060, "root", "B.isnotmyname30");
        mysqlx::Schema db = session.getSchema("MyPasswords");
        mysqlx::Table passwords = db.getTable("Passwords");

        mysqlx::Result res = passwords.update()
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



