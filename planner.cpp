#include <iostream>
#include <fstream>
// #include <boost/functional/hash.hpp>
#include <regex>
#include <unordered_set>
#include <set>
#include <list>
#include <unordered_map>
#include <algorithm>
#include <stdexcept>
#include <queue>
#include <climits>

#define SYMBOLS 0
#define INITIAL 1
#define GOAL 2
#define ACTIONS 3
#define ACTION_DEFINITION 4
#define ACTION_PRECONDITION 5
#define ACTION_EFFECT 6

class GroundedCondition;
class Condition;
class GroundedAction;
class Action;
class Env;

using namespace std;

bool print_status = true;

class GroundedCondition
{
private:
    string predicate;
    list<string> arg_values;
    bool truth = true;

public:
    GroundedCondition(string predicate, list<string> arg_values, bool truth = true)
    {
        this->predicate = predicate;
        this->truth = truth;  // fixed
        for (string l : arg_values)
        {
            this->arg_values.push_back(l);
        }
    }

    GroundedCondition(const GroundedCondition& gc)
    {
        this->predicate = gc.predicate;
        this->truth = gc.truth;  // fixed
        for (string l : gc.arg_values)
        {
            this->arg_values.push_back(l);
        }
    }

    string get_predicate() const
    {
        return this->predicate;
    }
    list<string> get_arg_values() const
    {
        return this->arg_values;
    }

    bool get_truth() const
    {
        return this->truth;
    }

    void set_truth(const bool &new_truth_val)
    {
        this->truth = new_truth_val;
    }

    friend ostream& operator<<(ostream& os, const GroundedCondition& pred)
    {
        os << pred.toString() << " ";
        return os;
    }

    bool operator==(const GroundedCondition& rhs) const
    {
        if (this->predicate != rhs.predicate || this->arg_values.size() != rhs.arg_values.size())
            return false;

        auto lhs_it = this->arg_values.begin();
        auto rhs_it = rhs.arg_values.begin();

        while (lhs_it != this->arg_values.end() && rhs_it != rhs.arg_values.end())
        {
            if (*lhs_it != *rhs_it)
                return false;
            ++lhs_it;
            ++rhs_it;
        }

        if (this->truth != rhs.get_truth()) // fixed
            return false;

        return true;
    }

    string toString() const
    {
        string temp = "";
        if (!this->truth)
            temp += "!";
        temp += this->predicate;
        temp += "(";
        for (string l : this->arg_values)
        {
            temp += l + ",";
        }
        temp = temp.substr(0, temp.length() - 1);
        temp += ")";
        return temp;
    }
};

struct GroundedConditionComparator
{
    bool operator()(const GroundedCondition& lhs, const GroundedCondition& rhs) const
    {
        return lhs == rhs;
    }
};

struct GroundedConditionHasher
{
    size_t operator()(const GroundedCondition& gcond) const
    {
        return hash<string>{}(gcond.toString());
    }
};

class Condition
{
private:
    string predicate;
    list<string> args;
    bool truth;

public:
    Condition(string pred, list<string> args, bool truth)
    {
        this->predicate = pred;
        this->truth = truth;
        for (string ar : args)
        {
            this->args.push_back(ar);
        }
    }

    string get_predicate() const
    {
        return this->predicate;
    }

    list<string> get_args() const
    {
        return this->args;
    }

    bool get_truth() const
    {
        return this->truth;
    }

    friend ostream& operator<<(ostream& os, const Condition& cond)
    {
        os << cond.toString() << " ";
        return os;
    }

    bool operator==(const Condition& rhs) const // fixed
    {

        if (this->predicate != rhs.predicate || this->args.size() != rhs.args.size())
            return false;

        auto lhs_it = this->args.begin();
        auto rhs_it = rhs.args.begin();

        while (lhs_it != this->args.end() && rhs_it != rhs.args.end())
        {
            if (*lhs_it != *rhs_it)
                return false;
            ++lhs_it;
            ++rhs_it;
        }

        if (this->truth != rhs.get_truth())
            return false;

        return true;
    }

