#ifndef LANGUAGEDEFINATIONXMLPARSER_H
#define LANGUAGEDEFINATIONXMLPARSER_H

#include <string>
#include <list>

#include <QtXml/QDomDocument>

#include "rapidxml.hpp"
#include "pcre.h"

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&); \
    void operator=(const TypeName&)

class Language;
class Contain;

struct StackItem
{
    StackItem(Language *l=NULL, Contain* c=NULL)
    {
        lan = l;
        con = c;
    }

    Language *lan;
    Contain *con;
};

struct FindResult
{
    FindResult(){start=-1; end=-1;}
    FindResult(int s, int e){start=s; end=e;}
    bool isValid(){return start!=-1||end!=-1;}
    int start;
    int end;
};

class RegExp
{
public:
    RegExp();
    bool compile(const std::string &pattern, bool isCaseSensitive, bool isGlobal=false);
    FindResult exec(const char* code, int len);
    bool test(const char*code, int len);
    int getLastIndex() const;
    void setLastIndex(int index);
    bool isValid() const;
    ~RegExp();
private:
    DISALLOW_COPY_AND_ASSIGN(RegExp);
private:
    pcre16* re;
    bool global;
    int lastIndex;
};

class Keywords
{
public:
    enum KeywordsType {
        Keyword,
        Literal,
        Constant,
        Type,
        Command,
        Property,
        BuiltIn,
        Title,
        NotFound//for return
    };

    Keywords(KeywordsType t, const std::string &kw);
    const std::string& getKeyword();
    const int getType(){return type;}
    static const char* getKeyTypeString(int kt);

private:
    KeywordsType type;
    std::string k;
};

class HighlighterUtil
{
public:
    static std::string joinKeywords(std::list<Keywords> keywords, char sep);
    static std::string joinStrings(std::list<std::string> sl, char sep);
};

class Contain
{
public:
    Contain();
    ~Contain();
    void setShowClassName(bool s);
    bool isShowClassName();
    void setEndWithParent(bool e);
    bool isEndWithParent();
    void setBeginWithKeyword(bool b);
    bool isBeginWithKeyword();
    void setRelevance(int relevance);
    int getRelevance();
    void setRefLanguageKeywords(bool r);
    bool isRefLanguageKeywords();
    bool isHaveSubLanguage();
    void setSubLanugage(const char *subLanguage);
    const std::string getSubLanguage();
    void setName(const std::string &name);
    const char * getName();
    std::string getRealName();
    void setBegin(const std::string &begin);
    const std::string getBegin();
    void setEnd(const std::string &end);
    const std::string& getEnd();
    void setLexems(const std::string &lexems);
    void setIllegal(const std::string &illegal);
    void addKeyword(Keywords::KeywordsType kt, const std::string &keyword);
    void addRefContain(Contain *contain);
    void compile(Language *lan);
    Contain *findMatchedContain(const std::string &match);
    RegExp& getBeginRe();
    RegExp& getEndRe();
    RegExp& getTerminatorsRe();
    RegExp& getLexemsRe();
    bool isRef();
    void setRef(bool r);
    void setParent(Contain *contain);
    Contain* getParent();
    const std::string& getTerminatorEnd();
    bool isReturnBegin();
    void setReturnBegin(bool b);
    void setReturnEnd(bool b);
    bool isExcludeBegin();
    bool isReturnEnd();
    bool isExcludeEnd();
    void setExcludeEnd(bool b);
    const std::list<Keywords>& getKeywords();
    int matchKeyword(const std::string &k);
    void setStarts(Contain *contain);
    Contain* getStarts();
    void setRefLanguageContains(bool b);
    bool isRefLanguageContains();
    bool isStarts();
private:
    bool showClassName;
    bool endsWithParent;
    bool beginWithKeyword;
    bool refLanguageKeywords;
    bool ref;
    bool returnBegin;
    bool excludeBegin;
    bool returnEnd;
    bool excludeEnd;
    bool refLanguageContains;
    int relevance;
    std::string name;
    std::string begin;
    RegExp beginRe;
    std::string end;
    RegExp endRe;
    std::string illegal;
    RegExp illegalRe;
    std::string lexems;
    RegExp lexemsRe;
    std::string terminatorEnd;
    RegExp terminatorsRe;
    std::list<Keywords> keywords;
    std::list<Contain *> refContains;
    Contain *parent;
    Contain *starts;
    std::string subLanguage;
};

class Language
{
public:
    Language();
    ~Language();
    void addKeyword(Keywords::KeywordsType kt, const std::string &keyword);
    void addLiteral(const std::string &literal);
    void addConstant(const std::string &constant);
    void addType(const std::string &type);
    void addCommand(const std::string &command);
    void addProperty(const std::string &property);
    void addBuiltIn(const std::string &builtIn);
    void addContain(Contain *contain);
    void setCaseSensitive(bool s);
    bool isCaseSensitive();
    void setName(const std::string &name);
    void setIllegal(const std::string &illegal);
    void setLexems(const std::string &lexems);
    const std::string& getLexems();
    const std::list<Keywords>& getKeywords();
    int matchKeyword(const std::string &k);
    Contain *findRefContain(const char *name);
    Contain *findMatchedContain(const std::string &match);
    void printDebugInfo();
    bool isCompiled();
    void compileLanguage();
    RegExp& getTerminatorsRe();
    RegExp& getLexemsRe();
    std::list<Contain *>& getContains();
private:
    bool compiled;
    bool caseSensitive;
    std::string name;

    std::string lexems;
    RegExp lexemsRe;

    RegExp terminatorsRe;
    //keywords type
    std::list<Keywords> keywords;

    std::string illegal;
    RegExp illegalRe;
    std::list<Contain *> contains;
};

class LanguageDefinationXmlParser
{
public:
    LanguageDefinationXmlParser();
    Language *startParse(const char *name, char *src);
private:
    void parseLanguageNode(rapidxml::xml_node<> *languageNode, Language *lan);
    void parseKeywordsNode(rapidxml::xml_node<> *keywordsNode, Language *lan);
    void parseContainsNode(rapidxml::xml_node<> *containsNode, Language *lan);
    bool parseContainNode(rapidxml::xml_node<> *node, Contain *contain, Language *lan);
private:
};

#endif // LANGUAGEDEFINATIONXMLPARSER_H
