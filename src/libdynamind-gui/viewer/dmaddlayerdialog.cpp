#include "dmaddlayerdialog.h"
#include "ui_dmaddlayerdialog.h"
#include <QColorDialog>
#include <QLabel>
#include <QTreeWidgetItem>
#include <QGroupBox>

#include "dmattribute.h"
#include "dmlayer.h"
#include "dmviewer.h"

namespace DM {

AddLayerDialog::AddLayerDialog(DM::System *system, QWidget *parent) :
    QDialog(parent), system(system),
    ui(new Ui::AddLayerDialog) {
    ui->setupUi(this);
    start = Qt::white;
    stop = Qt::black;
    foreach (DM::View v, system->getViews()) {
        QStringList strings;
        strings << QString::fromStdString(v.getName());
        
        switch (v.getType()) {
            case DM::FACE:
                strings << "Face";
                break;
            case DM::EDGE:
                strings << "Edge";
                break;
            case DM::NODE:
                strings << "Node";
                break;
            default:
                continue;
        }
        
        QTreeWidgetItem *item = new QTreeWidgetItem(strings);
        ui->viewList->addTopLevelItem(item);
    }
    
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

AddLayerDialog::~AddLayerDialog() {
    delete ui;
}

GLuint makeColorRampTexture(DM::Viewer *v, QColor start, QColor stop) {
    v->makeCurrent();
    GLuint texture;
    QImage p(100, 10, QImage::Format_RGB32);
    QPainter painter;
    double h_inc = (stop.hueF() - start.hueF()) / 100.0;
    double s_inc = (stop.saturationF() - start.saturationF()) / 100.0;
    double v_inc = (stop.valueF() - start.valueF()) / 100.0;
    
    painter.begin(&p);
    for (int i = 0; i < 100; i++) {
        double h = start.hueF() + (i*h_inc);
        double s = start.saturationF() + (i*s_inc);
        double v = start.valueF() + (i*v_inc);
                
        QColor c = QColor::fromHsvF(h, s, v);
        painter.setPen(c);
        painter.drawRect(i, 0, 1, 10);
    }
    painter.end();
    
    glEnable(GL_TEXTURE_2D);
    texture = v->bindTexture(p, GL_TEXTURE_2D, GL_RGB);
    return texture;
}

DM::Layer *AddLayerDialog::getLayer(DM::Viewer *v) {
    if (!view)
        return 0;
    DM::Layer *l = new DM::Layer(system, *view, attribute);
    if (ui->colorCheckBox->isChecked()) {
        l->setColorInterpretation(makeColorRampTexture(v, start.toHsv(), stop.toHsv()));
    }
    if (ui->heightCheckBox->isChecked()) {
        l->setHeightInterpretation(ui->heightSpinBox->value());
    }
    return l;
}

void AddLayerDialog::on_color_start_button_clicked() {
    start = QColorDialog::getColor(start, this);
    ui->color_start_label->setStyleSheet("background-color: " + start.name());
}

void AddLayerDialog::on_color_stop_button_clicked() {
    stop = QColorDialog::getColor(stop, this);
    ui->color_stop_label->setStyleSheet("background-color: " + stop.name());
}

void AddLayerDialog::on_viewList_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(current);
    ui->attributeList->clear();
    view = system->getViewDefinition(current->text(0).toStdString());
    
    if (!system->getUUIDsOfComponentsInView(*view).size()) return;
    
    QMap<std::string, DM::Attribute*> attributes;
    std::string uuid = system->getUUIDsOfComponentsInView(*view)[0];
    
    if (view->getType() == DM::FACE) {
        attributes = QMap<std::string, DM::Attribute *>(system->getFace(uuid)->getAllAttributes());
    }
    
    if (view->getType() == DM::EDGE) {
        attributes = QMap<std::string, DM::Attribute *>(system->getEdge(uuid)->getAllAttributes());
    }
    
    if (view->getType() == DM::NODE) {
        attributes = QMap<std::string, DM::Attribute *>(system->getNode(uuid)->getAllAttributes());
    }
    
    foreach(std::string key, attributes.keys()) {
        DM::Attribute *attr = attributes[key];
        if (attr->hasString() || attr->hasStringVector()) continue;
        
        QStringList strings;
        strings << QString::fromStdString(key);
        
        if (attr->hasDouble()) 
            strings << "double";
        if (attr->hasDoubleVector())
            strings << "double vector";
        
        QTreeWidgetItem *item = new QTreeWidgetItem(strings);
        ui->attributeList->addTopLevelItem(item);
    }
}

void AddLayerDialog::on_attributeList_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    attribute = current ? current->text(0).toStdString() : "";
    
    ui->interpreteGroup->setEnabled(current);
}

}
