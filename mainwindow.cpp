#include <sstream>

#include <QFileDialog>
#include <QMessageBox>

#include "lexical_analyzer.h"
#include "syntax_analyzer.h"

#include "mainwindow.hpp"

static const char g_files_filter[] = "C++ files (*.cpp *h *.hpp)" ;

MainWindow::MainWindow()
	: QWidget()
{
	setMinimumSize( 512, 384 );

	menu_bar_= new QMenuBar( this );
	setContentsMargins( 0, menu_bar_->height(), 0, 0 );

	file_menu_= new QMenu( "File" );
	file_menu_->addAction( "Open", this, SLOT(OnLoad()) );
	file_menu_->addAction( "Save", this, SLOT(OnSave()) );
	file_menu_->addAction( "Quit", this, SLOT(close()) );
	menu_bar_->addMenu( file_menu_ );

	layout_= new QBoxLayout( QBoxLayout::TopToBottom, this );
	setLayout( layout_ );

	help_menu_= new QMenu( "Help" );
	help_menu_->addAction( "About", this, SLOT(OnAbout()) );
	menu_bar_->addMenu( help_menu_ );

	tool_bar_= new QToolBar("tools", this);
	layout_->addWidget( tool_bar_ );

	tool_bar_->addAction( "save", this, SLOT(OnSave()) );
	tool_bar_->addAction( "load", this, SLOT(OnLoad()) );
	tool_bar_->addAction( "build", this, SLOT(OnBuild()) );

	editor_= new QTextEdit( this );
	layout_->addWidget( editor_ );


	output_= new QTextEdit( this );
	output_->setEnabled( false );
	layout_->addWidget( output_ );
}

MainWindow::~MainWindow()
{
}

void MainWindow::OnSave()
{
	if( file_name_.size() == 0 )
	{
		file_name_ = QFileDialog::getSaveFileName( this, "Save", "", g_files_filter );
		if( file_name_.size() == 0 ) return;
	}

	QFile file( file_name_ );
	if( file.open( QFile::WriteOnly ) )
		file.write( editor_->toPlainText().toLocal8Bit() );
}

void MainWindow::OnLoad()
{
	file_name_ = QFileDialog::getOpenFileName( this, "Load", "", g_files_filter );

	if( file_name_.size() != 0 )
	{
		QFile file( file_name_ );
		if( file.open( QFile::ReadOnly ) )
			editor_->setText( QString(file.readAll()) );
	}
}

void MainWindow::OnBuild()
{
	Lexems lexems;
	Enumeration enumeration;

	const std::string input_text= editor_->toPlainText().toStdString();

	try
	{
		lexems= LexemParser::Parse( input_text );
	}
	catch( LexicalError ex )
	{
		output_->setText( QString::fromStdString(ex.what()) );
		return;
	}

	try
	{
		enumeration= SyntaxAnalyzer::ParseEnumeration( lexems );
	}
	catch( SyntaxError error )
	{
		std::stringstream ss;

		if( error.lexem_ < lexems.size() )
		{
			unsigned int file_pos= lexems[error.lexem_].file_position;
			unsigned line= lexems[error.lexem_].line;

			unsigned int cur_line= 1;
			unsigned int cur_pos= 0;
			for( unsigned int i= 0; i < input_text.size(); i++ )
			{
				if( input_text[i] == '\n' ) cur_line++;
				cur_pos++;

				if( line == cur_line )
				{
					ss<< "Syntax error at line "<< line<< std::endl;
					while( cur_pos < file_pos && cur_pos < input_text.size() )
					{
						ss<< input_text[cur_pos];
						cur_pos++;
					}
					ss<< " <-------HERE"<< std::endl;
					break;
				}
			}
		}
		else
			ss<< "Syntax error at end of file"<< std::endl;

		output_->setText( QString::fromStdString(ss.str()) );
		return;
	}

	output_->setText( "Everything is ok" );
}

void MainWindow::OnAbout()
{
	QMessageBox( QMessageBox::Information, "About", "Kr", QMessageBox::Close ).exec();
}
