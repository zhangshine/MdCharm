#include <stdlib.h>
#include <cassert>
#include <stdio.h>
#include <QString>

#include "languagedefinationxmlparser.h"

using namespace std;
using namespace rapidxml;
//------------------------------ RegExp ----------------------------------------
RegExp::RegExp()
{
    re = NULL;
    lastIndex = 0;
}

bool RegExp::compile(const string &pattern, bool isCaseSensitive, bool isGlobal)
{
    const char *error;
    int errorOffset;
    int option = PCRE_MULTILINE;
    if(!isCaseSensitive){
        option |= PCRE_CASELESS;
    }
    QString realPattern = QString::fromUtf8(pattern.c_str(), pattern.length());
    re = pcre16_compile(realPattern.utf16(), option, &error, &errorOffset, NULL);
    assert(re);
    global = isGlobal;
    return re ? true : false;
}

FindResult RegExp::exec(const char *code, int len)
{
    if(!re)
        return FindResult();
    if(global && lastIndex>=len)
        return FindResult();
    int spce[21];
    memset(spce, 0, 21);
    QString realCode = QString::fromUtf8(code, len);
    int rc = pcre16_exec(re, NULL, realCode.utf16(), len, global ? lastIndex : 0, 0, spce, 21);
    if(rc<0)
        return FindResult();
    else {
        FindResult fr(spce[0], spce[1]);
        lastIndex = spce[1];
        return fr;
    }
}

bool RegExp::test(const char *code, int len)
{
    int spec[21];
    memset(spec, 0, 21);
    QString realCode = QString::fromUtf8(code, len);
    int rc = pcre16_exec(re, NULL, realCode.utf16(), len, 0, 0, spec, 21);
    return rc>=0;
}


int RegExp::getLastIndex() const
{
    return lastIndex;
}

void RegExp::setLastIndex(int index)
{
    lastIndex = index;
}

bool RegExp::isValid() const
{
    return re ? true : false;
}

RegExp::~RegExp()
{
    if(re)
        pcre16_free(re);
}

//------------------------------- Keywords -------------------------------------
Keywords::Keywords(KeywordsType t, const string &kw)
{
    type = t;
    k = kw;
}

const string& Keywords::getKeyword()
{
    return k;
}

const char* Keywords::getKeyTypeString(int kt)
{
    static const char *errorTexts[] = {
        "keyword",
        "literal",
        "constant",
        "type",
        "command",
        "property",
        "built_in",
        "title",
        ""
    };
    if(kt>=8||kt<0)
        return errorTexts[8];
    return errorTexts[kt];
}

//---------------------------- HighlightUtli -----------------------------------
string HighlighterUtil::joinKeywords(list<Keywords> keywords, char sep)
{
    string result;
    for(list<Keywords>::iterator it=keywords.begin(); it!=keywords.end(); it++){
        result.append((*it).getKeyword());
        result.push_back(sep);
    }
    result.erase(result.length()-1, 1);
//    printf("%s\n", result.c_str());
    return result;
}

string HighlighterUtil::joinStrings(list<string> sl, char sep)
{
    string result;
    for(list<string>::iterator it=sl.begin(); it!=sl.end(); it++){
//        if(it->empty())
//            continue;
        result.append((*it));
        result.push_back(sep);
    }
    result.erase(result.length()-1, 1);
//    printf("%s\n", result.c_str());
    return result;
}

//------------------------------- Contain --------------------------------------
Contain::Contain()
{
    showClassName = true;
    refLanguageKeywords = false;
    endsWithParent = false;
    beginWithKeyword = false;
    ref = false;
    returnBegin = false;
    excludeBegin = false;
    returnEnd = false;
    excludeEnd = false;
    refLanguageContains = false;
    relevance = 1;
    parent = NULL;
    starts = NULL;
}

Contain::~Contain()
{
}



void Contain::setShowClassName(bool s)
{
    showClassName = s;
}

bool Contain::isShowClassName()
{
    return showClassName;
}

void Contain::setEndWithParent(bool e)
{
    endsWithParent = e;
}

bool Contain::isEndWithParent()
{
    return endsWithParent;
}

void Contain::setBeginWithKeyword(bool b)
{
    beginWithKeyword = b;
}

bool Contain::isBeginWithKeyword()
{
    return beginWithKeyword;
}

void Contain::setRelevance(int relevance)
{
    this->relevance = relevance;
}

int Contain::getRelevance()
{
    return relevance;
}

void Contain::setRefLanguageKeywords(bool r)
{
    refLanguageKeywords = r;
}

bool Contain::isRefLanguageKeywords()
{
    return refLanguageKeywords;
}

