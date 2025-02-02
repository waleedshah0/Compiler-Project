#include <iostream>
#include <fstream>
#include <string>
#include <cctype>

using namespace std;

const int MAX = 500;

const string operators[] = {
    "!=", "<>", "=:=", "==", "*", "+", "/", "-", ">>", "<<", "++", "=+",
    "&&", "||", "=>", "=<", "%", ":", "::", "--"
};

char* keywords[] = {
    "loop", "agar", "magar", "asm", "else", "new", "this", "auto", "enum",
    "operator", "throw", "bool", "explicit", "private", "true", "break",
    "export", "protected", "try", "case", "extern", "public", "typedef",
    "catch", "false", "register", "typeid", "char", "float", "typename",
    "class", "for", "return", "union", "const", "friend", "short", "unsigned",
    "goto", "signed", "using", "continue", "if", "sizeof", "virtual", "default",
    "inline", "static", "void", "delete", "int", "volatile", "do", "long",
    "struct", "double", "mutable", "switch", "while", "namespace"
};

const char punctuations[] = { '[', '{', '<', '>', '}', ']', '(', ')', ';', ',' };

class Node {
    string identifier, scope, type;
    int lineNo;
    Node* next;

public:
    Node() : next(nullptr) {}
    Node(string key, string value, string type, int lineNo)
        : identifier(key), scope(value), type(type), lineNo(lineNo), next(nullptr) {}

    void print() {
        cout << "Identifier: " << identifier
             << "\nType: " << type
             << "\nScope: " << scope
             << "\nLine: " << lineNo << endl;
    }

    friend class SymbolTable;
};

class SymbolTable {
    Node* head[MAX];

public:
    SymbolTable() {
        for (int i = 0; i < MAX; i++) head[i] = nullptr;
    }

    int hashf(string id) {
        int asciiSum = 0;
        for (char c : id) asciiSum += c;
        return asciiSum % MAX;
    }

    bool insert(string id, string scope, string Type, int lineno) {
        int index = hashf(id);
        Node* node = new Node(id, scope, Type, lineno);

        if (!head[index]) {
            head[index] = node;
            return true;
        }

        Node* temp = head[index];
        while (temp->next) temp = temp->next;
        temp->next = node;
        return true;
    }

    int find(string id) {
        int index = hashf(id);
        Node* start = head[index];

        while (start) {
            if (start->identifier == id) return index;
            start = start->next;
        }

        return -1;
    }
};

// Utility functions
int len(char arr[]) {
    int i = 0;
    while (arr[i] != '\0') ++i;
    return i;
}

void strcopy(char temp[], char arr[], int start, int end) {
    int index = 0;
    for (int i = start; i < end; ++i) temp[index++] = arr[i];
    temp[index] = '\0';
}

bool checkInKeywords(char temp[]) {
    for (int i = 0; keywords[i]; ++i) {
        if (string(temp) == string(keywords[i])) return true;
    }
    return false;
}

int validateIdentifier(char arr[], int start) {
    int i = start;
    if (arr[i] == '_' || isalpha(arr[i])) {
        i++;
        while (arr[i] == '_' || isalnum(arr[i])) i++;
        return i;
    }
    return start;
}

int validateNumber(char arr[], int start) {
    int i = start;
    bool hasDecimal = false, hasExponent = false;

    if (arr[i] == '+' || arr[i] == '-') i++; // Optional sign
    while (isdigit(arr[i]) || arr[i] == '.') {
        if (arr[i] == '.') {
            if (hasDecimal) return start; // More than one decimal point
            hasDecimal = true;
        }
        i++;
    }
    if ((arr[i] == 'E' || arr[i] == 'e') && (isdigit(arr[i + 1]) || arr[i + 1] == '+' || arr[i + 1] == '-')) {
        hasExponent = true;
        i += 2; // Skip 'E'/'e' and exponent sign
        while (isdigit(arr[i])) i++;
    }
    return i > start ? i : start;
}

int validateOperator(char arr[], int start) {
    for (const string& op : operators) {
        bool match = true;
        int i = 0;
        while (op[i] != '\0' && arr[start + i] != '\0') {
            if (op[i] != arr[start + i]) {
                match = false;
                break;
            }
            i++;
        }
        if (match) return start + i;
    }
    return start;
}

int validatePunctuation(char arr[], int start) {
    for (char p : punctuations) {
        if (arr[start] == p) return start + 1;
    }
    return start;
}

void lexicalAnalysis(char arr[], SymbolTable& st, int lineNo) {
    ofstream tokenFile("Token.txt", ios::app);
    ofstream errorFile("Error.txt", ios::app);

    int size = len(arr);
    for (int i = 0; i < size;) {
        if (isspace(arr[i])) {
            i++;
            continue;
        }

        char temp[100];
        int result;

        // Identifier and Keyword Validation
        result = validateIdentifier(arr, i);
        if (result > i) {
            strcopy(temp, arr, i, result);
            if (checkInKeywords(temp)) {
                cout << "Keyword: " << temp << endl;
                tokenFile << "<KEYWORD, " << temp << ">\n";
            } else {
                cout << "Identifier: " << temp << endl;
                st.insert(temp, "local", "identifier", lineNo);
                tokenFile << "<IDENTIFIER, " << temp << ">\n";
            }
            i = result;
            continue;
        }

        // Number Validation
        result = validateNumber(arr, i);
        if (result > i) {
            strcopy(temp, arr, i, result);
            cout << "Number: " << temp << endl;
            tokenFile << "<NUMBER, " << temp << ">\n";
            i = result;
            continue;
        }

        // Operator Validation
        result = validateOperator(arr, i);
        if (result > i) {
            strcopy(temp, arr, i, result);
            cout << "Operator: " << temp << endl;
            tokenFile << "<OPERATOR, " << temp << ">\n";
            i = result;
            continue;
        }

        // Punctuation Validation
        result = validatePunctuation(arr, i);
        if (result > i) {
            cout << "Punctuation: " << arr[i] << endl;
            tokenFile << "<PUNCTUATION, " << arr[i] << ">\n";
            i = result;
            continue;
        }

        // Invalid Character
        cout << "Error: " << arr[i] << endl;
        errorFile << "Error: " << arr[i] << " at line " << lineNo << endl;
        i++;
    }

    tokenFile.close();
    errorFile.close();
}

// Main function
int main() {
    ifstream inputFile("input_file.txt");
    SymbolTable st;
    char buffer[4096];
    int lineNo = 1;

    while (inputFile.getline(buffer, sizeof(buffer))) {
        lexicalAnalysis(buffer, st, lineNo);
        lineNo++;
    }

    inputFile.close();
    return 0;
}
