#ifndef SESSIONFILEPARSER_H
#define SESSIONFILEPARSER_H

#include <QXmlStreamReader>

#include "editareawidget.h"
#include "rapidxml.hpp"

/*******************************************************************************
 * Session activeView index start from 0
 * MainView activeIndex index start from 0
 *
 ******************************************************************************/
class SessionFileParser
{
public:
    SessionFileParser();
    bool startParse(const QString &sessionXmlFilePath);
    QList<StateModel> getFileStateModelList();
    const QString getProjectDockWidgetDirPath();
private:
    void parseMdCharmElement(rapidxml::xml_node<> *mdcharmNode);
    void parseSessionElement(rapidxml::xml_node<> *sessionNode);
    void parseMainViewElement(rapidxml::xml_node<> *mainViewNode);
    void parseFileElement(rapidxml::xml_node<> *fileNode);
    void parseDockWidgetElement(rapidxml::xml_node<> *dockWidgetNode);
    void parseProjectElement(rapidxml::xml_node<> *projectNode);
private:
    QList<StateModel> sml;
    QXmlStreamReader reader;
    QString projectDockWidgetDirPath;
    int viewCount;
};

#endif // SESSIONFILEPARSER_H