bool Contain::isHaveSubLanguage()
{
    return !subLanguage.empty();
}

void Contain::setSubLanugage(const char * subLanguage)
{
    this->subLanguage = subLanguage;
}

const string Contain::getSubLanguage()
{
    return subLanguage;
}

void Contain::setName(const string &name)
{
    this->name = name;
}

const char *Contain::getName()
{
    return name.c_str();
}

std::string Contain::getRealName()
{
//    return name;
    return name.substr(0, name.find_first_of('|'));
}

void Contain::setBegin(const string &begin)
{
    if(begin.empty()) //TODO: patch for xml whitespace, try find a more better method
        this->begin.push_back(' ');
    else
        this->begin = begin;
}

const string Contain::getBegin()
{
    if(!beginWithKeyword)
        return begin;
    return "\\b("+HighlighterUtil::joinKeywords(keywords, '|')+")\\s";
}

void Contain::setEnd(const string &end)
{
    if(end.empty())//TODO: same with setBegin()
        this->end.push_back(' ');
    else
        this->end = end;
}

const string& Contain::getEnd()
{
    return end;
}

void Contain::setLexems(const string &lexems)
{
    this->lexems = lexems;
}

void Contain::setIllegal(const string &illegal)
{
    this->illegal = illegal;
}

void Contain::addKeyword(Keywords::KeywordsType kt, const string &keyword)
{
    keywords.push_back(Keywords(kt, keyword));
}

void Contain::addRefContain(Contain *contain)
{
    refContains.push_back(contain);
}

void Contain::compile(Language *lan)
{
    if(keywords.size()>0 || refLanguageKeywords)
        if(!lexems.empty())
            lexemsRe.compile(lexems, lan->isCaseSensitive(), true);
        else
            lexemsRe.compile("[a-zA-Z][a-zA-Z0-9_]*", lan->isCaseSensitive(), true);

    if(beginWithKeyword)
        begin = "\\b("+HighlighterUtil::joinKeywords(keywords, '|')+")\\s";
    if(!begin.empty())
        beginRe.compile(begin, lan->isCaseSensitive());
    else
        beginRe.compile("\\B|\\b", lan->isCaseSensitive());
    if(end.empty() && !endsWithParent)
        end = "\\B|\\b";
    if(!end.empty())
        endRe.compile(end, lan->isCaseSensitive());
    terminatorEnd = end.empty() ? "" : end;
    if(getParent() && endsWithParent && getParent()!=this && !getParent()->getTerminatorEnd().empty())
        if(!terminatorEnd.empty())
            terminatorEnd.append("|"+getParent()->getTerminatorEnd());
        else
            terminatorEnd.append(getParent()->getTerminatorEnd());

    if(!illegal.empty())
        illegalRe.compile(illegal, lan->isCaseSensitive());
    //TODO: starts

    list<string> terminators;
    for(list<Contain *>::iterator it=refContains.begin(); it!=refContains.end(); it++){
        Contain *contain = *it;
        terminators.push_back(contain->getBegin());
    }
    if(refLanguageContains){
        list<Contain *>& lanContains = lan->getContains();
        for(list<Contain *>::iterator it=lanContains.begin(); it!=lanContains.end(); it++){
            Contain *contain = *it;
            if(!contain->isRef()){
                terminators.push_front(contain->getBegin());
            }
        }
    }
    if(!terminatorEnd.empty())
        terminators.push_back(terminatorEnd);
    if(!illegal.empty())
        terminators.push_back(illegal);
//    printf("%s ", name.c_str());
    if(!terminators.empty())
        terminatorsRe.compile(HighlighterUtil::joinStrings(terminators, '|'), true, true);
}

Contain* Contain::findMatchedContain(const string &match)
{
    if(parent && parent->getStarts() && parent->getStarts()==this)
        return NULL;
    for(list<Contain *>::iterator it=refContains.begin(); it!=refContains.end(); it++){
        Contain *contain = *it;
        if(contain->getBeginRe().isValid()){
            FindResult fr = contain->getBeginRe().exec(match.c_str(), match.length());
            if(fr.isValid())
                return contain;
        }
    }
    return NULL;
}

RegExp& Contain::getBeginRe()
{
    return beginRe;
}

RegExp& Contain::getEndRe()
{
    return endRe;
}

RegExp& Contain::getTerminatorsRe()
{
    return terminatorsRe;
}

RegExp& Contain::getLexemsRe()
{
    return lexemsRe;
}

bool Contain::isRef()
{
    return ref;
}

void Contain::setRef(bool r)
{
    ref = r;
}

