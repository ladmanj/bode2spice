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

#include "mainwindow.h"
#include <QByteArray>
#include <QSizePolicy>
#include <QTextCodec>
#include <QtWidgets>

MainWindow::MainWindow()
    : plot(new QCustomPlot)
{
    setCentralWidget(plot);

    readSettings();

    createActions();
    createStatusBar();

    preparePlot();

    setCurrentFile(QString());
    setUnifiedTitleAndToolBarOnMac(true);
}

void MainWindow::preparePlot()
{
    plot->addGraph(plot->xAxis, plot->yAxis);
    plot->addGraph(plot->xAxis, plot->yAxis2);

    plot->xAxis->setLabel(tr("Frequency [Hz]"));
    plot->yAxis->setLabel(tr("Magnitude [dB]"));
    plot->yAxis2->setLabel(tr("Phase [°]"));
    plot->xAxis->setRange(10, 100e6);
    plot->yAxis->setRange(-50, +50);
    plot->yAxis2->setRange(-200, +200);
    plot->yAxis2->setVisible(true);
    plot->xAxis->setScaleType(QCPAxis::stLogarithmic);
    //    plot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    plot->yAxis->setScaleType(QCPAxis::stLinear);
    plot->yAxis2->setScaleType(QCPAxis::stLinear);
    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    plot->xAxis->setTicker(logTicker);
    plot->xAxis->setNumberFormat("eb");
    plot->xAxis->setNumberPrecision(0);
    plot->replot();
}

void MainWindow::closeEvent(QCloseEvent *event)

{
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::open()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}

QString MainWindow::getSpiceModelString(const bool inverse)
{
    QString spiceModelString;
    double mag, pha, re, im;
    for (int i = 0; i < input_data.size(); i++) {
        re = input_data[i].real;
        im = input_data[i].imag;

        mag = (sqrt(re * re + im * im));

        if (qIsNull(mag)) {
            //mark both positive and negative infinity as skipped value
            spiceModelString.append(QString("* skipped "));
        }

        mag = (inverse ? 1.0 / mag : mag);
        pha = (inverse ? -1.0 : 1.0) * qRadiansToDegrees(qAtan2(im, re));

        mag = 20 * log10(mag);

        spiceModelString.append(QString("+ (%1, %2, %3)")
                                    .arg(input_data[i].freq, 0, 'f', 3)
                                    .arg(mag, 0, 'f', 3)
                                    .arg(pha, 0, 'f', 3));

        if (i < (input_data.size() - 1))
            spiceModelString.append(QString("\n"));
    }
    return spiceModelString;
}

bool MainWindow::exportZ()
{
    return exportModel(":/templates/Z.asy", ":/templates/Z.sub", true);
}

bool MainWindow::exportTf()
{
    return exportModel(":/templates/Tf.asy", ":/templates/Tf.sub");
}

bool MainWindow::exportModel(const QString &symTemplate,
                             const QString &subTemplate,
                             const bool inverse)
{
    if (input_data.isEmpty()) {
        statusBar()->showMessage(tr("No data"), 2000);
        return false;
    }
    QTextCodec *codec = QTextCodec::codecForName("Windows-1252");
    QByteArray fileData;
    QFile file(symTemplate);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"), tr("Cannot read template."));
        return false;
    }

    fileData = file.readAll(); // read all the data into the byte array
    file.close();              // close the file handle

    QString text(codec->toUnicode(fileData)); // add to text string for easy string replace

    QString caption;
    caption.append("Export ");
    caption.append(inverse ? "Impedance " : "Transfer Function ");
    caption.append("model as");
    QFileDialog dialog(this, caption);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter("*.asy *.sub");
    if (dialog.exec() != QDialog::Accepted)
        return false;

    QString filename = dialog.selectedFiles().constFirst();

    QString path = QFileInfo(filename).path();
    QString base = QFileInfo(filename).baseName();

    file.setFileName(path + "/" + base + ".asy");

    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this,
                             tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                                 .arg(QDir::toNativeSeparators(file.fileName()),
                                      file.errorString()));
        return false;
    }

    text.replace(QString("MODELNAME"), base); // replace text in string

    file.write(codec->fromUnicode(text));
    file.close(); // close the file handle

    file.setFileName(subTemplate);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"), tr("Cannot read template."));
        return false;
    }

    fileData = file.readAll(); // read all the data into the byte array
    file.close();              // close the file handle

    text = codec->toUnicode(fileData);

    file.setFileName(path + "/" + base + ".sub");

    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this,
                             tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                                 .arg(QDir::toNativeSeparators(file.fileName()),
                                      file.errorString()));
        return false;
    }

    text.replace(QString("MODELNAME"), base);                        // replace text in string
    text.replace(QString("MODELDEF"), getSpiceModelString(inverse)); //insert bode data

    file.write(codec->fromUnicode(text));
    file.close(); // close the file handle

    statusBar()->showMessage(tr("Model exported"), 2000);
    return true;
}

