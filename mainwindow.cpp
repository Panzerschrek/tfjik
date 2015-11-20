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
	Enumerations enumerations;

	const std::string input_text= editor_->toPlainText().toStdString();

	try
	{
		lexems= LexemParser::Parse( input_text );
	}
	catch( LexicalError ex )
	{
		unsigned int last_line_begin_pos= 0;
		unsigned int line= 1;
		for( unsigned int i= 0; i < ex.file_pos_ - 1 && i < input_text.size(); i++ )
			if( input_text[i] == '\n' )
			{
				line++;
				last_line_begin_pos= i + 1;
			}

		std::stringstream ss;
		ss<< "Lexical error in line " << line<< std::endl;

		for( unsigned int i= last_line_begin_pos; i < input_text.size() && i < ex.file_pos_ + 1; i++ )
		{
			if( input_text[i] == '\n' ) break;
			ss<< input_text[i];
		}

		ss<< " <-------HERE"<< std::endl;

		output_->setText( QString::fromStdString(ss.str()) );
		return;
	}

	try
	{
		enumerations= SyntaxAnalyzer::ParseEnumerations( lexems );
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

				if( line == cur_line )
				{
					ss<< "Syntax error at line "<< line<< std::endl;
					while( cur_pos <= file_pos && cur_pos < input_text.size() )
					{
						ss<< input_text[cur_pos];
						cur_pos++;
					}
					ss<< " <-------HERE"<< std::endl;
					break;
				}
				cur_pos++;
			}
		}
		else
			ss<< "Syntax error at end of file"<< std::endl;

		output_->setText( QString::fromStdString(ss.str()) );
		return;
	}

	std::stringstream ss;

	for (const Enumeration& enumeration : enumerations)
	{
		ss<< "enum " << (enumeration.is_scopped ? "scopped " : "") << enumeration.name;
		if( !enumeration.base_type.empty() )
		{
			ss<< " : ";
			for( const CombinedName& base_type_part : enumeration.base_type )
			{
				base_type_part.Print( ss );
				ss<< " ";
			}
		}
		ss<< std::endl<< enumeration.members.size() << " members: "<< std::endl;
		for( const Enumeration::Member& member : enumeration.members )
		{
			ss<< member.name <<" = ";
			member.value.Print( ss );
			ss<< std::endl;
		}
		ss<< std::endl;
	}

	output_->setText( QString::fromStdString(ss.str()) );
}

void MainWindow::OnAbout()
{
	QMessageBox( QMessageBox::Information, "About", "Kr", QMessageBox::Close ).exec();
}