void Contain::setParent(Contain *contain)
{
    parent = contain;
}

Contain* Contain::getParent()
{
    if(parent && parent->getStarts() && parent->getStarts()==this)
        return parent->getParent();
    return parent;
}

const string& Contain::getTerminatorEnd()
{
    return terminatorEnd;
}

bool Contain::isExcludeBegin()
{
    return excludeBegin;
}

bool Contain::isReturnBegin()
{
    return returnBegin;
}

void Contain::setReturnBegin(bool b)
{
    returnBegin = b;
}

void Contain::setReturnEnd(bool b)
{
    returnEnd = b;
}

bool Contain::isReturnEnd()
{
    return returnEnd;
}
bool Contain::isExcludeEnd()
{
    return excludeEnd;
}

void Contain::setExcludeEnd(bool b)
{
    excludeEnd = b;
}

const std::list<Keywords>& Contain::getKeywords()
{
    return keywords;
}

int Contain::matchKeyword(const string &k)
{
    for(list<Keywords>::iterator it=keywords.begin(); it!=keywords.end(); it++){
        if(it->getKeyword()==k)
            return it->getType();
    }
    return Keywords::NotFound;
}

void Contain::setStarts(Contain *contain)
{
    starts = contain;
}

Contain* Contain::getStarts()
{
    return starts;
}

void Contain::setRefLanguageContains(bool b)
{
    refLanguageContains = b;
}

bool Contain::isRefLanguageContains()
{
    return refLanguageContains;
}

bool Contain::isStarts()
{
    return parent && parent->getStarts() && parent->getStarts()==this;
}

//------------------------------- Language -------------------------------------
Language::Language()
{
    compiled = false;
    caseSensitive = true;
}

void Language::addKeyword(Keywords::KeywordsType kt, const string &keyword)
{
    keywords.push_back(Keywords(kt, keyword));
}

void Language::addLiteral(const string &literal)
{
    keywords.push_back(Keywords(Keywords::Literal, literal));
}

void Language::addConstant(const string &constant)
{
    keywords.push_back(Keywords(Keywords::Constant, constant));
}

void Language::addType(const string &type)
{
    keywords.push_back(Keywords(Keywords::Type, type));
}

void Language::addCommand(const string &command)
{
    keywords.push_back(Keywords(Keywords::Command, command));
}

void Language::addProperty(const string &property)
{
    keywords.push_back(Keywords(Keywords::Property, property));
}

void Language::addBuiltIn(const string &builtIn)
{
    keywords.push_back(Keywords(Keywords::BuiltIn, builtIn));
}

void Language::addContain(Contain *contain)
{
    contains.push_front(contain);
}

void Language::setCaseSensitive(bool s)
{
    caseSensitive = s;
}

bool Language::isCaseSensitive()
{
    return caseSensitive;
}

void Language::setName(const string &name)
{
    this->name = name;
}

void Language::setIllegal(const string &illegal)
{
    this->illegal = illegal;
}

void Language::setLexems(const string &lexems)
{
    this->lexems = lexems;
}

const string& Language::getLexems()
{
    return lexems;
}

const list<Keywords>& Language::getKeywords()
{
    return keywords;
}

int Language::matchKeyword(const string &k)
{
    for(list<Keywords>::iterator it=keywords.begin(); it!=keywords.end(); it++){
        if(it->getKeyword()==k)
            return it->getType();
    }
    return Keywords::NotFound;
}

Contain *Language::findRefContain(const char *name)
{
    for(list<Contain *>::iterator it=contains.begin(); it!=contains.end(); it++){
        if(0==strcmp((*it)->getName(), name))
            return *it;
    }
    return NULL;
}

Contain *Language::findMatchedContain(const string &match)
{
    for(list<Contain *>::reverse_iterator it=contains.rbegin(); it!=contains.rend(); it++){
        Contain *contain = *it;
        if(contain->getBeginRe().isValid() && !contain->isRef()){
            FindResult fr = contain->getBeginRe().exec(match.c_str(), match.length());
            if(fr.isValid())
                return contain;
        }
    }
    return NULL;
}

void Language::printDebugInfo()
{
    printf("-----------------------Debug Info------------------------------\n");
    printf("keywords number: %d\n", keywords.size());
    for(list<Contain *>::iterator it=contains.begin(); it!=contains.end(); it++){
        printf("name: %s\n", (*it)->getName());
    }
}

bool Language::isCompiled()
{
    return compiled;
}

