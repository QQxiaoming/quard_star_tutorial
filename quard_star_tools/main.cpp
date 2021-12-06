#include "boardwindow.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QTranslator>

QString VERSION = APP_VERSION;
QString GIT_TAG =
#include <git_tag.inc>
;

class CommandLineParser
{
private:
    CommandLineParser(void){
        static CommandLineParser::GC gc;
        parser.setApplicationDescription("Quard Star Board GUI Tools, The following parameters can be configured to start the application:");
        parser.addHelpOption();
        parser.addVersionOption();
        foreach(QString opt,commandLineMap.keys()) {
            parser.addOption(commandLineMap.value(opt,QCommandLineOption("defaultValue")));
        }
    }
    static CommandLineParser* self;
    QCommandLineParser parser;
    bool processApp = false;
    QMap<QString, QCommandLineOption> commandLineMap = {
        {"env_path", QCommandLineOption({"e","env-path"}, "quard star qemu env path","env-path","../../../../output")},
        {"skin_color", QCommandLineOption({"c","skin-color"}, "GUI skin color","skin-color","green")},
    };

public:
    void process(const QCoreApplication &app) {
        parser.process(app);
        processApp = true;
    }
    QString getOpt(QString optKey) const {
        if(processApp) {
            foreach(QString opt,commandLineMap.keys()) {
                if(opt == optKey){
                    QCommandLineOption dstOpt = commandLineMap.value(opt,QCommandLineOption("defaultValue"));
                    if(parser.isSet(dstOpt)) {
                        return parser.value(dstOpt);
                    } else {
                        return dstOpt.defaultValues().at(0);
                    }
                }
            }
        }
        return "";
    }
    bool isSetOpt(QString optKey) const {
        if(processApp) {
            foreach(QString opt,commandLineMap.keys()) {
                if(opt == optKey){
                    QCommandLineOption dstOpt = commandLineMap.value(opt,QCommandLineOption("defaultValue"));
                    return parser.isSet(dstOpt);
                }
            }
        }
        return false;
    }
    static CommandLineParser *instance() {
        if(!self) {
            self = new CommandLineParser();
        }
        return self;
    }

private:
    class GC
    {
    public:
        ~GC() {
            if (self != nullptr) {
                delete self;
                self = nullptr;
            }
        }
    };
};

CommandLineParser* CommandLineParser::self = nullptr;
#define  AppComLineParser   CommandLineParser::instance()

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
    QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
    QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    QApplication application(argc, argv);

    QApplication::setApplicationName("Quard Star Board");
    QApplication::setOrganizationName("Copyright (c) 2021 Quard(QiaoQiming)");
    QApplication::setOrganizationDomain("https://github.com/QQxiaoming/quard_star_tutorial");
    QApplication::setApplicationVersion(VERSION+" "+GIT_TAG);

    AppComLineParser->process(application);
    QString env_path = AppComLineParser->getOpt("env_path");
    QString skin_color = AppComLineParser->getOpt("skin_color");

    BoardWindow window(env_path,skin_color);
    window.show();
    return application.exec();
}