    string toString() const
    {
        string temp = "";
        if (!this->truth)
            temp += "!";
        temp += this->predicate;
        temp += "(";
        for (string l : this->args)
        {
            temp += l + ",";
        }
        temp = temp.substr(0, temp.length() - 1);
        temp += ")";
        return temp;
    }
};

struct ConditionComparator
{
    bool operator()(const Condition& lhs, const Condition& rhs) const
    {
        return lhs == rhs;
    }
};

struct ConditionHasher
{
    size_t operator()(const Condition& cond) const
    {
        return hash<string>{}(cond.toString());
    }
};

class Action
{
private:
    string name;
    list<string> args;
    unordered_set<Condition, ConditionHasher, ConditionComparator> preconditions;
    unordered_set<Condition, ConditionHasher, ConditionComparator> effects;

public:
    Action(string name, list<string> args,
        unordered_set<Condition, ConditionHasher, ConditionComparator>& preconditions,
        unordered_set<Condition, ConditionHasher, ConditionComparator>& effects)
    {
        this->name = name;
        for (string l : args)
        {
            this->args.push_back(l);
        }
        for (Condition pc : preconditions)
        {
            this->preconditions.insert(pc);
        }
        for (Condition pc : effects)
        {
            this->effects.insert(pc);
        }
    }
    string get_name() const
    {
        return this->name;
    }
    list<string> get_args() const
    {
        return this->args;
    }
    unordered_set<Condition, ConditionHasher, ConditionComparator> get_preconditions() const
    {
        return this->preconditions;
    }
    unordered_set<Condition, ConditionHasher, ConditionComparator> get_effects() const
    {
        return this->effects;
    }

    bool operator==(const Action& rhs) const
    {
        if (this->get_name() != rhs.get_name() || this->get_args().size() != rhs.get_args().size())
            return false;

        return true;
    }

    friend ostream& operator<<(ostream& os, const Action& ac)
    {
        os << ac.toString() << endl;
        os << "Precondition: ";
        for (Condition precond : ac.get_preconditions())
            os << precond;
        os << endl;
        os << "Effect: ";
        for (Condition effect : ac.get_effects())
            os << effect;
        os << endl;
        return os;
    }

    string toString() const
    {
        string temp = "";
        temp += this->get_name();
        temp += "(";
        for (string l : this->get_args())
        {
            temp += l + ",";
        }
        temp = temp.substr(0, temp.length() - 1);
        temp += ")";
        return temp;
    }
};

struct ActionComparator
{
    bool operator()(const Action& lhs, const Action& rhs) const
    {
        return lhs == rhs;
    }
};

struct ActionHasher
{
    size_t operator()(const Action& ac) const
    {
        return hash<string>{}(ac.get_name());
    }
};

class Env
{
private:
    unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> initial_conditions;
    unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> goal_conditions;
    unordered_set<Action, ActionHasher, ActionComparator> actions;
    unordered_set<string> symbols;

public:
    void remove_initial_condition(GroundedCondition gc)
    {
        this->initial_conditions.erase(gc);
    }
    void add_initial_condition(GroundedCondition gc)
    {
        this->initial_conditions.insert(gc);
    }
    void add_goal_condition(GroundedCondition gc)
    {
        this->goal_conditions.insert(gc);
    }
    void remove_goal_condition(GroundedCondition gc)
    {
        this->goal_conditions.erase(gc);
    }
    void add_symbol(string symbol)
    {
        symbols.insert(symbol);
    }
    void add_symbols(list<string> symbols)
    {
        for (string l : symbols)
            this->symbols.insert(l);
    }
    void add_action(Action action)
    {
        this->actions.insert(action);
    }

    Action get_action(string name)
    {
        for (Action a : this->actions)
        {
            if (a.get_name() == name)
                return a;
        }
        throw runtime_error("Action " + name + " not found!");
    }

    unordered_set<Action, ActionHasher, ActionComparator> get_all_actions()
    {
        return actions;
    }

    unordered_set<string> get_symbols() const
    {
        return this->symbols;
    }

    unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator>  get_initial_conditions() const
    {
        return this->initial_conditions;
    }

    unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator>  get_goal_conditions() const
    {
        return this->goal_conditions;
    }

    friend ostream& operator<<(ostream& os, const Env& w)
    {
        os << "***** Environment *****" << endl << endl;
        os << "Symbols: ";
        for (string s : w.get_symbols())
            os << s + ",";
        os << endl;
        os << "Initial conditions: ";
        for (GroundedCondition s : w.initial_conditions)
            os << s;
        os << endl;
        os << "Goal conditions: ";
        for (GroundedCondition g : w.goal_conditions)
            os << g;
        os << endl;
        os << "Actions:" << endl;
        for (Action g : w.actions)
            os << g << endl;
        cout << "***** Environment Created! *****" << endl;
        return os;
    }
};

class GroundedAction
{
private:
    string name;
    list<string> arg_values;
    unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> gPreconditions;
    unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> gEffects;

public:
    GroundedAction(string name, list<string> arg_values)
    {
        this->name = name;
        for (string ar : arg_values)
        {
            this->arg_values.push_back(ar);
        }
    }

    GroundedAction() = default;

    GroundedAction(string naam,
                    list<string> arg_values,
                   unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> new_gPreconditions,
                   unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> new_gEffects):
          name{naam},gPreconditions{new_gPreconditions},gEffects{new_gEffects}
    {
        for (string ar : arg_values)
        {
            this->arg_values.push_back(ar);
        }
    }

    string get_name() const
    {
        return this->name;
    }

    list<string> get_arg_values() const
    {
        return this->arg_values;
    }

    unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> get_preconditions() const
    {
        return this->gPreconditions;
    }

    unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> get_effects() const
    {
        return this->gEffects;
    }

    bool operator==(const GroundedAction& rhs) const
    {
        if (this->name != rhs.name || this->arg_values.size() != rhs.arg_values.size())
            return false;

        auto lhs_it = this->arg_values.begin();
        auto rhs_it = rhs.arg_values.begin();

        while (lhs_it != this->arg_values.end() && rhs_it != rhs.arg_values.end())
        {
            if (*lhs_it != *rhs_it)
                return false;
            ++lhs_it;
            ++rhs_it;
        }
        return true;
    }

    friend ostream& operator<<(ostream& os, const GroundedAction& gac)
    {
        os << gac.toString() << " ";
        return os;
    }

    string toString() const
    {
        string temp = "";
        temp += this->name;
        temp += "(";
        for (string l : this->arg_values)
        {
            temp += l + ",";
        }
        temp = temp.substr(0, temp.length() - 1);
        temp += ")";
        return temp;
    }
};

list<string> parse_symbols(string symbols_str)
{
    list<string> symbols;
    size_t pos = 0;
    string delimiter = ",";
    while ((pos = symbols_str.find(delimiter)) != string::npos)
    {
        string symbol = symbols_str.substr(0, pos);
        symbols_str.erase(0, pos + delimiter.length());
        symbols.push_back(symbol);
    }
    symbols.push_back(symbols_str);
    return symbols;
}