void MainWindow::about()
{
    QMessageBox::about(this,
                       tr("About Application"),
                       tr("The <b>bode2spice</b> tool reads data "
                          "from Comma Separated Values file, exported by the Omicron Labs "
                          "Bode Analyzer Suite or similar source containing complex impedance "
                          "or transfer function, plots it for inspection and generates "
                          "LTSpice compatible component to ease embedding into simulation."));
}

void MainWindow::setDelimiter(const QString cdata)
{
    QByteArray ba = cdata.toLocal8Bit();
    if (*ba.data() != 0) {
        delimiter = *ba.data();
    } else {
        delmEdit->setText(QString(delimiter));
    }
}

void MainWindow::setHeadLen(const int num)
{
    headerLen = num;
}

void MainWindow::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QToolBar *fileToolBar = addToolBar(tr("File"));

    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
    QAction *openAct = new QAction(openIcon, tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::open);
    fileMenu->addAction(openAct);
    fileToolBar->addAction(openAct);

    fileMenu->addSeparator();

    const QIcon exportZIcon = /*QIcon::fromTheme("document-export", */ QIcon(
        ":/images/exportZ.png" /*)*/);
    QAction *exportZAct = new QAction(exportZIcon, tr("Export &Impedance"), this);
    QKeySequence exportZkey = QKeySequence(tr("Ctrl+I"));
    exportZAct->setShortcut(exportZkey);
    exportZAct->setStatusTip(tr("Export Impedance model"));
    connect(exportZAct, &QAction::triggered, this, &MainWindow::exportZ);
    fileMenu->addAction(exportZAct);
    fileToolBar->addAction(exportZAct);

    const QIcon exportTfIcon = /*QIcon::fromTheme("document-export",*/ QIcon(
        ":/images/exportTf.png" /*)*/);
    QAction *exportTfAct = new QAction(exportTfIcon, tr("Export &Transfer Function"), this);
    QKeySequence exportTfkey = QKeySequence(tr("Ctrl+T"));
    exportTfAct->setShortcut(exportTfkey);
    exportTfAct->setStatusTip(tr("Export Transfer Function model"));
    connect(exportTfAct, &QAction::triggered, this, &MainWindow::exportTf);
    fileMenu->addAction(exportTfAct);
    fileToolBar->addAction(exportTfAct);

    QToolBar *importToolBar = addToolBar(tr("Import parameters"));
    QLabel *headLenLabel = new QLabel(this);
    headLenLabel->setText(tr("CSV header lines: "));
    importToolBar->addWidget(headLenLabel);

    headerSpinBox = new QSpinBox(this);
    headerSpinBox->setMinimum(1);
    headerSpinBox->setMaximum(256);
    headerSpinBox->setValue(headerLen);
    importToolBar->addWidget(headerSpinBox);
    importToolBar->addSeparator();

    QLabel *csvSepLabel = new QLabel(this);
    csvSepLabel->setText(tr("CSV data separator: "));
    importToolBar->addWidget(csvSepLabel);

    delmEdit = new QLineEdit(this);
    delmEdit->setText(QString(delimiter));
    delmEdit->setMaxLength(1);
    delmEdit->setMaximumWidth(20);
    importToolBar->addWidget(delmEdit);

    QToolBar *plotToolBar = addToolBar(tr("Import parameters"));
    impButton = new QPushButton(this);
    impButton->setCheckable(true);
    impButton->setChecked(impedance);
    impButton->setText(!impedance ? tr("switch plot to Z ") : tr("switch plot to Tf"));
    //plotToolBar->addSeparator();
    plotToolBar->addWidget(impButton);

    connect(delmEdit, SIGNAL(textChanged(QString)), this, SLOT(setDelimiter(QString)));

    connect(headerSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setHeadLen(int)));

    connect(impButton, SIGNAL(clicked()), this, SLOT(replotData()));

    fileMenu->addSeparator();

    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    QAction *exitAct = fileMenu->addAction(exitIcon, tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    aboutAct->setStatusTip(tr("Show the application's About box"));

    QAction *aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::readSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() * 3 / 8, availableGeometry.height() / 3);
        move((availableGeometry.width() - width()) / 2, (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }

    headerLen = settings.value("header ength", QVariant(static_cast<int>(1))).toInt();

    QByteArray ba;
    ba = settings.value("csv delimiter", QVariant(static_cast<const char *>(";")))
             .toString()
             .toLocal8Bit();
    if (*ba.data() != 0) {
        delimiter = *ba.data();
    } else {
        delimiter = ';';
    }

    impedance = settings.value("plot type", QVariant(false)).toBool();
}

