#ifndef GUIImport_H
#define GUIImport_H

#include <dmcompilersettings.h>
#include <QDialog>
#include <QTreeWidget>
#include <QSignalMapper>

namespace DM {
class Module;
}
namespace Ui {
class GUIImport;
}

class Import;

class DM_HELPER_DLL_EXPORT GUIImport : public QDialog
{
	Q_OBJECT

public:
	explicit GUIImport(DM::Module * m, QWidget *parent = 0);
	~GUIImport();

	void updateTree();
	void updateViewConfig();

private:
	Ui::GUIImport *ui;
	Import * m;
	QSignalMapper* treeCheckMapper;
	QString oldpwd;

public slots:
	void accept();
	void on_pushButton_Filename_clicked();
	void on_lineEdit_Filename_textChanged();
	void on_wfsUpdateButton_clicked();
	void on_viewTree_itemDoubleClicked(QTreeWidgetItem * item, int column);
	void updateTreeChecks(QObject* obj);
	void on_selectAllButton_clicked();
	void on_selectNothingButton_clicked();
};

#endif // GUIImport_H