Env* create_env(char* filename)
{
    ifstream input_file(filename);
    Env* env = new Env();
    regex symbolStateRegex("symbols:", regex::icase);
    regex symbolRegex("([a-zA-Z0-9_, ]+) *");
    regex initialConditionRegex("initialconditions:(.*)", regex::icase);
    regex conditionRegex("(!?[A-Z][a-zA-Z_]*) *\\( *([a-zA-Z0-9_, ]+) *\\)");
    regex goalConditionRegex("goalconditions:(.*)", regex::icase);
    regex actionRegex("actions:", regex::icase);
    regex precondRegex("preconditions:(.*)", regex::icase);
    regex effectRegex("effects:(.*)", regex::icase);
    int parser = SYMBOLS;

    unordered_set<Condition, ConditionHasher, ConditionComparator> preconditions;
    unordered_set<Condition, ConditionHasher, ConditionComparator> effects;
    string action_name;
    string action_args;

    string line;
    if (input_file.is_open())
    {
        while (getline(input_file, line))
        {
            string::iterator end_pos = remove(line.begin(), line.end(), ' ');
            line.erase(end_pos, line.end());

            if (line == "")
                continue;

            if (parser == SYMBOLS)
            {
                smatch results;
                if (regex_search(line, results, symbolStateRegex))
                {
                    line = line.substr(8);
                    sregex_token_iterator iter(line.begin(), line.end(), symbolRegex, 0);
                    sregex_token_iterator end;

                    env->add_symbols(parse_symbols(iter->str()));  // fixed

                    parser = INITIAL;
                }
                else
                {
                    cout << "Symbols are not specified correctly." << endl;
                    throw;
                }
            }
            else if (parser == INITIAL)
            {
                const char* line_c = line.c_str();
                if (regex_match(line_c, initialConditionRegex))
                {
                    const std::vector<int> submatches = { 1, 2 };
                    sregex_token_iterator iter(
                        line.begin(), line.end(), conditionRegex, submatches);
                    sregex_token_iterator end;

                    while (iter != end)
                    {
                        // name
                        string predicate = iter->str();
                        iter++;
                        // args
                        string args = iter->str();
                        iter++;

                        if (predicate[0] == '!')
                        {
                            env->remove_initial_condition(
                                GroundedCondition(predicate.substr(1), parse_symbols(args)));
                        }
                        else
                        {
                            env->add_initial_condition(
                                GroundedCondition(predicate, parse_symbols(args)));
                        }
                    }

                    parser = GOAL;
                }
                else
                {
                    cout << "Initial conditions not specified correctly." << endl;
                    throw;
                }
            }
            else if (parser == GOAL)
            {
                const char* line_c = line.c_str();
                if (regex_match(line_c, goalConditionRegex))
                {
                    const std::vector<int> submatches = { 1, 2 };
                    sregex_token_iterator iter(
                        line.begin(), line.end(), conditionRegex, submatches);
                    sregex_token_iterator end;

                    while (iter != end)
                    {
                        // name
                        string predicate = iter->str();
                        iter++;
                        // args
                        string args = iter->str();
                        iter++;

                        if (predicate[0] == '!')
                        {
                            env->remove_goal_condition(
                                GroundedCondition(predicate.substr(1), parse_symbols(args)));
                        }
                        else
                        {
                            env->add_goal_condition(
                                GroundedCondition(predicate, parse_symbols(args)));
                        }
                    }

                    parser = ACTIONS;
                }
                else
                {
                    cout << "Goal conditions not specified correctly." << endl;
                    throw;
                }
            }
            else if (parser == ACTIONS)
            {
                const char* line_c = line.c_str();
                if (regex_match(line_c, actionRegex))
                {
                    parser = ACTION_DEFINITION;
                }
                else
                {
                    cout << "Actions not specified correctly." << endl;
                    throw;
                }
            }
            else if (parser == ACTION_DEFINITION)
            {
                const char* line_c = line.c_str();
                if (regex_match(line_c, conditionRegex))
                {
                    const std::vector<int> submatches = { 1, 2 };
                    sregex_token_iterator iter(
                        line.begin(), line.end(), conditionRegex, submatches);
                    sregex_token_iterator end;
                    // name
                    action_name = iter->str();
                    iter++;
                    // args
                    action_args = iter->str();
                    iter++;

                    parser = ACTION_PRECONDITION;
                }
                else
                {
                    cout << "Action not specified correctly." << endl;
                    throw;
                }
            }
            else if (parser == ACTION_PRECONDITION)
            {
                const char* line_c = line.c_str();
                if (regex_match(line_c, precondRegex))
                {
                    const std::vector<int> submatches = { 1, 2 };
                    sregex_token_iterator iter(
                        line.begin(), line.end(), conditionRegex, submatches);
                    sregex_token_iterator end;

                    while (iter != end)
                    {
                        // name
                        string predicate = iter->str();
                        iter++;
                        // args
                        string args = iter->str();
                        iter++;

                        bool truth;

                        if (predicate[0] == '!')
                        {
                            predicate = predicate.substr(1);
                            truth = false;
                        }
                        else
                        {
                            truth = true;
                        }

                        Condition precond(predicate, parse_symbols(args), truth);
                        preconditions.insert(precond);
                    }

                    parser = ACTION_EFFECT;
                }
                else
                {
                    cout << "Precondition not specified correctly." << endl;
                    throw;
                }
            }
            else if (parser == ACTION_EFFECT)
            {
                const char* line_c = line.c_str();
                if (regex_match(line_c, effectRegex))
                {
                    const std::vector<int> submatches = { 1, 2 };
                    sregex_token_iterator iter(
                        line.begin(), line.end(), conditionRegex, submatches);
                    sregex_token_iterator end;

                    while (iter != end)
                    {
                        // name
                        string predicate = iter->str();
                        iter++;
                        // args
                        string args = iter->str();
                        iter++;

                        bool truth;

                        if (predicate[0] == '!')
                        {
                            predicate = predicate.substr(1);
                            truth = false;
                        }
                        else
                        {
                            truth = true;
                        }

                        Condition effect(predicate, parse_symbols(args), truth);
                        effects.insert(effect);
                    }

                    env->add_action(
                        Action(action_name, parse_symbols(action_args), preconditions, effects));

                    preconditions.clear();
                    effects.clear();
                    parser = ACTION_DEFINITION;
                }
                else
                {
                    cout << "Effects not specified correctly." << endl;
                    throw;
                }
            }
        }
        input_file.close();
    }

    else
        cout << "Unable to open file";

    return env;
}

