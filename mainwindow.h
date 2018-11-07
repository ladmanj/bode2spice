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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qcustomplot.h"

#include <QMainWindow>
#include <QVector>

typedef struct
{
    double freq;
    double real;
    double imag;
} freim;

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QPlainTextEdit;
class QSessionManager;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

    void loadFile(const QString &fileName);
    QString getSpiceModelString(const bool inverse = false);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void open();
    void about();
    void plotData();
    bool exportZ();
    bool exportTf();
    void setDelimiter(const QString cdata);
    void setHeadLen(const int num);
    void replotData();

private:
    void createActions();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    bool maybeSave();
    void setCurrentFile(const QString &fileName);
    void preparePlot();
    bool exportModel(const QString &symTemplate, const QString &subTemplate, const bool inverse = false);

    QPlainTextEdit *textEdit;
    QCustomPlot *plot;
    QString curFile;

    int  headerLen;
    char delimiter;
    bool impedance;

    QVector<freim> input_data;

    QSpinBox *headerSpinBox;
    QLineEdit *delmEdit;
    QPushButton *impButton;

};

#endif
