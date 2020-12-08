/**********************************************************************

	--- Qt Architect generated file ---

	File: optionsdata.cpp
	Last generated: Sun Apr 26 23:24:19 1998

	DO NOT EDIT!!!  This file will be automatically
	regenerated by qtarch.  All changes will be lost.

 *********************************************************************/

#include "optionsdata.h"

#define Inherited QDialog

#include <qlabel.h>
#include <qpushbt.h>
#include <qframe.h>
#include <qlayout.h>

OptionsData::OptionsData
(
    QWidget *parent,
    const char *name
)
    :
    Inherited(parent, name, TRUE, 0)
{
    QFrame *dlgedit_Frame_3;
    dlgedit_Frame_3 = new QFrame(this, "Frame_3");
    dlgedit_Frame_3->setGeometry(0, 320, 280, 70);
    dlgedit_Frame_3->setMinimumSize(10, 10);
    dlgedit_Frame_3->setMaximumSize(32767, 32767);
    dlgedit_Frame_3->setFrameStyle(49);

    QFrame *dlgedit_Frame_2;
    dlgedit_Frame_2 = new QFrame(this, "Frame_2");
    dlgedit_Frame_2->setGeometry(0, 140, 280, 180);
    dlgedit_Frame_2->setMinimumSize(10, 10);
    dlgedit_Frame_2->setMaximumSize(32767, 32767);
    dlgedit_Frame_2->setFrameStyle(49);

    QFrame *dlgedit_Frame_1;
    dlgedit_Frame_1 = new QFrame(this, "Frame_1");
    dlgedit_Frame_1->setGeometry(0, 0, 280, 140);
    dlgedit_Frame_1->setMinimumSize(10, 10);
    dlgedit_Frame_1->setMaximumSize(32767, 32767);
    dlgedit_Frame_1->setFrameStyle(49);

    QPushButton *dlgedit_PushButton_1;
    dlgedit_PushButton_1 = new QPushButton(this, "PushButton_1");
    dlgedit_PushButton_1->setGeometry(30, 340, 80, 30);
    dlgedit_PushButton_1->setMinimumSize(10, 10);
    dlgedit_PushButton_1->setMaximumSize(32767, 32767);
    connect(dlgedit_PushButton_1, SIGNAL(clicked()), SLOT(my_accept()));
    dlgedit_PushButton_1->setText("OK");
    dlgedit_PushButton_1->setAutoRepeat(FALSE);
    dlgedit_PushButton_1->setAutoResize(FALSE);

    QPushButton *dlgedit_PushButton_2;
    dlgedit_PushButton_2 = new QPushButton(this, "PushButton_2");
    dlgedit_PushButton_2->setGeometry(170, 340, 80, 30);
    dlgedit_PushButton_2->setMinimumSize(10, 10);
    dlgedit_PushButton_2->setMaximumSize(32767, 32767);
    connect(dlgedit_PushButton_2, SIGNAL(clicked()), SLOT(accept()));
    dlgedit_PushButton_2->setText("Cancel");
    dlgedit_PushButton_2->setAutoRepeat(FALSE);
    dlgedit_PushButton_2->setAutoResize(FALSE);

    QLabel *dlgedit_Label_2;
    dlgedit_Label_2 = new QLabel(this, "Label_2");
    dlgedit_Label_2->setGeometry(20, 50, 160, 30);
    dlgedit_Label_2->setMinimumSize(10, 10);
    dlgedit_Label_2->setMaximumSize(32767, 32767);
    dlgedit_Label_2->setText("Recording Threshold:");
    dlgedit_Label_2->setAlignment(290);
    dlgedit_Label_2->setMargin(-1);

    rec_thresh = new QLineEdit(this, "LineEdit_1");
    rec_thresh->setGeometry(190, 50, 70, 30);
    rec_thresh->setMinimumSize(10, 10);
    rec_thresh->setMaximumSize(32767, 32767);
    rec_thresh->setText("");
    rec_thresh->setMaxLength(32767);
    rec_thresh->setEchoMode(QLineEdit::Normal);
    rec_thresh->setFrame(TRUE);

    QLabel *dlgedit_Label_4;
    dlgedit_Label_4 = new QLabel(this, "Label_4");
    dlgedit_Label_4->setGeometry(20, 90, 160, 30);
    dlgedit_Label_4->setMinimumSize(10, 10);
    dlgedit_Label_4->setMaximumSize(32767, 32767);
    dlgedit_Label_4->setText("Accepted Silence Frames:");
    dlgedit_Label_4->setAlignment(290);
    dlgedit_Label_4->setMargin(-1);

    acc_sil_frames = new QLineEdit(this, "LineEdit_2");
    acc_sil_frames->setGeometry(190, 90, 70, 30);
    acc_sil_frames->setMinimumSize(10, 10);
    acc_sil_frames->setMaximumSize(32767, 32767);
    acc_sil_frames->setText("");
    acc_sil_frames->setMaxLength(32767);
    acc_sil_frames->setEchoMode(QLineEdit::Normal);
    acc_sil_frames->setFrame(TRUE);

    QLabel *dlgedit_Label_5;
    dlgedit_Label_5 = new QLabel(this, "Label_5");
    dlgedit_Label_5->setGeometry(10, 150, 100, 30);
    dlgedit_Label_5->setMinimumSize(10, 10);
    dlgedit_Label_5->setMaximumSize(32767, 32767);
    dlgedit_Label_5->setFrameStyle(33);
    dlgedit_Label_5->setText("Recognizer:");
    dlgedit_Label_5->setAlignment(289);
    dlgedit_Label_5->setMargin(-1);

    QLabel *dlgedit_Label_6;
    dlgedit_Label_6 = new QLabel(this, "Label_6");
    dlgedit_Label_6->setGeometry(20, 190, 160, 30);
    dlgedit_Label_6->setMinimumSize(10, 10);
    dlgedit_Label_6->setMaximumSize(32767, 32767);
    dlgedit_Label_6->setText("Adjustment Window Width:");
    dlgedit_Label_6->setAlignment(290);
    dlgedit_Label_6->setMargin(-1);

    adj_win_width = new QLineEdit(this, "LineEdit_3");
    adj_win_width->setGeometry(190, 190, 70, 30);
    adj_win_width->setMinimumSize(10, 10);
    adj_win_width->setMaximumSize(32767, 32767);
    adj_win_width->setText("");
    adj_win_width->setMaxLength(32767);
    adj_win_width->setEchoMode(QLineEdit::Normal);
    adj_win_width->setFrame(TRUE);

    QLabel *dlgedit_Label_7;
    dlgedit_Label_7 = new QLabel(this, "Label_7");
    dlgedit_Label_7->setGeometry(20, 230, 160, 30);
    dlgedit_Label_7->setMinimumSize(10, 10);
    dlgedit_Label_7->setMaximumSize(32767, 32767);
    dlgedit_Label_7->setText("Rejection Threshold:");
    dlgedit_Label_7->setAlignment(290);
    dlgedit_Label_7->setMargin(-1);

    QLabel *dlgedit_Label_8;
    dlgedit_Label_8 = new QLabel(this, "Label_8");
    dlgedit_Label_8->setGeometry(20, 270, 160, 30);
    dlgedit_Label_8->setMinimumSize(10, 10);
    dlgedit_Label_8->setMaximumSize(32767, 32767);
    dlgedit_Label_8->setText("Minimum Distance Between Different Hypos:");
    dlgedit_Label_8->setAlignment(1314);
    dlgedit_Label_8->setMargin(-1);

    rej_thresh = new QLineEdit(this, "LineEdit_4");
    rej_thresh->setGeometry(190, 230, 70, 30);
    rej_thresh->setMinimumSize(10, 10);
    rej_thresh->setMaximumSize(32767, 32767);
    rej_thresh->setText("");
    rej_thresh->setMaxLength(32767);
    rej_thresh->setEchoMode(QLineEdit::Normal);
    rej_thresh->setFrame(TRUE);

    min_dist = new QLineEdit(this, "LineEdit_5");
    min_dist->setGeometry(190, 270, 70, 30);
    min_dist->setMinimumSize(10, 10);
    min_dist->setMaximumSize(32767, 32767);
    min_dist->setText("");
    min_dist->setMaxLength(32767);
    min_dist->setEchoMode(QLineEdit::Normal);
    min_dist->setFrame(TRUE);

    QLabel *dlgedit_Label_10;
    dlgedit_Label_10 = new QLabel(this, "Label_10");
    dlgedit_Label_10->setGeometry(10, 10, 100, 30);
    dlgedit_Label_10->setMinimumSize(10, 10);
    dlgedit_Label_10->setMaximumSize(32767, 32767);
    dlgedit_Label_10->setFrameStyle(33);
    dlgedit_Label_10->setText("Sound:");
    dlgedit_Label_10->setAlignment(289);
    dlgedit_Label_10->setMargin(-1);

    //QBoxLayout* dlgedit_layout_1 = new QBoxLayout( this, QBoxLayout::TopToBottom, 5, 5, NULL );
    //dlgedit_layout_1->addStrut( 0 );

    resize(280, 390);
    setMinimumSize(0, 0);
    setMaximumSize(32767, 32767);
}


OptionsData::~OptionsData()
{
}
void OptionsData::my_accept()
{
}