void Language::compileLanguage()
{
    if(compiled)
        return;
    compiled = true;
    if(keywords.size()>0){
        if(!lexems.empty()){
            lexemsRe.compile(lexems, caseSensitive, true);
        } else {
            lexemsRe.compile("[a-zA-Z][a-zA-Z0-9_]*", caseSensitive, true);
        }
    }
    //start compile
    list<string> terminators;
    for(list<Contain *>::iterator it=contains.begin(); it!=contains.end(); it++){
        Contain *contain = *it;
        contain->compile(this);
        if(!contain->isRef())
            terminators.push_front(contain->getBegin());
    }
    if(!illegal.empty()){
        illegalRe.compile(illegal, caseSensitive);
        terminators.push_back(illegal);
    }
    if(!terminators.empty())
        terminatorsRe.compile(HighlighterUtil::joinStrings(terminators, '|'), caseSensitive, true);
}

RegExp& Language::getTerminatorsRe()
{
    return terminatorsRe;
}

RegExp& Language::getLexemsRe()
{
    return lexemsRe;
}

list<Contain *>& Language::getContains()
{
    return contains;
}

Language::~Language()
{
    //free contains
    for(list<Contain *>::iterator it= contains.begin(); it != contains.end(); it++){
        delete *it;
    }
}
//---------------------------- LanguageDefinationXmlParser ---------------------
LanguageDefinationXmlParser::LanguageDefinationXmlParser()
{
}

Language* LanguageDefinationXmlParser::startParse(const char* name, char *src)
{
    Language *lan = new Language;
    lan->setName(name);
    xml_document<> doc;
    doc.parse<0>(src);
    xml_node<> *firstNode = doc.first_node("Language");
    parseLanguageNode(firstNode, lan);
//    lan->printDebugInfo();
//    lan->compileLanguage();
    return lan;
}

void LanguageDefinationXmlParser::parseLanguageNode(xml_node<> *languageNode, Language* lan)
{
    xml_attribute<> *caseSensitive = languageNode->first_attribute("casesensitive");
    if(caseSensitive && 0==strcmp(caseSensitive->value(), "false"))
        lan->setCaseSensitive(false);
    else
        lan->setCaseSensitive(true);//it is true by default
    xml_node<> *node = languageNode->first_node();
    while(node){
        if(0==strcmp(node->name(), "Contains"))
            parseContainsNode(node, lan);
        else if(0==strcmp(node->name(), "Illegal"))
            lan->setIllegal(node->value());
        else if(0==strcmp(node->name(), "Keywords")){
            xml_node<> *kNode = node->first_node();
            while(kNode){
                if(0==strcmp(kNode->name(), "Keyword"))
                    lan->addKeyword(Keywords::Keyword, kNode->value());
                else if(0==strcmp(kNode->name(), "BuiltIn"))
                    lan->addKeyword(Keywords::BuiltIn, kNode->value());
                else if(0==strcmp(kNode->name(), "Literal"))
                    lan->addKeyword(Keywords::Literal, kNode->value());
                else if(0==strcmp(kNode->name(), "Constant"))
                    lan->addKeyword(Keywords::Constant, kNode->value());
                else if (0==strcmp(kNode->name(), "Type"))
                    lan->addKeyword(Keywords::Type, kNode->value());
                else if (0==strcmp(kNode->name(), "Command"))
                    lan->addKeyword(Keywords::Command, kNode->value());
                else if (0==strcmp(kNode->name(), "Property"))
                    lan->addKeyword(Keywords::Property, kNode->value());
                else if (0==strcmp(kNode->name(), "Title"))
                    lan->addKeyword(Keywords::Title, kNode->value());
                kNode = kNode->next_sibling();
            }
        } else if(0==strcmp(node->name(), "Lexems")){
            lan->setLexems(node->value());
        }
        node = node->next_sibling();
    }
}

void LanguageDefinationXmlParser::parseContainsNode(xml_node<> *containsNode, Language *lan)
{
    xml_node<> *node = containsNode->first_node();
    while(node){
        if(0==strcmp(node->name(), "Contain")){
            Contain *contain = new Contain();
            if(parseContainNode(node, contain, lan))
                lan->addContain(contain);
            else
                delete contain;
        }
        node = node->next_sibling();
    }
}

