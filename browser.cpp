#include "browser.h"
#include "ui_browser.h"

Browser::Browser(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Browser) {
    ui->setupUi(this);
    ui->print->hide();
    ui->web->setContextMenuPolicy(Qt::NoContextMenu);

    //Chemin de l’app
    QDir pathApplicationDir = QDir(QCoreApplication::applicationDirPath()).absolutePath();
#ifdef Q_OS_MAC
    pathApplicationDir.cdUp();
    pathApplicationDir.cdUp();
    pathApplicationDir.cdUp();
#endif
    QFileInfo appPath = QFileInfo(pathApplicationDir.absolutePath());
    if((appPath.absoluteFilePath().endsWith("/blink-Browser-build-64")) || (appPath.absoluteFilePath().endsWith("/blink-Browser-build-32")))
        appPath = QFileInfo(appPath.absoluteFilePath().remove("-build-64").remove("-build-32"));
    if(appPath.absoluteFilePath().endsWith("/blink-Browser-build/release"))
        appPath = QFileInfo(appPath.absoluteFilePath().remove("-build/release"));
    if(appPath.absoluteFilePath().endsWith("/blink-Browser-build/debug"))
        appPath = QFileInfo(appPath.absoluteFilePath().remove("-build/debug"));
    if(appPath.absoluteFilePath().endsWith("/blink-Browser-build"))
        appPath = QFileInfo(appPath.absoluteFilePath().remove("-build"));
    QString GlobalappPath = appPath.absoluteFilePath() + "/";

    qDebug("Chemin de l’application %s", qPrintable(GlobalappPath));
    QFileInfoList settingsFileList = QDir(GlobalappPath).entryInfoList(QStringList() << "*.ini", QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    WebUrlRequestInterceptor *wuri = new WebUrlRequestInterceptor();
    home = "https://www.buzzinglight.com";
    ticketWidth = 0;
    doPrint = false;
    if(settingsFileList.count()) {
        QSettings settings(settingsFileList.first().absoluteFilePath(), QSettings::IniFormat);
        qDebug("Chemin du fichier ini : %s", qPrintable(settingsFileList.first().absoluteFilePath()));
        settings.setIniCodec(QTextCodec::codecForName("UTF-8"));

        settings.beginGroup("config");
        home = settings.value("home", "https://www.buzzinglight.com").toString().trimmed();
        quint8 buttonBar = settings.value("buttonbar", 1).toInt();
        eventEater = new EventEater(settings.value("timeout", -1).toInt());
        ui->web->installEventFilter(eventEater);
        if(eventEater->timeout > 0)
            startTimer(1000);
        settings.endGroup();

        //Tickets
        {
            settings.beginGroup("ticket");
            ticketWidth = settings.value("width", 0).toInt();
            if(ticketWidth)
                ui->print->show();
            settings.endGroup();
        }


        //Site autorisés
        {
            int size = settings.beginReadArray("allow");
            for (int i = 0; i < size; ++i) {
                settings.setArrayIndex(i);
                QString url = settings.value("url").toString().trimmed();
                if(!url.isEmpty()) {
                    qDebug("Domaine %s autorisé", qPrintable(url));
                    wuri->allow << url;
                }
            }
            settings.endArray();
        }

        //Site interdits
        {
            int size = settings.beginReadArray("deny");
            for (int i = 0; i < size; ++i) {
                settings.setArrayIndex(i);
                QString url = settings.value("url").toString().trimmed();
                if(!url.isEmpty()) {
                    qDebug("Domaine %s bloqué", qPrintable(url));
                    wuri->deny << url;
                }
            }
            settings.endArray();
        }

        //Barre
        if(buttonBar == 0) {
            ui->layout->removeItem(ui->toolbar);
            ui->print->hide();
            ui->back->hide();
            ui->home->hide();
            ui->url->hide();
            ui->widget->layout()->setMargin(0);
        }

    }

    //Moteur Web
    QWebEngineProfile::defaultProfile()->setRequestInterceptor(wuri);
    QWebEngineProfile::defaultProfile()->clearHttpCache();
    connect(ui->web, SIGNAL(urlChanged(QUrl)), SLOT(urlChanged(QUrl)));
    connect(ui->web, SIGNAL(loadStarted()), SLOT(loadStarted()));
    connect(ui->web, SIGNAL(loadProgress(int)), SLOT(loadProgress(int)));
    connect(ui->web, SIGNAL(loadFinished(bool)), SLOT(loadFinished(bool)));
    setUrl();


    //HTTP et UDP
    MainWindowInterface::main = this;
    http = new InterfaceHttp(this);
    udp  = new Udp(this);
    connect((InterfaceHttp*)http, SIGNAL(outgoingMessage(QString)), SLOT(incomingMessage(QString)));
    connect((Udp*)udp           , SIGNAL(outgoingMessage(QString)), SLOT(incomingMessage(QString)));
    connect((InterfaceHttp*)http, SIGNAL(outgoingData(QString,quint16,QString,QList<QVariant>)), SLOT(incomingData(QString,quint16,QString,QList<QVariant>)));
    connect((Udp*)udp           , SIGNAL(outgoingData(QString,quint16,QString,QList<QVariant>)), SLOT(incomingData(QString,quint16,QString,QList<QVariant>)));


    if(ticketWidth)
        ui->web->setMaximumSize(ticketWidth, ui->web->maximumSize().height());
    else
        showFullScreen();
}

Browser::~Browser() {
    delete ui;
}

void Browser::setUrl(QString url) {
    if(url.isEmpty())
        url = home;
    ui->web->load(url);
    //ui->web->page()->triggerAction(QWebEnginePage::ReloadAndBypassCache);
    eventEater->lastAction = QDateTime::currentDateTime();
}

void Browser::action() {
    if(sender() == ui->home)
        setUrl();
    else if(sender() == ui->back)
        ui->web->back();
    else if(sender() == ui->print)
        print(0);
}

void Browser::urlChanged() {
    urlChanged(ui->web->url());
}
void Browser::urlChanged(const QUrl &url) {
    ui->url->setText(url.toString());
}
void Browser::loadStarted() {
    ui->url->setText(tr("Chargement de la page"));
}
void Browser::loadProgress(int percent) {
    if(percent < 100)
        ui->url->setText(tr("Chargement de la page (%1%)").arg(percent));
    else
        urlChanged();
}
void Browser::loadFinished(bool termine) {
    if(termine) {
        qDebug("Fin du chargement de la page");
        urlChanged();
        if(doPrint) {
            doPrint = false;
            qDebug("Lancement de l'impression");
            print(0);
        }
    }
    else
        ui->url->setText(tr("Erreur de chargement de la page"));
}

void Browser::incomingMessage(const QString &log) {
    qDebug("%s", qPrintable(log));
}
void Browser::incomingData(const QString &, quint16, const QString &, const QList<QVariant> &valeurs) {
    if((valeurs.count()) && (valeurs.at(2) == "print")) {
        //http://192.168.0.101:15556/?v1=print&v2=192.168.0.101/update/Tablets/SpecialBuzzingLight/printer/stories/&v3=test
        doPrint = true;
        QString url = QString("http://%1?story=%2").arg(valeurs.at(3).toString()).arg(valeurs.at(4).toString());
        qDebug("URL = %s", qPrintable(url));
        setUrl(url);
    }
}


void Browser::timerEvent(QTimerEvent *) {
    if(eventEater->lastAction.secsTo(QDateTime::currentDateTime()) > eventEater->timeout) {
        if(ui->web->url().toString() != home) {
            qDebug("%s != %s", qPrintable(ui->web->url().toString()), qPrintable(home));
            setUrl();
        }
        else
            qDebug("%s == %s", qPrintable(ui->web->url().toString()), qPrintable(home));
        eventEater->lastAction = QDateTime::currentDateTime();
    }
}

void Browser::print(qint8 action) {
    if(action == 0) {
        ui->web->setGeometry(QRect(ui->web->geometry().topLeft(), QSize(ticketWidth, ui->web->page()->contentsSize().height())));
        qDebug("%d × %d", ui->web->geometry().width(), ui->web->geometry().height());
        QTimer::singleShot(100, this, SLOT(print()));
    }
    else {
        QPixmap pixmapToPrint = ui->web->grab().scaled(QSize(ticketWidth, ticketWidth*100), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        pixmapToPrint.save(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first() + "/test.png");
        bool hasPrinted = false;
        qreal paperWidth = 72;//72 pour tickets 180
        foreach(const QPrinterInfo &printerInfo, QPrinterInfo::availablePrinters()) {
            if((!hasPrinted) && (printerInfo.printerName().toLower().contains("tm_"))) {
                QPrinter printer(printerInfo, QPrinter::PrinterResolution);
                if(printer.isValid()) {
                    //printer.setDocName(ticketFilename.fileName());
                    printer.setPrintRange(QPrinter::CurrentPage);
                    printer.setOrientation(QPrinter::Portrait);
                    printer.setPaperSize(QSize(paperWidth, paperWidth / (qreal)pixmapToPrint.width() * (qreal)pixmapToPrint.height()), QPrinter::Millimeter);

                    printer.setCopyCount(1);
                    printer.setCreator("Buzzing Light");
                    printer.setColorMode(QPrinter::GrayScale);
                    printer.setFullPage(true);
                    //printer.setResolution(600);

                    QPainter printerPainter;
                    if(printerPainter.begin(&printer)) {
                        printerPainter.drawPixmap(printer.pageRect(), pixmapToPrint);
                        printerPainter.end();
                        hasPrinted = true;
                    }
                }
            }
        }
    }
}





WebUrlRequestInterceptor::WebUrlRequestInterceptor(QObject *p)
    :QWebEngineUrlRequestInterceptor(p) {
}
void WebUrlRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info) {
    QString rsrct = "";
    switch(info.resourceType()){
    case 0:rsrct="ResourceTypeMainFrame = 0, // top level page";break;
    case 1:rsrct="ResourceTypeSubFrame, // frame or iframe";break;
    case 2:rsrct="ResourceTypeStylesheet, // a CSS stylesheet";break;
    case 3:rsrct="ResourceTypeScript, // an external script";break;
    case 4:rsrct="ResourceTypeImage, // an image (jpg/gif/png/etc)";break;
    case 5:rsrct="ResourceTypeFontResource, // a font";break;
    case 6:rsrct="ResourceTypeSubResource, // an other subresource.";break;
    case 7:rsrct="ResourceTypeObject, // an object (or embed) tag for a plugin,";break;
    case 8:rsrct="ResourceTypeMedia, // a media resource.";break;
    case 9:rsrct="ResourceTypeWorker, // the main resource of a dedicated worker.";break;
    case 10:rsrct="ResourceTypeSharedWorker, // the main resource of a shared worker.";break;
    case 11:rsrct="ResourceTypePrefetch, // an explicitly requested prefetch";break;
    case 12:rsrct="ResourceTypeFavicon, // a favicon";break;
    case 13:rsrct="ResourceTypeXhr, // a XMLHttpRequest";break;
    case 14:rsrct="ResourceTypePing, // a ping request for <a ping>";break;
    case 15:rsrct="ResourceTypeServiceWorker, // the main resource of a service worker.";break;
    case 16:rsrct="ResourceTypeUnknown";break;

    default : rsrct="Unknown type";break;
    }

    bool block = allow.count();
    foreach(const QString &url, deny)
        if(info.requestUrl().host().contains(url))
            block = true;
    foreach(const QString &url, allow)
        if(info.requestUrl().host().contains(url))
            block = false;
    if(block) {
        info.block(true);
        if(info.resourceType() == 0)
            MainWindowInterface::main->setUrl();
    }
}



EventEater::EventEater(qint32 _timeout) {
    timeout = _timeout;
    qDebug("Timeout %d", timeout);
}
bool EventEater::eventFilter(QObject *obj, QEvent *event) {
    bool resetTimeout = false;
    if (event->type() == QEvent::KeyPress)
        resetTimeout = true;
    else if (event->type() == QEvent::MouseButtonPress)
        resetTimeout = true;
    else if (event->type() == QEvent::MouseMove)
        resetTimeout = true;
    else if (event->type() == QEvent::Wheel)
        resetTimeout = true;
    else if (event->type() == QEvent::TouchUpdate)
        resetTimeout = true;
    if(resetTimeout) {
        //qDebug("-> %d (%d)", event->type(), resetTimeout);
        lastAction = QDateTime::currentDateTime();
    }
    return QObject::eventFilter(obj, event);
}
