#include "sessionfileparser.h"
#include "utils.h"

#include <QTextStream>
#include <QFile>

using namespace rapidxml;

SessionFileParser::SessionFileParser()
{
    viewCount = 0;
}

QList<StateModel> SessionFileParser::getFileStateModelList()
{
    return sml;
}

const QString SessionFileParser::getProjectDockWidgetDirPath()
{
    return projectDockWidgetDirPath;
}

bool SessionFileParser::startParse(const QString &sessionXmlFilePath)
{
    QFile file(sessionXmlFilePath);
    if(!file.open(QFile::ReadOnly))
        return false;

    QByteArray fileContent = file.readAll();
    xml_document<> doc;
    doc.parse<0>(fileContent.data());//TODO: Why file.readAll().data() not work
    xml_node<> *node = doc.first_node();
    while(node){
        if(0==strcmp(node->name(), Utils::AppNameCStr))
            parseMdCharmElement(node);
        node = node->next_sibling();
    }
    file.close();
    return true;
}

void SessionFileParser::parseMdCharmElement(xml_node<> *mdcharmNode)
{
    xml_node<> *node = mdcharmNode->first_node();
    while(node){
        if(0==strcmp(node->name(), "Session"))
            parseSessionElement(node);
        node = node->next_sibling();
    }
}

void SessionFileParser::parseSessionElement(xml_node<> *sessionNode)
{
    xml_node<> *node = sessionNode->first_node();
    while(node){
        if(0==strcmp(node->name(), "MainView")){
            parseMainViewElement(node);
            viewCount++;
        } else if (0==strcmp(node->name(), "DockWidget"))
            parseDockWidgetElement(node);
        node = node->next_sibling();
    }
}

void SessionFileParser::parseMainViewElement(xml_node<> *mainViewNode)
{
    xml_node<> *node = mainViewNode->first_node();
    while(node){
        if(0==strcmp(node->name(), "File"))
            parseFileElement(node);
        node = node->next_sibling();
    }
}

void SessionFileParser::parseDockWidgetElement(xml_node<> *dockWidgetNode)
{
    xml_node<> *node = dockWidgetNode->first_node();
    while(node){
        if(0==strcmp(node->name(), "Project"))
            parseProjectElement(node);
        node = node->next_sibling();
    }
}

void SessionFileParser::parseFileElement(xml_node<> *fileNode)
{
    StateModel sm;
    xml_attribute<> *attr = fileNode->first_attribute();
    while(attr){
        if(0==strcmp(attr->name(), "isCurrentTab"))
            if(0==strcmp(attr->value(), "1"))
                sm.setIsCurrentTab(true);
            else
                sm.setIsCurrentTab(false);
        else if(0==strcmp(attr->name(), "firstVisibleLine"))
            sm.setFirstVisibleLine(atoi(attr->value()));
        else if(0==strcmp(attr->name(), "selectionStart"))
            sm.setSelectionStart(atoi(attr->value()));
        else if(0==strcmp(attr->name(), "selectionEnd"))
            sm.setSelectionEnd(atoi(attr->value()));
        else if(0==strcmp(attr->name(), "verticalScrollBarCurrentValue"))
            sm.setVerticalScrllBarCurrentValue(atoi(attr->value()));
        else if(0==strcmp(attr->name(), "verticalScrollBarMaxValue"))
            sm.setVerticalScrollBarMaxValue(atoi(attr->value()));
        else if(0==strcmp(attr->name(), "filename"))
            sm.setFileFullPath(attr->value());
        attr = attr->next_attribute();
    }
    sm.setViewNum(viewCount);
    sml.append(sm);
}

void SessionFileParser::parseProjectElement(xml_node<> *projectNode)
{
    xml_attribute<> *attr = projectNode->first_attribute();
    while(attr){
        if(0==strcmp(attr->name(), "dirPath"))
            projectDockWidgetDirPath = QString::fromUtf8(attr->value());
        attr = attr->next_attribute();
    }
}