bool LanguageDefinationXmlParser::parseContainNode(xml_node<> *node, Contain *contain, Language *lan)
{
    //Deal Attribute: Name
    xml_attribute<> *nameAttr = node->first_attribute("name");
    if(nameAttr)
        contain->setName(nameAttr->value());
    xml_attribute<> *refLanguageContainAttr = node->first_attribute("refLanguageContains");
    if(refLanguageContainAttr&& 0==strcmp(refLanguageContainAttr->value(), "true"))
        contain->setRefLanguageContains(true);

    //Deal Nodes
    xml_node<> *currentNode = node->first_node();
    while(currentNode){
        if(0==strcmp(currentNode->name(), "Keywords")){
            xml_attribute<> *attr = currentNode->first_attribute("refLanguageKeywords");
            if(attr){
                if(0==strcmp(attr->value(), "true"))
                    contain->setRefLanguageKeywords(true);
                //default is not ref language keyword, so we not deal 'else'
            }
            xml_node<> *kNode = currentNode->first_node();
            while(kNode){
                if(0==strcmp(kNode->name(), "Keyword"))
                    contain->addKeyword(Keywords::Keyword, kNode->value());
                else if(0==strcmp(kNode->name(), "BuiltIn"))
                    contain->addKeyword(Keywords::BuiltIn, kNode->value());
                else if(0==strcmp(kNode->name(), "Literal"))
                    contain->addKeyword(Keywords::Literal, kNode->value());
                else if(0==strcmp(kNode->name(), "Constant"))
                    contain->addKeyword(Keywords::Constant, kNode->value());
                else if (0==strcmp(kNode->name(), "Type"))
                    contain->addKeyword(Keywords::Type, kNode->value());
                else if (0==strcmp(kNode->name(), "Command"))
                    contain->addKeyword(Keywords::Command, kNode->value());
                else if (0==strcmp(kNode->name(), "Property"))
                    contain->addKeyword(Keywords::Property, kNode->value());
                else if (0==strcmp(kNode->name(), "Title"))
                    contain->addKeyword(Keywords::Title, kNode->value());
                kNode = kNode->next_sibling();
            }
        } else if(0==strcmp(currentNode->name(), "RefContains")){
            xml_attribute<> *attr = currentNode->first_attribute("refSelf");
            if(attr && 0==strcmp(attr->value(), "true"))
                contain->addRefContain(contain);//self
            xml_node<> *refNode = currentNode->first_node();
            while(refNode){
                if(0==strcmp(refNode->name(), "RefContain")){
                    xml_attribute<> *isRef = refNode->first_attribute("notSetRef");
                    Contain *c = lan->findRefContain(refNode->value());
                    if(!c){
                        assert(0 && "this should not be happend");
                        return false;
                    }
                    if(!isRef)
                        c->setRef(true);
                    c->setParent(contain);
                    contain->addRefContain(c);
                }
                refNode = refNode->next_sibling();
            }
        } else if(0==strcmp(currentNode->name(), "ShowClassName")){
            if(0==strcmp(currentNode->value(), "false"))
                contain->setShowClassName(false);
            //it is true by default, so we donot deal "else"
        } else if(0==strcmp(currentNode->name(), "Begin")){
            contain->setBegin(currentNode->value());
        } else if(0==strcmp(currentNode->name(), "End")){
            contain->setEnd(currentNode->value());
        } else if(0==strcmp(currentNode->name(), "EndsWithParent")){
            if(0==strcmp(currentNode->value(), "true"))
                contain->setEndWithParent(true);
            else
                contain->setEndWithParent(false);
        } else if(0==strcmp(currentNode->name(), "Relevance")){
            contain->setRelevance(atoi(currentNode->value()));
        } else if(0==strcmp(currentNode->name(), "Illegal")){
            contain->setIllegal(currentNode->value());
        } else if (0==strcmp(currentNode->name(), "BeginWithKeyword")){
            if(0==strcmp(currentNode->value(), "true"))
                contain->setBeginWithKeyword(true);
            //it is false by default
        } else if (0==strcmp(currentNode->name(), "Lexems")){
            contain->setLexems(currentNode->value());
        } else if (0==strcmp(currentNode->name(), "Starts")){
            Contain *c = lan->findRefContain(currentNode->value());
            if(!c){
                assert(0 && "this should not be happend");
                return false;
            }
            c->setRef(true);
            c->setParent(contain);
            contain->setStarts(c);
        } else if (0==strcmp(currentNode->name(), "ExcludeEnd")){
            if(0==strcmp(currentNode->value(), "true"))
                contain->setExcludeEnd(true);
        } else if (0==strcmp(currentNode->name(), "ReturnBegin")){
            if(0==strcmp(currentNode->value(), "true"))
                contain->setReturnBegin(true);
        } else if (0==strcmp(currentNode->name(), "SubLanguage")){
            contain->setSubLanugage(currentNode->value());
        } else if (0==strcmp(currentNode->name(), "ReturnEnd")){
            if(0==strcmp(currentNode->value(), "true"))
                contain->setReturnEnd(true);
        }
        currentNode = currentNode->next_sibling();
    }
    return true;
}
