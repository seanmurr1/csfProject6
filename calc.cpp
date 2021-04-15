/*
 * CSF A5
 * Shelby Coe : scoe4
 * Sean Murray : smurra42
 */

// calc.cpp

#include "calc.h"
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <cctype>

using std::string;
using std::map;
using std::stringstream;
using std::vector;

// Struct to represent calculator object
struct Calc {
private:
    // Map to store persisting variables
    map<string, int> dictionary;

public:

    // Empty constructor and destructor
    Calc() { }
    ~Calc() { }

    // Evaluates a string expression and tries to store result 
    // Returns 1 on success, 0 on failure
    int evalExpr(const string &expr, int &result) {
        // Breaking expression into tokens
        vector<string> tokens = tokenize(expr);
        // Checking for valid number of tokens in expression
        if (tokens.size() < 1 || tokens.size() > 5) {
            return 0;
        }        
        // Case: only one token
        if (tokens.size() == 1) {
            if (evalToken(tokens[0], result)) {
                return 1;
            }
            // Case: bad token 
            return 0;
        }
        // Case: three tokens
        if (tokens.size() == 3) {
            return evalExpr3(tokens, result);
        }
        // Case: 5 tokens
        if (tokens.size() == 5) {
            return evalExpr5(tokens, result);
        }
        // Case: bad number of tokens
        return 0;
    }

private:
    // Attempts to evaluate expression with 3 tokens
    int evalExpr3(vector<string> tokens, int &result) {
	// Case: single assignment (var = operand)
        if (tokens[1].compare("=") == 0) {
            return singleAssignment(tokens, result);
        }
	// Case: expression (operand op operand)
	
        // Obtaining operands
        int val1;
        int val2;
        if (!evalToken(tokens[0], val1) || !evalToken(tokens[2], val2)) {
            return 0;
        }
        // Checking operator
        char op = checkOp(tokens[1]);
        if (op == 0) {
            return 0;
        }
         // Processing operator
        if (!processOp(val1, val2, op, result)) {
            return 0;
        }
        return 1;
    }

    // Special case of 3 tokens (var = operand)
    int singleAssignment(vector<string> tokens, int &result) {
        // Checking for good variable name
        if (!checkVar(tokens[0])) {
            return 0;
        }
        // Obtaining operand
        if (!evalToken(tokens[2], result)) {
            return 0;
        }
	// Storing result
        dictionary[tokens[0]] = result;
        return 1;
    }

    // Attempts to evaluate expression with 5 tokens (var = operand op operand)
    int evalExpr5(vector<string> tokens, int &result) {
        // Checking for proper "=" placement
        if (tokens[1].compare("=") != 0) {
            return 0;
        }
        // Checking for valid variable name at beginning
        if (!checkVar(tokens[0])) {
            return 0;
        }

        // Attempting to obtain operands
        int val1;
        int val2;
        if (!evalToken(tokens[2], val1) || !evalToken(tokens[4], val2)) {
            return 0;
        }
        // Checking operator
        char op = checkOp(tokens[3]);
        if (op == 0) {
            return 0;
        }
        // Processing operator
        if (!processOp(val1, val2, op, result)) {
            return 0;
        }
        // Storing value
        dictionary[tokens[0]] = result;
        return 1;
    }

    // Processes operator and attempts to store in result
    // Returns true on success, false on failure
    bool processOp(int val1, int val2, char op, int &result) {
        switch(op) {
            case '+': result = val1 + val2;
                      return true;
            case '-': result = val1 - val2;
                      return true;
            case '*': result = val1 * val2;
                      return true;
            case '/':
                      if (val2 == 0) {		// Cannot do division by zero
                            return false;
                      }
                      result = val1 / val2;
                      return true;
            default: return false;
	    }
    }


    // Checks if a string token is a valid operator
    // Returns 0 if tok is not a valid op, the op otherwise
    char checkOp(string &tok) {
	// Operator is only one char
        if (tok.length() != 1) {
            return 0;
        }
	// Getting and checking char
        char op = tok[0];
        if (op == '+' || op == '-' || op == '*' || op == '/') {
            return op;
        }
	// Case: bad op
        return 0;
    }


    // Evaluates a single token and stores result 
    bool evalToken(string &tok, int &result) {
	// Case: token is an int
        if (checkInt(tok)) {
            result = std::stoi(tok);
            return true;
        }
	// Case: token is a valid var that we have in our dictionary
        if (checkVar(tok) && (dictionary.find(tok) != dictionary.end())) {
            result = dictionary.at(tok);
            return true;
        }
	// Case: bad token
        return false;
    }

    // Checks if a given string is a valid integer value
    bool checkInt(string &tok) {
        for (int i = 0; i < (int) tok.length(); i++) {
		// Checking if int is negative
            	if (i == 0 && tok[i] == '-' && (int) tok.length() > 1) {
			continue;
		}
		if (!isdigit(tok[i])) {
                	return false;
            	}
        }
        return true;
    }

    // Checks if a given string is a valid variable name
    bool checkVar(string &tok) {
        for (int i = 0; i < (int) tok.length(); i++) {
            if (!isalpha(tok[i])) {
                return false;
            }
        }
        return true;
    }


    // Breaks input string into separate tokens stored in a vector of strings
    vector<string> tokenize(const string &expr) {
        // Vector
        vector<string> vec;
        // String stream
        stringstream s(expr);
        // Current token
        string tok;
        // Taking in all tokens
        while (s >> tok) {
            vec.push_back(tok);
        }
        return vec;
    }

};

// Create calc struct
extern "C" struct Calc *calc_create(void) {
    return new Calc();
}

// Destroy calc struct
extern "C" void calc_destroy(struct Calc *calc) {
    delete calc;
}

// Evaluate expression with calc objec
// Attempts to store result
extern "C" int calc_eval(struct Calc *calc, const char *expr, int *result) {
    return calc->evalExpr(expr, *result);
}