//=====================================================================================================================

struct State_hasher
{
    size_t operator()(const unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> &gconds) const
    {
        vector<string> gconds_vector;
        for(const auto &gcond:gconds)
        {
            gconds_vector.emplace_back(gcond.toString());
        }
        sort(gconds_vector.begin(), gconds_vector.end());
        string total_string;
        for(const auto elt:gconds_vector)
        {
            total_string = total_string + elt;
        }
        return hash<string>{}(total_string);
    }
};

//=====================================================================================================================

template <typename T>
void print_unordered_set(const unordered_set<T> &u_set)
{
    for(const auto &x:u_set)
    {
        cout<<x<<endl;
    }
}

//=====================================================================================================================

template <typename T>
void print_list(const list<T> &my_list)
{
    for(const auto &x:my_list)
    {
        cout<<x<<"->";
    }
    cout<<endl;
}

//=====================================================================================================================

struct Node
{
    unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> gc;
    unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> parent;
    GroundedAction parent_action;
    //Consists of the state and the action pair which yields this state
    double gcost;
    double hcost;
    double fcost;
    static double heuristic_weight;

    //---------------------------------------------------------

    Node(unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> new_gc,
         unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> new_parent,
         GroundedAction new_parent_action,
         double g_cost,
         double h_cost):
         gc(new_gc),parent(new_parent),parent_action(new_parent_action),gcost(g_cost),hcost(h_cost){
        fcost = calculate_fcost();
    }

    Node(unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> new_gc,
         double g_cost):
         gc(new_gc),gcost(g_cost),hcost(0){
        fcost = calculate_fcost();
    }

    Node():
        gcost(0),hcost(0){
        fcost = calculate_fcost();
    }

    double calculate_fcost()
    {
        return gcost + hcost;
    }

    double calculate_hcost(const unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> &goal_coordinate) const
    {
        //Write heuristics formula;
        return 0;
    }

    void set_fcost(const double &new_f_cost)
    {
        fcost = new_f_cost;
    }

    void set_hcost(const double &new_h_cost)
    {
        hcost = new_h_cost;
        Node::set_fcost(calculate_fcost());
    }

    void set_gcost(const double &new_g_cost)
    {
        gcost = new_g_cost;
        Node::set_fcost(calculate_fcost());
    }

