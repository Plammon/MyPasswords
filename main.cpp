#include <iostream>
#include <string>
#include <mysqlx/xdevapi.h>
#include "main.h"
using namespace std;
using namespace mysqlx;

// Fonksiyon prototipleri
void Start();
void Signup();
bool isPasswordStrong(const std::string& password);

int main() {
   try{

        Start(); // Programı başlat
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

void Start() {
    int f;
    cout << "Welcome to my website." << endl;
    cout << "Please choose one:" << endl;
    cout << "1 - Sign up" << endl;
    cout << "2 - Sign in" << endl;
    cout << "3 - Exit\t:" << endl;
    cin >> f;
    if (f == 1) {
        Signup();
    } else if (f == 2) {
        // Signin(); // Henüz yazılmadı
        cout << "Signin is not implemented yet." << endl;
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

/*
void Signin(){
    int  decision;
    cout<<"What would you like to do"<<endl;
    cout<<"1-Edit Passwords"<<endl;
    cout<<"2-Create a new password and see how strong it is"<<endl;
    cout<<"3-Info"<<endl;
    cout<<"4-Exit"<<endl;
    cin >> decision;

    if(decision==1){
      EditPasswords();
    }
    if(decision==2){
        PasswordTester();
    }
    if(decision==3){
       Info();
    }
    else if(decision==4){
        return;
    }
}

void EditPasswords(){
    int i;
    cout<<"What exactly do you wanna do ?"<<endl;
    cout<<"1-Add a new password"<<endl;
    cout<<"2-Delete a password"<<endl;
    cout<<"3-List all the passwords"<<endl;
    cin >> i;
    if(i==1){
        AddPassword();
    }
    if(i==2){
        DeletePassword();
    }
    else if(i==3){
        ListPasswords();
    }


}

*/