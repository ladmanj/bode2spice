/****************************************************************************

    Copyright 2018 Jakub Ladman

    This file is part of bode2spice.

    bode2spice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    bode2spice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with bode2spice.  If not, see <https://www.gnu.org/licenses/>.

****************************************************************************/

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(bode2spice);

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Jakub Ladman");
    QCoreApplication::setApplicationName("bode2spice");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file to open.");
    parser.process(app);

    MainWindow mainWin;
    if (!parser.positionalArguments().isEmpty())
        mainWin.loadFile(parser.positionalArguments().first());
    mainWin.show();
    app.setWindowIcon(QIcon(":/images/bode2spice.svg"));
    return app.exec();
}