    void print_node() const
    {
        cout<<"State is: "<<endl;
        for(const auto &elt:gc)
        {
            cout<<elt.toString()<<"\t";
        }
        cout<<endl<<"----------------------------------------------------------------------"<<endl;

        if(!parent.size())
            cout<<"No Parent"<<endl;
        else
        {
            cout<<"Parents is: "<<endl;
            for(const auto &elt:parent)
            {
                cout<<elt.toString()<<"\t";
            }
            cout<<endl<<"-----ACTION------"<<endl;
            cout<<parent_action.toString()<<endl;
        }
        cout<<"----------------------------------------------------------------------"<<endl;
        cout<<"gcost: "<<gcost<<"\t"<<"hcost: "<<hcost<<"\t"<<"fcost: "<<fcost<<endl;
        cout<<"===================================================================="<<endl;
    }
};

//=====================================================================================================================

inline bool operator < (const Node& lhs, const Node& rhs)
{
    return lhs.fcost < rhs.fcost;
}

//=====================================================================================================================

inline bool operator == (const Node& lhs, const Node& rhs)
{
    return lhs.gc == rhs.gc;
}

//=====================================================================================================================

inline bool operator != (const Node& lhs, const Node& rhs)
{
    return !(lhs==rhs);
}

//=====================================================================================================================

struct Node_Comp{
    bool operator()(const Node &a, const Node &b){
        return a.fcost>b.fcost;
    }
};

//=====================================================================================================================

vector<list<string>> create_next_round_of_combinations(vector<list<string>> present_symbols_list,
                                                        const unordered_set<string> &all_symbols,
                                                        vector<unordered_set<string>> &symbols_present_in_list)
{
    vector<list<string>> new_symbols_list;
    vector<unordered_set<string>> new_symbols_present_in_list;
    int counter = 0;
    for(const auto &present_symbol:present_symbols_list)
    {
        for(const auto &symbol:all_symbols)
        {
            if(!symbols_present_in_list[counter].count(symbol))
            {
                new_symbols_list.emplace_back(present_symbol);
                new_symbols_list[new_symbols_list.size()-1].emplace_back(symbol);
                new_symbols_present_in_list.emplace_back(symbols_present_in_list[counter]);
                new_symbols_present_in_list[new_symbols_present_in_list.size()-1].insert(symbol);
            }
        }
        counter++;
    }
    symbols_present_in_list = new_symbols_present_in_list;
    return std::move(new_symbols_list);
}

//=====================================================================================================================

unordered_map<int,vector<list<string>>> get_all_possible_permutations(const unordered_set<Action, ActionHasher, ActionComparator> &actions,
                                                                       const unordered_set<string> &all_symbols)
{
    unordered_set<int> action_arg_count;   //This maintains count of the arguments in each action
    int max = -1;
    for(const auto &action:actions)
    {
        int num_action_arguments = action.get_args().size();
        action_arg_count.insert(num_action_arguments);
        if(max<num_action_arguments)
            max = num_action_arguments;
    }

    unordered_set<string> present_symbols_set = all_symbols;   // This maintains the present combinations of symbols.
    vector<list<string>> present_symbols_list;
    vector<unordered_set<string>> symbols_present_in_list;
    for(const auto &symbol:all_symbols)
    {
        present_symbols_list.emplace_back(list<string> {symbol});
        symbols_present_in_list.emplace_back(unordered_set<string> {symbol});
    }

    unordered_map<int,vector<list<string>>> arg_count_symbol_combination_map; //Key is the number of arguments in the action and values are all possible combinations of symbols
    for(int i=1;i<=max;i++)
    {
        if(i!=1)
        {
            present_symbols_list = create_next_round_of_combinations(std::move(present_symbols_list),all_symbols,symbols_present_in_list);
        }

        if(action_arg_count.count(i))
        {
            arg_count_symbol_combination_map[i] = present_symbols_list;
        }
    }

    return std::move(arg_count_symbol_combination_map);
}

//=====================================================================================================================

unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator>
        get_grounded_conditions(const unordered_set<Condition, ConditionHasher, ConditionComparator> &conditions,
                                unordered_map<string,string> placeholder_to_symbol_map)
{
    unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> grounded_conditions;
    for(const auto &condition:conditions)
    {
        auto new_predicate = condition.get_predicate();
        auto new_truth = condition.get_truth();
        auto old_args = condition.get_args();
        list<string> new_args;
        for(auto it=old_args.begin();it!=old_args.end();it++)
        {
            if(placeholder_to_symbol_map.count(*it))
                new_args.emplace_back(placeholder_to_symbol_map[*it]);
            else
                new_args.emplace_back(*it);     //This is for cases eg. MovetoTable(b,x) Effect would include On(b,Table) But table is not in placeholder_to_symbol_map
        }
        grounded_conditions.insert(GroundedCondition{std::move(new_predicate),std::move(new_args),std::move(new_truth)});
    }
    return std::move(grounded_conditions);
}

