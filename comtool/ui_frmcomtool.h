/********************************************************************************
** Form generated from reading UI file 'frmcomtool.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FRMCOMTOOL_H
#define UI_FRMCOMTOOL_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListView>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBox>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_frmComTool
{
public:
    QHBoxLayout *horizontalLayout_2;
    QVBoxLayout *verticalLayout;
    QToolBox *toolBox_2;
    QWidget *page_3;
    QHBoxLayout *horizontalLayout_4;
    QFrame *frameTop;
    QGridLayout *gridLayout_3;
    QLabel *labPortName;
    QComboBox *cboxPortName;
    QLabel *labBaudRate;
    QComboBox *cboxBaudRate;
    QLabel *labDataBit;
    QComboBox *cboxDataBit;
    QLabel *labParity;
    QComboBox *cboxParity;
    QLabel *labStopBit;
    QComboBox *cboxStopBit;
    QPushButton *btnOpen;
    QSpacerItem *horizontalSpacer;
    QWidget *page_4;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QTextEdit *textEdit;
    QVBoxLayout *verticalLayout_3;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QPushButton *pushButton_5;
    QPlainTextEdit *plainTextEdit;
    QWidget *page;
    QVBoxLayout *verticalLayout_4;
    QTreeWidget *treeWidget;
    QListView *listView;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *pushButton_3;
    QPushButton *pushButton_4;
    QTreeWidget *treeWidget_2;

    void setupUi(QWidget *frmComTool)
    {
        if (frmComTool->objectName().isEmpty())
            frmComTool->setObjectName("frmComTool");
        frmComTool->resize(826, 632);
        frmComTool->setStyleSheet(QString::fromUtf8("background-color: #e0e0e0;"));
        horizontalLayout_2 = new QHBoxLayout(frmComTool);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(-1, 12, -1, -1);
        toolBox_2 = new QToolBox(frmComTool);
        toolBox_2->setObjectName("toolBox_2");
        page_3 = new QWidget();
        page_3->setObjectName("page_3");
        page_3->setGeometry(QRect(0, 0, 399, 483));
        horizontalLayout_4 = new QHBoxLayout(page_3);
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        frameTop = new QFrame(page_3);
        frameTop->setObjectName("frameTop");
        frameTop->setFrameShape(QFrame::Shape::Box);
        frameTop->setFrameShadow(QFrame::Shadow::Sunken);
        gridLayout_3 = new QGridLayout(frameTop);
        gridLayout_3->setObjectName("gridLayout_3");
        gridLayout_3->setContentsMargins(6, 6, 6, 6);
        labPortName = new QLabel(frameTop);
        labPortName->setObjectName("labPortName");

        gridLayout_3->addWidget(labPortName, 0, 0, 1, 1);

        cboxPortName = new QComboBox(frameTop);
        cboxPortName->setObjectName("cboxPortName");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(cboxPortName->sizePolicy().hasHeightForWidth());
        cboxPortName->setSizePolicy(sizePolicy);
        cboxPortName->setEditable(true);

        gridLayout_3->addWidget(cboxPortName, 0, 1, 1, 1);

        labBaudRate = new QLabel(frameTop);
        labBaudRate->setObjectName("labBaudRate");

        gridLayout_3->addWidget(labBaudRate, 1, 0, 1, 1);

        cboxBaudRate = new QComboBox(frameTop);
        cboxBaudRate->setObjectName("cboxBaudRate");
        sizePolicy.setHeightForWidth(cboxBaudRate->sizePolicy().hasHeightForWidth());
        cboxBaudRate->setSizePolicy(sizePolicy);
        cboxBaudRate->setEditable(true);

        gridLayout_3->addWidget(cboxBaudRate, 1, 1, 1, 1);

        labDataBit = new QLabel(frameTop);
        labDataBit->setObjectName("labDataBit");

        gridLayout_3->addWidget(labDataBit, 2, 0, 1, 1);

        cboxDataBit = new QComboBox(frameTop);
        cboxDataBit->setObjectName("cboxDataBit");
        sizePolicy.setHeightForWidth(cboxDataBit->sizePolicy().hasHeightForWidth());
        cboxDataBit->setSizePolicy(sizePolicy);

        gridLayout_3->addWidget(cboxDataBit, 2, 1, 1, 1);

        labParity = new QLabel(frameTop);
        labParity->setObjectName("labParity");

        gridLayout_3->addWidget(labParity, 3, 0, 1, 1);

        cboxParity = new QComboBox(frameTop);
        cboxParity->setObjectName("cboxParity");
        sizePolicy.setHeightForWidth(cboxParity->sizePolicy().hasHeightForWidth());
        cboxParity->setSizePolicy(sizePolicy);

        gridLayout_3->addWidget(cboxParity, 3, 1, 1, 1);

        labStopBit = new QLabel(frameTop);
        labStopBit->setObjectName("labStopBit");

        gridLayout_3->addWidget(labStopBit, 4, 0, 1, 1);

        cboxStopBit = new QComboBox(frameTop);
        cboxStopBit->setObjectName("cboxStopBit");
        sizePolicy.setHeightForWidth(cboxStopBit->sizePolicy().hasHeightForWidth());
        cboxStopBit->setSizePolicy(sizePolicy);

        gridLayout_3->addWidget(cboxStopBit, 4, 1, 1, 1);

        btnOpen = new QPushButton(frameTop);
        btnOpen->setObjectName("btnOpen");

        gridLayout_3->addWidget(btnOpen, 5, 0, 1, 2);


        horizontalLayout_4->addWidget(frameTop);

        horizontalSpacer = new QSpacerItem(184, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer);

        toolBox_2->addItem(page_3, QString::fromUtf8("\345\217\202\346\225\260\351\205\215\347\275\256"));
        page_4 = new QWidget();
        page_4->setObjectName("page_4");
        page_4->setGeometry(QRect(0, 0, 399, 483));
        verticalLayout_2 = new QVBoxLayout(page_4);
        verticalLayout_2->setObjectName("verticalLayout_2");
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        textEdit = new QTextEdit(page_4);
        textEdit->setObjectName("textEdit");

        horizontalLayout->addWidget(textEdit);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setContentsMargins(12, -1, -1, -1);
        pushButton = new QPushButton(page_4);
        pushButton->setObjectName("pushButton");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(pushButton->sizePolicy().hasHeightForWidth());
        pushButton->setSizePolicy(sizePolicy1);

        verticalLayout_3->addWidget(pushButton);

        pushButton_2 = new QPushButton(page_4);
        pushButton_2->setObjectName("pushButton_2");
        sizePolicy1.setHeightForWidth(pushButton_2->sizePolicy().hasHeightForWidth());
        pushButton_2->setSizePolicy(sizePolicy1);

        verticalLayout_3->addWidget(pushButton_2);

        pushButton_5 = new QPushButton(page_4);
        pushButton_5->setObjectName("pushButton_5");
        sizePolicy1.setHeightForWidth(pushButton_5->sizePolicy().hasHeightForWidth());
        pushButton_5->setSizePolicy(sizePolicy1);

        verticalLayout_3->addWidget(pushButton_5);


        horizontalLayout->addLayout(verticalLayout_3);


        verticalLayout_2->addLayout(horizontalLayout);

        plainTextEdit = new QPlainTextEdit(page_4);
        plainTextEdit->setObjectName("plainTextEdit");

        verticalLayout_2->addWidget(plainTextEdit);

        toolBox_2->addItem(page_4, QString::fromUtf8("\350\260\203\350\257\225\347\225\214\351\235\242"));
        page = new QWidget();
        page->setObjectName("page");
        page->setGeometry(QRect(0, 0, 399, 483));
        verticalLayout_4 = new QVBoxLayout(page);
        verticalLayout_4->setObjectName("verticalLayout_4");
        treeWidget = new QTreeWidget(page);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        treeWidget->setHeaderItem(__qtreewidgetitem);
        treeWidget->setObjectName("treeWidget");
        treeWidget->header()->setVisible(false);

        verticalLayout_4->addWidget(treeWidget);

        listView = new QListView(page);
        listView->setObjectName("listView");

        verticalLayout_4->addWidget(listView);

        toolBox_2->addItem(page, QString::fromUtf8("\344\273\273\345\212\241\345\210\227\350\241\250"));

        verticalLayout->addWidget(toolBox_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        horizontalLayout_3->setContentsMargins(-1, 0, -1, -1);
        pushButton_3 = new QPushButton(frmComTool);
        pushButton_3->setObjectName("pushButton_3");

        horizontalLayout_3->addWidget(pushButton_3);

        pushButton_4 = new QPushButton(frmComTool);
        pushButton_4->setObjectName("pushButton_4");

        horizontalLayout_3->addWidget(pushButton_4);


        verticalLayout->addLayout(horizontalLayout_3);


        horizontalLayout_2->addLayout(verticalLayout);

        treeWidget_2 = new QTreeWidget(frmComTool);
        treeWidget_2->setObjectName("treeWidget_2");
        treeWidget_2->header()->setVisible(true);

        horizontalLayout_2->addWidget(treeWidget_2);


        retranslateUi(frmComTool);

        toolBox_2->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(frmComTool);
    } // setupUi

    void retranslateUi(QWidget *frmComTool)
    {
        labPortName->setText(QCoreApplication::translate("frmComTool", "\344\270\262\345\217\243\345\217\267", nullptr));
        labBaudRate->setText(QCoreApplication::translate("frmComTool", "\346\263\242\347\211\271\347\216\207", nullptr));
        labDataBit->setText(QCoreApplication::translate("frmComTool", "\346\225\260\346\215\256\344\275\215", nullptr));
        labParity->setText(QCoreApplication::translate("frmComTool", "\346\240\241\351\252\214\344\275\215", nullptr));
        labStopBit->setText(QCoreApplication::translate("frmComTool", "\345\201\234\346\255\242\344\275\215", nullptr));
        btnOpen->setText(QCoreApplication::translate("frmComTool", "\346\211\223\345\274\200\344\270\262\345\217\243", nullptr));
        toolBox_2->setItemText(toolBox_2->indexOf(page_3), QCoreApplication::translate("frmComTool", "\345\217\202\346\225\260\351\205\215\347\275\256", nullptr));
        pushButton->setText(QCoreApplication::translate("frmComTool", "\350\247\243\346\236\220", nullptr));
        pushButton_2->setText(QCoreApplication::translate("frmComTool", "\345\217\221\351\200\201", nullptr));
        pushButton_5->setText(QCoreApplication::translate("frmComTool", "\350\247\243\346\236\220\345\233\276\345\203\217", nullptr));
        toolBox_2->setItemText(toolBox_2->indexOf(page_4), QCoreApplication::translate("frmComTool", "\350\260\203\350\257\225\347\225\214\351\235\242", nullptr));
        toolBox_2->setItemText(toolBox_2->indexOf(page), QCoreApplication::translate("frmComTool", "\344\273\273\345\212\241\345\210\227\350\241\250", nullptr));
        pushButton_3->setText(QCoreApplication::translate("frmComTool", "(\345\217\221\351\200\201)\344\273\273\345\212\241\347\241\256\350\256\244", nullptr));
        pushButton_4->setText(QCoreApplication::translate("frmComTool", "(\345\217\221\351\200\201)\346\225\260\346\215\256\346\226\207\344\273\266", nullptr));
        QTreeWidgetItem *___qtreewidgetitem = treeWidget_2->headerItem();
        ___qtreewidgetitem->setText(2, QCoreApplication::translate("frmComTool", "\346\225\260\345\200\274", nullptr));
        ___qtreewidgetitem->setText(1, QCoreApplication::translate("frmComTool", "\347\261\273\345\236\213", nullptr));
        ___qtreewidgetitem->setText(0, QCoreApplication::translate("frmComTool", "\345\255\227\346\256\265", nullptr));
        (void)frmComTool;
    } // retranslateUi

};

namespace Ui {
    class frmComTool: public Ui_frmComTool {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FRMCOMTOOL_H
