#include <cstdio>

#include <QApplication>
#include <QDir>
#include <QSplashScreen>
#include <QTime>
#include <QThread>
#include <QFile>
#include <QMap>

#include "version.h"
#include "mdcharmapplication.h"
#include "mdcharmform.h"
#include "configuration.h"
#include "util/spellcheck/spellchecker.h"
#include "codesyntaxhighlighter.h"
#include <BreakpadHandler.h>

void delayForShowSplash(unsigned int msec);
void initHilighter();

int main(int argc, char** argv)
{
#ifdef QT_V5
    qApp->addLibraryPath("/usr/local/lib/Qt-5.0.2/plugins");
    qApp->addLibraryPath("/usr/local/lib/Qt-5.0.2/");
#endif

    MdCharmApplication app(argc, argv);
    if(app.isRunning())
        return 0;
    app.setApplicationName(Utils::AppName);
    app.setApplicationVersion(QLatin1String(VERSION_STR));
    app.setOrganizationName(Utils::AppName);
    app.setOrganizationDomain(QLatin1String("mdcharm.com"));

    Configuration *conf = Configuration::getInstance();
    bool isShowSplash = conf->isShowSplash();
    if(argc>1)
        isShowSplash = false;
    QTime startTime;
    QSplashScreen *splash = NULL;
    if (isShowSplash)
    {
        splash = new QSplashScreen(QPixmap(":/mdcharm-splash.png"));
        splash->show();
        app.processEvents();
        startTime = QTime::currentTime();
    }

    BreakpadQt::GlobalHandler *handler = BreakpadQt::GlobalHandler::instance();
    handler->setDumpPath(QDir::tempPath());
#ifdef Q_OS_LINUX
    handler->setReporter(qApp->applicationDirPath().append("/mdcharm_crashrpt"));
#else
    handler->setReporter(qApp->applicationDirPath().append("/crashrpt"));
#endif
    handler->appendArgument(conf->configFileDirPath().append("/sysinfo.txt"));
    if(conf->isCheckSpell())
        MdCharmGlobal::getInstance();
    QStringList argsList = app.arguments();
    argsList.removeFirst();

    initHilighter();

    MdCharmForm mcf;
    mcf.connect(&app, SIGNAL(openFiles(QStringList)), SLOT(openFilesAndShow(QStringList)));
    if(isShowSplash)
    {
        QTime endTime = QTime::currentTime();

        if((endTime.msec()-startTime.msec())<750)
        {
            delayForShowSplash(750-(endTime.msec()-startTime.msec()));
        }
    }
    QApplication::setOverrideCursor(Qt::BusyCursor);
    if(!conf->isHaveValidApplicationState())
        mcf.showMaximized();
    else{
        mcf.show();
        mcf.restoreMdCharmState();
    }
    mcf.openArgFiles(argsList);
    QApplication::restoreOverrideCursor();
    mcf.checkCodeSyntaxCss();

    if(isShowSplash)
    {
        splash->finish(&mcf);
        delete splash;
    }
    return app.exec();//FIXME crashrpt 2364fed2-bfb0-451b-b171-cc574f249e77 689c4c7f-7d04-49a8-9e81-3d794ecda3cc
}

void delayForShowSplash(unsigned int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while(QTime::currentTime()<dieTime) ;
            QCoreApplication::processEvents();
}

void initHilighter()
{
    LanguageManager *languageManager = LanguageManager::getInstance();
    QMap<QString, QString> xmlMap;
    xmlMap.insert("bash", ":/highlighter/highlighter/bash.xml");
    xmlMap.insert("cpp", ":/highlighter/highlighter/cpp.xml");
    xmlMap.insert("cs", ":/highlighter/highlighter/cs.xml");
    xmlMap.insert("css", ":/highlighter/highlighter/css.xml");
    xmlMap.insert("diff", ":/highlighter/highlighter/diff.xml");
    xmlMap.insert("http", ":/highlighter/highlighter/http.xml");
    xmlMap.insert("ini", ":/highlighter/highlighter/ini.xml");
    xmlMap.insert("java", ":/highlighter/highlighter/java.xml");
    xmlMap.insert("javascript", ":/highlighter/highlighter/javascript.xml");
    xmlMap.insert("json", ":/highlighter/highlighter/json.xml");
    xmlMap.insert("markdown", ":/highlighter/highlighter/markdown.xml");
    xmlMap.insert("perl", ":/highlighter/highlighter/perl.xml");
    xmlMap.insert("php", ":/highlighter/highlighter/php.xml");
    xmlMap.insert("python", ":/highlighter/highlighter/python.xml");
    xmlMap.insert("ruby", ":/highlighter/highlighter/ruby.xml");
    xmlMap.insert("sql", ":/highlighter/highlighter/sql.xml");
    xmlMap.insert("xml", ":/highlighter/highlighter/xml.xml");
    QMapIterator<QString, QString> it(xmlMap);
    while(it.hasNext()){
        it.next();
        QFile file(it.value());
        if(!file.open(QFile::ReadOnly))
            continue;
        languageManager->addLanguage(it.key().toStdString(), file.readAll().data());
        file.close();
    }
}
