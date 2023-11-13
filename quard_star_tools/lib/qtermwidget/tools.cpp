#include "tools.h"

#include <QCoreApplication>
#include <QDir>
#include <QtDebug>


Q_LOGGING_CATEGORY(qtermwidgetLogger, "qtermwidget", QtWarningMsg)

/*! Helper function to get possible location of layout files.
By default the KB_LAYOUT_DIR is used (linux/BSD/macports).
But in some cases (apple bundle) there can be more locations).
*/
QString get_kb_layout_dir()
{
    QString rval = QString();
    QString k(QLatin1String(":/lib/qtermwidget/kb-layouts"));
    QDir d(k);
    if (d.exists())
    {
        rval = k.append(QLatin1Char('/'));
        return rval;
    }
    
    return QString();
}

/*! Helper function to add custom location of color schemes.
*/
namespace {
    QStringList custom_color_schemes_dirs;
}
void add_custom_color_scheme_dir(const QString& custom_dir)
{
    if (!custom_color_schemes_dirs.contains(custom_dir))
        custom_color_schemes_dirs << custom_dir;
}

/*! Helper function to get possible locations of color schemes.
By default the COLORSCHEMES_DIR is used (linux/BSD/macports).
But in some cases (apple bundle) there can be more locations).
*/
const QStringList get_color_schemes_dirs()
{
    QStringList rval;
    QString k(QLatin1String(":/lib/qtermwidget/color-schemes"));
    QDir d(k);

    if (d.exists())
        rval << k.append(QLatin1Char('/'));


    for (const QString& custom_dir : std::as_const(custom_color_schemes_dirs))
    {
        d.setPath(custom_dir);
        if (d.exists())
            rval << custom_dir;
    }
#ifdef QT_DEBUG
    if(rval.isEmpty()) {
        qDebug() << "Cannot find color-schemes in any location!";
    }
#endif
    return rval;
}
