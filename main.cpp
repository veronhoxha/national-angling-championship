#include <iostream>
#include "library/seqinfileenumerator.hpp"
#include "library/stringstreamenumerator.hpp"
#include "library/counting.hpp"
#include "library/summation.hpp"
#include "library/linsearch.hpp"

/*
At every competition of the National Angling Championship, the results of the
competitors were recorded and put into a text file. Every line of the file contains the name
of the angler the ID of the competition (string without spaces), and the species and the
size of the caught fishes (pair of a string without spaces and a natural number). Data is
separated by space or tab. The lines of the file are ordered by contest ID. The file is
assumed to be in the correct form.
Sample line in the file:

Peter LAC0512 carp 45 carp 53 catfish 96

(1) Is there an angler who has caught only and at least three catfishes on one of his
contests? Give the contest ID, too.

(2) How many contests there are where at least one angler has caught only and at least
three catfishes?
*/

using namespace std;

struct Fish
{
    string fish;
    int size;

    friend istream& operator>>(istream &is, Fish &f)
    {
        is >> f.fish >> f.size;
        return is;
    }
};


struct Result
{
    bool allCatfish;
    int catfishCnt;

    Result () {}

    Result(bool aC, int cC) : allCatfish(aC), catfishCnt(cC) {}
};


class optSum : public Summation<Fish, Result>
{
     Result func(const Fish& e) const override {return Result (e.fish == "catfish", e.fish == "catfish" ? 1 : 0) ;}
     Result neutral() const override {return Result(true, 0);}
     Result add( const Result& a, const Result& b) const override
     {
         return Result (a.allCatfish && b.allCatfish, a.catfishCnt + b.catfishCnt);

     }
};


struct Competition
{
    string name;
    string comp;
    bool allCatfish;
    int catfishCnt;

    friend istream& operator>> (istream &is, Competition &c);
};


istream& operator>> (istream &is, Competition &c)
{
    string line;
    getline(is, line);

    stringstream ss(line);
    ss >> c.name >> c.comp;

    optSum pr;
    StringStreamEnumerator<Fish> ssenor (ss);

    pr.addEnumerator(&ssenor);

    pr.run();

    c.allCatfish = pr.result().allCatfish;
    c.catfishCnt = pr.result().catfishCnt;
    return is;
}


class isThereAnAngler : public LinSearch<Competition>
{
    bool cond(const Competition& e) const override
    {
        return e.allCatfish && e.catfishCnt >= 3;
    }
};

struct Contest {
    string ID;
    bool anglersCondition;

    Contest() {}
    Contest(string ids, int aC): ID(ids), anglersCondition(aC) {}
};


class countAll : public Summation<Competition, Contest>
{
private:
    string _name;
public:
    countAll(const string &name) : _name(name) {}
protected:
    Contest func(const Competition& e) const override { return Contest(e.comp, e.allCatfish && e.catfishCnt >= 3 ? 1 : 0);}
    Contest neutral() const override { return Contest("", false); }
    Contest add( const Contest& a, const Contest& b) const override
    {
        return Contest(a.ID, a.anglersCondition || b.anglersCondition);
    }

    bool whileCond(const Competition& current) const { return current.comp == _name; }
    void first() override { }
};


class ContestEnor : public Enumerator<Contest>
{
private:
    SeqInFileEnumerator<Competition>* _f;
    Contest _contest;

    bool _end;

    bool _empty;

public:
    ContestEnor(const string &fname) : _empty(true) { _f = new SeqInFileEnumerator<Competition>(fname);}
    ~ContestEnor() { delete _f;}

    void first() override { _f->first(); next(); if (!end()) _empty = false; }
    void next() override;
    bool end() const override { return _end;}
    Contest current() const override {return _contest;}

    bool isEmpty() const { return _empty;}
};


void ContestEnor::next()
{
    _end = _f->end();
    if (_end) return;

    _contest.ID = _f->current().comp;

    countAll pr(_contest.ID);
    pr.addEnumerator(_f);

    pr.run();

    _contest.anglersCondition = pr.result().anglersCondition ? 1 : 0;
}


class Output : public Summation<Contest, ostream>
{

public:
    Output(ostream *o) : Summation<Contest, ostream>::Summation(o) {}
protected:
    std::string func(const Contest& e) const override
    {
        ostringstream oss;
        oss << "Contest ID: " << e.ID << ", condition: " << e.anglersCondition << endl;
        return oss.str();
    }
    bool cond(const Contest& e) const { return e.anglersCondition; }
};


class OutputCnt : public Counting<Contest>
{
    bool cond(const Contest& e) const override
    {
        return e.anglersCondition;
    }
};


int main()
{
    try {

        isThereAnAngler pr;

        SeqInFileEnumerator<Competition> enor("input.txt");
        pr.addEnumerator(&enor);

        pr.run();

        OutputCnt prOutput;
        ContestEnor contestEnor("input.txt");

        prOutput.addEnumerator(&contestEnor);
        prOutput.run();

        if (contestEnor.isEmpty())
        {
            cout << "File is empty!\n";
        }
        else if (pr.found())
        {
            cout << "Yes there is, " << pr.elem().name << " with Contest ID: " << pr.elem().comp << endl;
        }
        else
        {
            cout << "There is no angler with only and at least three catfishes caught!\n";
        }

        if (!contestEnor.isEmpty())
        {
            cout << "There is/are " << prOutput.result() << " contest/s where at least one angler has caught only and at least three catfishes!" << endl;
        }

    } catch (SeqInFileEnumerator<Competition>::Exceptions ex)
    {
        cout << "File not found!\n";
    }
    return 0;
}
