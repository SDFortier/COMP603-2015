/*
= Brainfuck

If you have gcc:

----
g++ -o brainfuck.exe brainfuck.cpp
brainfuck.exe helloworld.bf
----
*/

#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

/**
 * Primitive Brainfuck commands
 */
typedef enum {
    INCREMENT, // +
    DECREMENT, // -
    SHIFT_LEFT, // <
    SHIFT_RIGHT, // >
    INPUT, // ,
    OUTPUT, // .
    ZERO // [-] or [+]
} Command;

// Forward references. Silly C++!
class CommandNode;
class Loop;
class Program;

/**
 * Visits?!? Well, that'd indicate visitors!
 * A visitor is an interface that allows you to walk through a tree and do stuff.
 */
class Visitor {
    public:
        virtual void visit(const CommandNode * leaf) = 0;
        virtual void visit(const Loop * loop) = 0;
        virtual void visit(const Program * program) = 0;
};

/**
 * The Node class (like a Java abstract class) accepts visitors, but since it's pure virtual, we can't use it directly.
 */
class Node {
    public:
        virtual void accept (Visitor *v) = 0;
};

/**
 * CommandNode publicly extends Node to accept visitors.
 * CommandNode represents a leaf node with a primitive Brainfuck command in it.
 */
class CommandNode : public Node {
    public:
        Command command;
        int count;
        CommandNode(char c, int n) {
            switch(c) {
                case '+': command = INCREMENT; break;
                case '-': command = DECREMENT; break;
                case '<': command = SHIFT_LEFT; break;
                case '>': command = SHIFT_RIGHT; break;
                case ',': command = INPUT; break;
                case '.': command = OUTPUT; break;
                case 'z': command = ZERO; break;
            }
            count = n;
        }
        void accept (Visitor * v) {
            v->visit(this);
        }
};

class Container: public Node {
    public:
        vector<Node*> children;
        virtual void accept (Visitor * v) = 0;
};

/**
 * Loop publicly extends Node to accept visitors.
 * Loop represents a loop in Brainfuck.
 */
class Loop : public Container {
    public:
        void accept (Visitor * v) {
            v->visit(this);
        }
};

/**
 * Program is the root of a Brainfuck program abstract syntax tree.
 * Because Brainfuck is so primitive, the parse tree is the abstract syntax tree.
 */
class Program : public Container {
    public:
        void accept (Visitor * v) {
            v->visit(this);
        }
};

/**
 * Read in the file by recursive descent.
 * Modify as necessary and add whatever functions you need to get things done.
 */
void parse(fstream & file, Container * container) {
    char c;
    // How to peek at the next character
    // c = (char)file.peek();
    // How to print out that character
    // cout << c;
    // How to read a character from the file and advance to the next character
    // file >> c;
    // How to print out that character
    // cout << c;
    // How to insert a node into the container.
    // container->children.push_back(new CommandNode(c));
    while(file >> c) {
        int count = 1;
        if(c == '<' || c == '>' || c == '.' || c == ',' || c == '_' || c == '-' || c == '+') {
            while((char)file.peek() == c) {
                file >> c;
                count++;
            }
            container->children.push_back(new CommandNode(c, count));
        }
        else if(c == '[') {
            Loop *loop = new Loop();
            parse(file, loop);
            if(loop->children.size() == 1) {
                CommandNode* child = dynamic_cast<CommandNode*>(loop->children.front());
                if(child->command == INCREMENT || child->command == DECREMENT) {
                    container->children.push_back(new CommandNode('z', 1));
                    delete loop;
                }
                else {
                    container->children.push_back(loop);
                }
            }
            else {
                container->children.push_back(loop);
            }
        }
        else if(c == ']') {
            return;
        }
    }
}

/**
 * A printer for Brainfuck abstract syntax trees.
 * As a visitor, it will just print out the commands as is.
 * For Loops and the root Program node, it walks trough all the children.
 */
class Printer : public Visitor {
    public:
        void visit(const CommandNode * leaf) {
            if(leaf->count > 1) {
                cout << leaf->count;
            }
            switch (leaf->command) {
                case INCREMENT:   cout << '+'; break;
                case DECREMENT:   cout << '-'; break;
                case SHIFT_LEFT:  cout << '<'; break;
                case SHIFT_RIGHT: cout << '>'; break;
                case INPUT:       cout << ','; break;
                case OUTPUT:      cout << '.'; break;
                case ZERO:        cout << 'z'; break;
            }
        }
        void visit(const Loop * loop) {
            cout << '[';
            for (vector<Node*>::const_iterator it = loop->children.begin(); it != loop->children.end(); ++it) {
                (*it)->accept(this);
            }
            cout << ']';
        }
        void visit(const Program * program) {
            for (vector<Node*>::const_iterator it = program->children.begin(); it != program->children.end(); ++it) {
                (*it)->accept(this);
            }
            cout << '\n';
        }
};

class Interpreter : public Visitor {
    char memory[30000];
    int pointer;
    public:
        void visit(const CommandNode * leaf) {
            for(int i = 0; i < leaf->count; i++) {
                switch (leaf->command) {
                    case INCREMENT:
                        memory[pointer]++;
                        break;
                    case DECREMENT:
                        memory[pointer]--;
                        break;
                    case SHIFT_LEFT:
                        pointer--;
                        break;
                    case SHIFT_RIGHT:
                        pointer++;
                        break;
                    case INPUT:
                        cin.get(memory[pointer]);
                        break;
                    case OUTPUT:
                        cout << memory[pointer];
                        break;
                    case ZERO:
                        memory[pointer] = 0;
                        break;
                }
            }
        }
        void visit(const Loop * loop) {
            while(memory[pointer]) {
                for (vector<Node*>::const_iterator it = loop->children.begin(); it != loop->children.end(); ++it) {
                    (*it)->accept(this);
                }
            }
        }
        void visit(const Program * program) {
            // zero init the memory array
            // set pointer to zero
            for(int i = 0; i < sizeof(memory); i++) {
                memory[i] = 0;
            }
            pointer = 0;
            for (vector<Node*>::const_iterator it = program->children.begin(); it != program->children.end(); ++it) {
                (*it)->accept(this);
            }
        }
};

int main(int argc, char *argv[]) {
    fstream file;
    Program program;
    Printer printer;
    Interpreter interpreter;
    if (argc == 1) {
        cout << argv[0] << ": No input files." << endl;
    } else if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            file.open(argv[i], fstream::in);
            parse(file, & program);
            //program.accept(&printer);
            program.accept(&interpreter);
            file.close();
        }
    }
}
