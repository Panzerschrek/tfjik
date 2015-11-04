#pragma once
#include <QMainWindow>
#include <QTextEdit>
#include <QBoxLayout>
#include <QToolBar>

#include <QMenu>
#include <QMenuBar>

class MainWindow : public QWidget
{
	Q_OBJECT

public:
	MainWindow();
	~MainWindow();

private slots:
	void OnSave();
	void OnLoad();
	void OnBuild();

	void OnAbout();

private:
	QMenuBar* menu_bar_;
	QMenu* file_menu_;
	QMenu* help_menu_;

	QBoxLayout* layout_;

	QToolBar* tool_bar_;

	QTextEdit* editor_;
	QTextEdit* output_;

	QString file_name_;
};
