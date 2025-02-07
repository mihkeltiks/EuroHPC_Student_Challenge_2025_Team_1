#ifndef COMMAND_LINE_PARAMETERS_H
#define COMMAND_LINE_PARAMETERS_H


#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <cctype>
#include <sstream>
#include <iomanip>


namespace CommandlineParameters {
    
    /**
     * @brief Convert string to arbitrary type (add function overload to add a type)
     * @param s input string, used for initialization of the object
     * @return object of the correct type, initialized by the supplied string
     * 
     * The default implementation tries to convert using cast operator.
     */
    template<class T> T stringToOther(const std::string& s)         { return (T)s; }
    template<> int stringToOther(const std::string& s)              { return stoi(s); }
    template<> double stringToOther(const std::string& s)           { return stod(s); }
    template<> float stringToOther(const std::string& s)            { return stof(s); }
    template<> long stringToOther(const std::string& s)             { return stol(s); }
    template<> long long stringToOther(const std::string& s)        { return stoll(s); }
    template<> bool stringToOther(const std::string& s)             { 
        std::string tmp=s; 
        std::transform(s.begin(), s.end(), tmp.begin(), tolower); 
        return (tmp == "1") || (tmp == "true") || (tmp == "t") || (tmp == "yes") || (tmp == "y") || (tmp == "ok");
    }
    
    enum ParameterType {
        type_int,
        type_double,
        type_bool,
        type_string,
        type_invalid
    };

    struct ParameterValue {
        std::string rawValue;
        char delimiter = '\0';
        std::string noError() { return ""; }
        
        virtual std::string setString(const std::string& s) {
            rawValue = s;
            return noError();
        }
        
        virtual std::string setString(char delimiter, const std::string& s) {
            // TODO: do something with the delimiter
            this->delimiter = delimiter;
            return setString(s);
        }
        
        template<class T> operator T() const            { return T(rawValue); }
        operator int() const                            { return stoi(rawValue); }
        operator double() const                         { return stod(rawValue); }
    };
    
    template<class T>
    struct TypedParameterValue : public ParameterValue {
        T* variablePointer = nullptr;
        T defaultValue;
        bool defaultValueSet = false;
        std::function<std::string(void)> onChangeStringFunction;
        std::function<std::string(const T&)> onChangeFunction;
        
        void attachVariable(T& variable) {
            variablePointer = &variable;
        }
        
        void addOnChangeHandler(std::function<std::string(const T&)> handler) {
            onChangeFunction = handler;
        }
        
        /**
         * @brief Set the default value for this variable
         * @param val the default value
         */
        void setDefaultValue(const T& val) {
            defaultValue = val;
            defaultValueSet = true;
        }
        
     protected:
        /**
         * @brief Sets the value according to the provided string, returns error string.
         * @param s
         * @return string with the error description or empty string (for no error)
         */
        std::string setString(const std::string& s) override {
            ParameterValue::setString(s);
            if (variablePointer)
                *variablePointer = stringToOther<T>(s);
                
            // try calling the handlers by their priority (type-correct first, then the string version, finally give up and return no-error string)
            if (onChangeFunction)
                return onChangeFunction(*variablePointer);
            if (onChangeStringFunction)
                return onChangeStringFunction();
            return this->noError();
        }
    };

    /**
     * @class ParameterDefinition
     * @author Matjaž
     * @date 15/12/16
     * @file CommandLineParameters.h
     * @brief A struct containing all the required fields to parse a single argument, either a flag (just '-name') or an argument with one or more values ('-name value1 value2 ...')
     * 
     * Flags are always boolean arguments, therefore if numberOfValues is set to 0 (the default) then the argument will be expected to be bool
     */
    struct ParameterDefinition {
        std::string name;
        std::string description;
        ParameterValue defaultValue;
        bool optional = true;
        bool repeatable = false;
        bool isHelpVisible = true;
        int numExpectedValues = 0;
        ParameterType parameterType = type_invalid;
        std::shared_ptr<ParameterValue> value;
        
        ParameterDefinition() {}
        ParameterDefinition(std::string name, std::string description, bool optional = true, bool repeatable = false) : 
                name(name), description(description), optional(optional), repeatable(repeatable) {
        }
        
        /**
         * @brief Provide a default value for this parameter. The default 'default value' is empty string / invalid number.
         * @param value
         * return self - so it can be further customized
         */
        ParameterDefinition& setDefaultValue(const std::string& value) {
            defaultValue.setString(value);
            return *this;
        }
        
        /**
         * @brief Set the number of values to expect after the 'flag' part, the default is zero, making the parameter a flag
         * @param value
         * return self - so it can be further customized
         */
        ParameterDefinition& setNumberOfValues(int num) {
            numExpectedValues = num;
            return *this;
        }
        
        /**
         * @brief Make this particular definition invisible on the help screen
         * @return 
         */
        ParameterDefinition& makeHelpInvisible() {
            isHelpVisible = false;
            return *this;
        }
        
        /**
         * @brief bind a variable to the parameter; variable will be setup automatically every time the argument isparsed
         * @param var   variable to bind
         * @return      VariableHolder object, which holds the variable (can be used to apply additional settings)
         */
        template<class T> TypedParameterValue<T>& bindToVariable(T& var) {;
            auto varHolder = std::shared_ptr<TypedParameterValue<T>>(new TypedParameterValue<T>);
            varHolder->attachVariable(var);
            value = varHolder;
            return *varHolder;
        }
    };
    