//=====================================================================================================================

vector<GroundedAction> get_all_possible_actions(const unordered_set<Action, ActionHasher, ActionComparator> &actions,
                                                const unordered_set<string> &all_symbols)
{
    auto permutation_map = get_all_possible_permutations(actions,all_symbols);
    vector<GroundedAction> all_actions;
    for(const auto action:actions)
    {
        auto action_name = action.get_name();
        auto precond = action.get_preconditions();
        auto effects = action.get_effects();
        auto args = action.get_args();
        int symbols_in_action = args.size();
        auto possible_permutations = permutation_map[symbols_in_action];
        for(int i=0;i<possible_permutations.size();i++)
        {
            assert(args.size()==possible_permutations[i].size());
            auto it_permutation = possible_permutations[i].begin();
            unordered_map<string,string> placeholder_to_symbol_map;
            for(auto it_args = args.begin();it_args!=args.end();it_args++,it_permutation++)
            {
                placeholder_to_symbol_map[*it_args] = *it_permutation;
            }
//            cout<<"Action name: "<<action_name<<endl;
//            for(const auto &elt:placeholder_to_symbol_map)
//            {
//                cout<<elt.first<<":"<<elt.second<<endl;
//            }
            auto grounded_preconditions = get_grounded_conditions(std::move(precond),placeholder_to_symbol_map);
//            for(const auto &elt:grounded_preconditions)
//            {
//                cout<<elt.toString()<<endl;
//            }
            auto grounded_effects = get_grounded_conditions(std::move(effects),placeholder_to_symbol_map);
//            cout<<"-----------------------------------------------"<<endl;
//            for(const auto &elt:grounded_effects)
//            {
//                cout<<elt.toString()<<endl;
//            }
//            cout<<"==============================================================="<<endl;
            all_actions.emplace_back(GroundedAction{action_name,possible_permutations[i],std::move(grounded_preconditions),std::move(grounded_effects)});
        }
    }
    return std::move(all_actions);
}

//=====================================================================================================================

unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> get_new_grounded_conditions(unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> present_grounded_conditions,
                                                                                                                   const unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> &action_effects)
{
    for(auto effect:action_effects)
    {
        if(effect.get_truth())
            present_grounded_conditions.insert(effect);
        else
        {
            effect.set_truth(true);
            present_grounded_conditions.erase(present_grounded_conditions.find(effect));
        }
    }

    return std::move(present_grounded_conditions);
}


//=====================================================================================================================

bool are_all_elements_present_in_collection(const unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> &subset_of_conditions,
                                            const unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> &superset_of_conditions)
{
    for(const auto &subset_condition:subset_of_conditions)
    {
        if(!superset_of_conditions.count(subset_condition))
        {
            return false;
        }
    }
    return true;
}

//=====================================================================================================================

void expand_state(const Node &present_node,
                  const vector<GroundedAction> &action_list,
                  unordered_map<unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator>,Node,State_hasher> &node_map,
                  priority_queue<Node, vector<Node>, Node_Comp> &open,
                  const unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> &goal_ground_conditions,
                  const unordered_set<unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator>,State_hasher> &closed)
{

    for(const auto &gaction:action_list)
    {

        auto preconds = gaction.get_preconditions();

        if(!are_all_elements_present_in_collection(preconds,present_node.gc))
            continue;
//        cout<<gaction.toString()<<endl;
        const auto new_grounded_conditions = get_new_grounded_conditions(present_node.gc,gaction.get_effects());
        if(!closed.count(new_grounded_conditions))
        {
            if(!node_map.count(new_grounded_conditions))
            {

                node_map.insert({new_grounded_conditions,Node{new_grounded_conditions,present_node.gc,gaction,present_node.gcost+1,0}});
                auto new_h_cost = node_map.at(new_grounded_conditions).calculate_hcost(goal_ground_conditions);
                node_map.at(new_grounded_conditions).set_hcost(new_h_cost);
                open.push(node_map.at(new_grounded_conditions));
            }
            else
            {
                if(present_node.gcost+1<node_map.at(new_grounded_conditions).gcost)
                {

                    node_map.at(new_grounded_conditions).set_gcost(present_node.gcost+1);
                    node_map.at(new_grounded_conditions).parent = present_node.gc;
                    node_map.at(new_grounded_conditions).parent_action = gaction;
                    open.push(node_map.at(new_grounded_conditions));
                }
            }

        }
    }
}

