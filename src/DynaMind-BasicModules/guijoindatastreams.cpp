#include "guijoindatastreams.h"
#include "ui_guijoindatastreams.h"
#include <QInputDialog>


GUIJoinDatastreams::GUIJoinDatastreams(DM::Module *m, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GUIJoinDatastreams)
{
    ui->setupUi(this);
    this->m = (JoinDatastreams *) m;

    ui->listWidget->clear();
    std::vector<std::string> systemlist = m->getParameter<std::vector<std::string> >("Inports");
    foreach (std::string in, systemlist)
        ui->listWidget->addItem(QString::fromStdString(in));


    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(addSystem()));
}

GUIJoinDatastreams::~GUIJoinDatastreams()
{
    delete ui;
}

void GUIJoinDatastreams::addSystem()
{    bool ok;
     QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                          tr("Name"), QLineEdit::Normal,
                                          "", &ok);
      if (ok && !text.isEmpty()) {
          this->m->addSystem(text.toStdString());
          ui->listWidget->addItem(text);
      }


}