    /**
     * @class ParseResult
     * @author Matjaž
     * @date 12/05/17
     * @file CommandLineParameters.h
     * @brief (TODO) when parsing argument, several errors may be present; this struct could be used as the return value from parse, to differentiate between errors.
     */
    struct ParseResult {
        bool unknownParametersPresent = false;
        bool errors = false;
    };

    /**
     * @class ParameterSet
     * @author Matjaž
     * @date 15/12/16
     * @file CommandLineParameters.h
     * @brief This is the main class for parsing arguments from the command line
     *
     * Objects of this class are simple finite state machines, full parse is achieved after calling @link #parseSingleArgument multiple times for consecutive words in the command line 
     * 
     */
    class ParameterSet {
        std::map<std::string, ParameterDefinition> definitions;
        std::ostringstream log;
        // the state:
        std::string fsmActiveArgDefinitionString;   // this argument is being parsed
        char fsmActiveArgDelimiter;                 // the last delimiter that was parsed
        int fsmActiveArgExpectedValues=0;           // this many more words are expected
        bool fsmActiveArgExpectingFirstValue=false; // marks that a new argument started parsing and that it requires a value
        std::string argumentNameDelimiters=",.:;="; // these characters act as delimiters (in addition to space) when parsing arguments 
        
    public:
        ParameterSet() {
            
        }
        
        /**
         * @brief Add an argument definition : name, description, etc
         * @param name          name of the argument - also the way (the exact string) it is provided on the commandline
         * @param description   description of this argument
         * @param optional      is the aargument optional or obligatory [TODO]
         * @param repeatable    can the argument be repeated without an error [TODO]
         * @return              the parameter definition object, which can be used to apply additional settings
         */
        ParameterDefinition& addDefinition(std::string name, std::string description, bool optional = true, bool repeatable = false) {
            return definitions[name] = ParameterDefinition(name, description, optional, repeatable);
        }
        
        /**
         * @brief Parse the input arguments as received in main
         * @param vars      the C array of 
         * @param numVars   the length of #vars array
         */
        ParseResult parse(char const * const args[], int numArgs) {
            ParseResult result;
            for (int i=0; i<numArgs; ++i) {
                const auto &arg = args[i];
                if (!parseSingleArgument(arg)) {
                    result.errors = true;
                }
            }
            return result;
        }
        
        /**
         * @brief Parse a single string, return true if it matches to one of the stored definitions, false otherwise.
         * @param arg the argument string to parse
         * @return true on success, false if the argument is unknown
         */
        bool parseSingleArgument(const std::string& arg) {
            if (fsmActiveArgExpectedValues == 0) {
                // start parsing a new argument
                auto nameEndIndex = arg.find_first_of(argumentNameDelimiters);
                std::string token = arg.substr(0, nameEndIndex);
                const auto& def = definitions.find(token);
                if (def != definitions.end()) {
                    // whole word is a known argument
                    log << "Parsing a known argument " << arg;
                    fsmActiveArgExpectedValues = def->second.numExpectedValues;
                    fsmActiveArgDefinitionString = token;
                    
                    if (fsmActiveArgExpectedValues == 0) {
                        if (def->second.value)
                            def->second.value->setString("true");
                            
                        log << "; no values expected; done\n";
                    } else {
                        fsmActiveArgExpectingFirstValue = true;
                        if (nameEndIndex < arg.size()) {
                            // argument value is provided within the argument string
                            parseArgValue(arg[nameEndIndex], arg.substr(nameEndIndex+1));
                        }
                    }
                    
                    return true;
                } else {
                    log << "an unknown argument " << arg << "; done\n";
                }
                return false;
            } else {
                // continue parsing the started argument
                return parseArgValue(' ', arg);
            }
        }
        
        std::string generateHelpScreen() const {
            std::ostringstream out;
            size_t nameWidth = 0;
            for (const auto def : definitions) {
                if (def.second.isHelpVisible)
                    nameWidth = std::max(nameWidth, def.first.size());
            }
            
            for (const auto def : definitions) {
                if (def.second.isHelpVisible)
                    out << std::setw(nameWidth + 2) << std::left << def.first << def.second.description << "\n";
            }
            return out.str();
        }
        
        /**
         * @brief Get the parse log (mostly you'd want this for debugging)
         * @return the string containing the whole log
         */
        std::string getLog() const {
            return log.str();
        }
        
    protected:
        /**
         * @brief Parse a single parameter value
         * @param delimiter 
         * @param arg       
         * @return 
         */
        bool parseArgValue(char delimiter, const std::string& arg) {            
            // set the parameter value
            const auto& def = definitions.find(fsmActiveArgDefinitionString);
            def->second.value->setString(arg);
            // TODO: delimters are so far unused, only stored, never accessed...
            fsmActiveArgDelimiter = delimiter;
            
            // log the parameter value
            if (fsmActiveArgExpectingFirstValue) {
                log << " (" << arg;
                fsmActiveArgExpectingFirstValue = false;
            } else
                log << " " << arg;
            fsmActiveArgExpectedValues --;
            if (fsmActiveArgExpectedValues == 0)
                log << ")\n";
                
            return true;
        }
    };
        
}

#endif //COMMAND_LINE_PARAMETERS_H