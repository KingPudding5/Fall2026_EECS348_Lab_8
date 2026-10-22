// Author - Francis Tryban
// Lab_8 String Based Calculator
// Date Oct 21


#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>

using namespace std;

// struct to hold the pieces of a number
struct NumberParts {
    char sign = '+';
    string integerPart = "0";
    string decimalPart = "0";
};

// checks if a string is a valid double:
bool isValidDouble(const string& s) {
    if (s.empty()) return false;

    size_t start = 0;
    // Check for leading + or -
    if (s[0] == '+' || s[0] == '-') {
        start = 1;
        if (s.length() == 1) return false; // check if string is only + or -
    }

    bool decimalFound = false;
    int digitsBeforeDecimal = 0;
    int digitsAfterDecimal = 0;

    for (size_t i = start; i < s.length(); ++i) { //iterate over the characters
        char c = s[i];
        if (c == '.') {
            if (decimalFound) return false; // 2 decimals is 2 many 
            decimalFound = true;
        } else if (isdigit(c)) {
            if (decimalFound) {
                digitsAfterDecimal++;
            } else {
                digitsBeforeDecimal++;
            }
        } else {
            return false; // if char not digit not good
        }
    }

    // If there is a decimal point, there must be at least one digit on BOTH sides (.01 and -5. are no good)
    if (decimalFound) {
        if (digitsBeforeDecimal == 0 || digitsAfterDecimal == 0) return false;
    } else {
        // no decimal so just need at least one digit total
        if (digitsBeforeDecimal == 0) return false;
    }

    return true;
}

// breaks a VALID string into | sign | int part | decimal part |
NumberParts parseNumber(const string& s) {
    NumberParts p;
    size_t start = 0;

    // take the sign if there is one, otherwise its + by default
    if (s[0] == '+' || s[0] == '-') {
        p.sign = s[0];
        start = 1;
    }

    // find the decimal point
    size_t dotPos = s.find('.', start);

    if (dotPos == string::npos) { // if no decimal 
        p.integerPart = s.substr(start);
        p.decimalPart = "0";
    } else {
        p.integerPart = s.substr(start, dotPos - start); // has a decimal. split on decimal
        p.decimalPart = s.substr(dotPos + 1);
    }

    // strip leading zeros off int part
    size_t firstNonZero = p.integerPart.find_first_not_of('0');
    if (firstNonZero != string::npos) {
        p.integerPart = p.integerPart.substr(firstNonZero);
    } else {
        p.integerPart = "0"; // it was all zeros so leave one
    }

    return p;
}

// lines both numbers up digit for digit so the loops can go column by column
// adding trailing or leading zereos 
void padParts(NumberParts& a, NumberParts& b) {
    while (a.decimalPart.length() < b.decimalPart.length()) a.decimalPart += "0"; //decimals get trailing zeros
    while (b.decimalPart.length() < a.decimalPart.length()) b.decimalPart += "0";

    while (a.integerPart.length() < b.integerPart.length()) a.integerPart = "0" + a.integerPart; //ints get leading zeros
    while (b.integerPart.length() < a.integerPart.length()) b.integerPart = "0" + b.integerPart;
}

// adds 2 VALID digit strings right to left w/ carry
// no decimal in here, it will be added back later
string addDigits(const string& a, const string& b) {
    string result = "";
    int carry = 0;

    for (int i = (int)a.length() - 1; i >= 0; --i) {
        int sum = (a[i] - '0') + (b[i] - '0') + carry; // convert individual charecter into int
        carry = sum / 10;                   
        result = to_string(sum % 10) + result; // left append the mod 10 value to the temp sum
    }

    if (carry > 0) result = "1" + result; // leftover carry goes on the very front
    return result;
}

// subtracts small from big right to left w/ borrow
string subtractDigits(const string& big, const string& small) {
    string result = "";
    int borrow = 0;

    for (int i = (int)big.length() - 1; i >= 0; --i) {
        int diff = (big[i] - '0') - (small[i] - '0') - borrow;
        if (diff < 0) {
            diff += 10; // borrow 10 from the next column over
            borrow = 1;
        } else {
            borrow = 0;
        }
        result = to_string(diff) + result;
    }

    return result; // never a leftover borrow since big >= small
}

// put the dot back in + clean up extra zeros
string formatResult(const string& digits, size_t decLen, char sign) { // decLen = how many digits were after the dot
    string intPart = digits.substr(0, digits.length() - decLen);
    string decPart = digits.substr(digits.length() - decLen);

    // strip leading zeros on the int part but keep at least one 0
    size_t firstNonZero = intPart.find_first_not_of('0');
    intPart = (firstNonZero == string::npos) ? "0" : intPart.substr(firstNonZero);

    // strip trailing zeros on the decimal part
    size_t lastNonZero = decPart.find_last_not_of('0');
    decPart = (lastNonZero == string::npos) ? "" : decPart.substr(0, lastNonZero + 1);

    if (intPart == "0" && decPart.empty()) return "0"; // if the answer is zero just say 0, dont print -0

    string result = intPart;
    if (!decPart.empty()) result += "." + decPart;
    if (sign == '-') result = "-" + result;
    return result;
}

// take the 2 original strings and give back the sum as a string
string addNumbers(const string& num1, const string& num2) {
    NumberParts a = parseNumber(num1);
    NumberParts b = parseNumber(num2);
    padParts(a, b);

    // glue int + decimal together w/o the dot so its just one long digit string
    string digitsA = a.integerPart + a.decimalPart;
    string digitsB = b.integerPart + b.decimalPart;
    size_t decLen = a.decimalPart.length(); // same for both after padding

    // same sign -> just add and keep the sign (-2 + -3 = -(2 + 3))
    if (a.sign == b.sign) {
        return formatResult(addDigits(digitsA, digitsB), decLen, a.sign);
    }

    // diff signs -> subtraction: big - small, answer gets the bigger ones sign
    if (digitsA >= digitsB) {
        return formatResult(subtractDigits(digitsA, digitsB), decLen, a.sign);
    } else {
        return formatResult(subtractDigits(digitsB, digitsA), decLen, b.sign);
    }
}

// reads the file line by line, checks each line, and prints the sum
void processFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error opening file: " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue; // skip totally blank lines

        stringstream ss(line);
        string num1, num2, extra;

        // need EXACTLY 2 values: if i cant grab 2, OR theres a 3rd one left over, its bad
        if (!(ss >> num1 >> num2) || (ss >> extra)) {
            cout << "Error: Line contains fewer or more than two values: " << line << endl;
            continue;
        }

        if (isValidDouble(num1) && isValidDouble(num2)) {
            cout << num1 << " + " << num2 << " = " << addNumbers(num1, num2) << endl;
        } else {
            // say which one was bad
            if (!isValidDouble(num1)) cout << "Invalid number: " << num1 << endl;
            if (!isValidDouble(num2)) cout << "Invalid number: " << num2 << endl;
        }
    }

    file.close();
}

int main() {
    // spec says the file is user specified so ask for it
    string filename;
    cout << "Enter the test file name: ";
    cin >> filename;

    processFile(filename);
    return 0;
}