////=====================================================================================================================

list<GroundedAction> back_track(unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> present_grounded_conditions,
                                const unordered_map<unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator>,Node,State_hasher> &node_map,
                                list<GroundedAction> actions,
                                const unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> &start_gc)
{
    cout<<"Starting backtracking"<<endl;
    while(!are_all_elements_present_in_collection(present_grounded_conditions,start_gc))
    {
        actions.emplace_back(node_map.at(present_grounded_conditions).parent_action);
        present_grounded_conditions = node_map.at(present_grounded_conditions).parent;
    }
    actions.reverse();
    return std::move(actions);
}

//=====================================================================================================================

list<GroundedAction> planner(Env* env)
{

    list<GroundedAction> actions;
    priority_queue<Node, vector<Node>, Node_Comp> open;
    unordered_map<unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator>,Node,State_hasher> node_map; //This serves as my map since it's an implicit directed graph
    unordered_set<unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator>,State_hasher> closed;
    const auto action_list = get_all_possible_actions(env->get_all_actions(),env->get_symbols());
    const auto start_gc = env->get_initial_conditions();
    const auto goal_gc = env->get_goal_conditions();
    int node_count = 0;
    Node start_node{start_gc,0};
    node_map.insert({start_gc,start_node});
    open.push(start_node);
    bool goal_reached = false;
    int loop_iteration_counter = 1;
    auto final_grounded_conditions = unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> {GroundedCondition{"",list<string> {}}};
//    node_map.at(start_gc).print_node();
    while(!open.empty())
    {
//        cout<<"Loop iteration counter "<<loop_iteration_counter<<endl;
//        cout<<"--------------------------"<<endl;
        const auto node_to_expand = open.top();
//        node_to_expand.print_node();
        open.pop();

        //Note below: We have this condition instead of checking if goal_gc is closed or not as goal can be partially specified.
        //In that case the hashes won't match, but goal condition is satisfied.
        if(are_all_elements_present_in_collection(goal_gc,node_to_expand.gc))
        {
            cout<<"Goal has been found"<<endl;
            goal_reached = true;
            final_grounded_conditions = node_to_expand.gc;
            break;
        }

        if(!closed.count(node_to_expand.gc))
        {
            closed.insert(node_to_expand.gc);
            expand_state(node_to_expand,action_list,node_map,open,goal_gc,closed);
        }

        loop_iteration_counter++;
    }

    if(!goal_reached)
    {
        cout<<"No path to goal"<<endl;
    }
    else
    {
        cout<<"PATH FOUND"<<endl;
        actions = back_track(final_grounded_conditions,node_map,std::move(actions),start_gc);
    }

    return std::move(actions);

    /// Use the index_in_map to backtrack. Keep selecting parents with lower gcost while backtracking
}

int main(int argc, char* argv[])
{
    // DO NOT CHANGE THIS FUNCTION
    char* filename = (char*)("example.txt");
    if (argc > 1)
        filename = argv[1];

    cout << "Environment: " << filename << endl << endl;
    Env* env = create_env(filename);
    if (print_status)
    {
        cout << *env;
    }

    list<GroundedAction> actions = planner(env);

    cout << "\nPlan: " << endl;
    for (GroundedAction gac : actions)
    {
        cout << gac << endl;
    }

    return 0;
}