void MainWindow::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
    settings.setValue("header ength", headerLen);
    settings.setValue("csv delimiter", QVariant(&delimiter));
    settings.setValue("plot type", impedance);
}

bool MainWindow::maybeSave()
{
    return true;
}

void MainWindow::replotData()
{
    impedance = impButton->isChecked();
    impButton->setText(!impedance ? tr("switch plot to Z ") : tr("switch plot to Tf"));
    plotData();
}

void MainWindow::plotData()
{
    double re, im, mag, phase;

    if (input_data.isEmpty())
        return;

    QVector<QCPGraphData> dataMag, dataPha;
    double minmag = 0;
    double maxmag = 0;
    double minpha = 0;
    double maxpha = 0;

    for (int i = 0; i < input_data.size(); i++) {
        re = input_data[i].real;
        im = input_data[i].imag;

        mag = sqrt(re * re + im * im);

        if (!impedance && qIsNull(mag))
            continue; //skip invalid data

        phase = qRadiansToDegrees(qAtan2(im, re));

        mag = impedance ? mag : 20 * log10(mag);

        dataMag.append(QCPGraphData(input_data[i].freq, mag));
        dataPha.append(QCPGraphData(input_data[i].freq, phase));

        if (!i)
            minmag = mag;
        if (!i)
            minpha = phase;
        if (!i)
            maxmag = mag;
        if (!i)
            maxpha = phase;

        if (mag < minmag)
            minmag = mag;
        if (phase < minpha)
            minpha = phase;
        if (mag > maxmag)
            maxmag = mag;
        if (phase > maxpha)
            maxpha = phase;
    }

    plot->graph(0)->data()->set(dataMag);
    plot->graph(0)->setName(impedance ? tr("impedance") : tr("magnitude"));
    plot->graph(1)->data()->set(dataPha);
    plot->graph(1)->setName(tr("phase"));
    plot->graph(1)->setPen(QPen(Qt::red));

    plot->xAxis->setRange(dataMag.first().key, dataMag.last().key);
    plot->yAxis->setLabel(impedance ? tr("Impedance [Ω]") : tr("Magnitude [dB]"));
    plot->yAxis->setScaleType(impedance ? QCPAxis::stLogarithmic : QCPAxis::stLinear);

    if (impedance) {
        plot->yAxis->setRange(minmag / 10, maxmag * 10);
        QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
        plot->yAxis->setTicker(logTicker);
        plot->yAxis->setNumberFormat("eb");
        plot->yAxis->setNumberPrecision(0);
    } else {
        plot->yAxis->setRange(minmag - 10, maxmag + 10);
        QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);
        fixedTicker->setTickStep(10.0);
        plot->yAxis->setTicker(fixedTicker);
        plot->yAxis->setNumberFormat("f");
        plot->yAxis->setNumberPrecision(0);
    }

    plot->yAxis2->setRange(minpha - 20, maxpha + 20);
    plot->legend->setVisible(true);
    plot->axisRect()->insetLayout()->setInsetAlignment(
        0, Qt::AlignLeft | Qt::AlignTop); // make legend align in top left corner or axis rect
    plot->replot();
    statusBar()->showMessage(tr("Plot updated"), 2000);
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this,
                             tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                                 .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    input_data.clear();

    int head = headerLen;
    while (!file.atEnd()) {
        if (head) {
            file.readLine();
            head--;
            continue;
        }

        QList<QByteArray> list = file.readLine().split(delimiter);
        freim data;

        if (list.size() > 2) {
            bool success;
            data.freq = list[0].toDouble(&success);
            if (!success)
                continue; //skip until next row
            data.real = list[1].toDouble(&success);
            if (!success)
                continue;
            data.imag = list[2].toDouble(&success);
            if (!success)
                continue;

            input_data.append(data);
        } else {
            qDebug("too few columns\n");
        }
    }
    if (input_data.isEmpty()) {
        statusBar()->showMessage(tr("Data not found"), 2000);
        return;
    }
    plotData();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    setWindowModified(false);

    QString shownName = curFile;
    if (curFile.isEmpty())
        shownName = tr("untitled.txt");
    setWindowFilePath(shownName);
}
