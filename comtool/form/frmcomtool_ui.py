# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'frmcomtool.ui'
##
## Created by: Qt User Interface Compiler version 6.10.2
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide6.QtCore import (QCoreApplication, QDate, QDateTime, QLocale,
    QMetaObject, QObject, QPoint, QRect,
    QSize, QTime, QUrl, Qt)
from PySide6.QtGui import (QBrush, QColor, QConicalGradient, QCursor,
    QFont, QFontDatabase, QGradient, QIcon,
    QImage, QKeySequence, QLinearGradient, QPainter,
    QPalette, QPixmap, QRadialGradient, QTransform)
from PySide6.QtWidgets import (QApplication, QComboBox, QFrame, QGridLayout,
    QHBoxLayout, QHeaderView, QLabel, QPlainTextEdit,
    QPushButton, QSizePolicy, QSpacerItem, QTextEdit,
    QToolBox, QTreeWidget, QTreeWidgetItem, QVBoxLayout,
    QWidget)
import main_rc

class Ui_frmComTool(object):
    def setupUi(self, frmComTool):
        if not frmComTool.objectName():
            frmComTool.setObjectName(u"frmComTool")
        frmComTool.resize(826, 632)
        frmComTool.setStyleSheet(u"background-color: #e0e0e0;")
        self.horizontalLayout_2 = QHBoxLayout(frmComTool)
        self.horizontalLayout_2.setObjectName(u"horizontalLayout_2")
        self.verticalLayout = QVBoxLayout()
        self.verticalLayout.setObjectName(u"verticalLayout")
        self.verticalLayout.setContentsMargins(-1, 12, -1, -1)
        self.toolBox_2 = QToolBox(frmComTool)
        self.toolBox_2.setObjectName(u"toolBox_2")
        self.page_3 = QWidget()
        self.page_3.setObjectName(u"page_3")
        self.page_3.setGeometry(QRect(0, 0, 154, 202))
        self.horizontalLayout_4 = QHBoxLayout(self.page_3)
        self.horizontalLayout_4.setObjectName(u"horizontalLayout_4")
        self.frameTop = QFrame(self.page_3)
        self.frameTop.setObjectName(u"frameTop")
        self.frameTop.setFrameShape(QFrame.Shape.Box)
        self.frameTop.setFrameShadow(QFrame.Shadow.Sunken)
        self.gridLayout_3 = QGridLayout(self.frameTop)
        self.gridLayout_3.setObjectName(u"gridLayout_3")
        self.gridLayout_3.setContentsMargins(6, 6, 6, 6)
        self.labPortName = QLabel(self.frameTop)
        self.labPortName.setObjectName(u"labPortName")

        self.gridLayout_3.addWidget(self.labPortName, 0, 0, 1, 1)

        self.cboxPortName = QComboBox(self.frameTop)
        self.cboxPortName.setObjectName(u"cboxPortName")
        sizePolicy = QSizePolicy(QSizePolicy.Policy.Preferred, QSizePolicy.Policy.Fixed)
        sizePolicy.setHorizontalStretch(1)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.cboxPortName.sizePolicy().hasHeightForWidth())
        self.cboxPortName.setSizePolicy(sizePolicy)
        self.cboxPortName.setEditable(True)

        self.gridLayout_3.addWidget(self.cboxPortName, 0, 1, 1, 1)

        self.labBaudRate = QLabel(self.frameTop)
        self.labBaudRate.setObjectName(u"labBaudRate")

        self.gridLayout_3.addWidget(self.labBaudRate, 1, 0, 1, 1)

        self.cboxBaudRate = QComboBox(self.frameTop)
        self.cboxBaudRate.setObjectName(u"cboxBaudRate")
        sizePolicy.setHeightForWidth(self.cboxBaudRate.sizePolicy().hasHeightForWidth())
        self.cboxBaudRate.setSizePolicy(sizePolicy)
        self.cboxBaudRate.setEditable(True)

        self.gridLayout_3.addWidget(self.cboxBaudRate, 1, 1, 1, 1)

        self.labDataBit = QLabel(self.frameTop)
        self.labDataBit.setObjectName(u"labDataBit")

        self.gridLayout_3.addWidget(self.labDataBit, 2, 0, 1, 1)

        self.cboxDataBit = QComboBox(self.frameTop)
        self.cboxDataBit.setObjectName(u"cboxDataBit")
        sizePolicy.setHeightForWidth(self.cboxDataBit.sizePolicy().hasHeightForWidth())
        self.cboxDataBit.setSizePolicy(sizePolicy)

        self.gridLayout_3.addWidget(self.cboxDataBit, 2, 1, 1, 1)

        self.labParity = QLabel(self.frameTop)
        self.labParity.setObjectName(u"labParity")

        self.gridLayout_3.addWidget(self.labParity, 3, 0, 1, 1)

        self.cboxParity = QComboBox(self.frameTop)
        self.cboxParity.setObjectName(u"cboxParity")
        sizePolicy.setHeightForWidth(self.cboxParity.sizePolicy().hasHeightForWidth())
        self.cboxParity.setSizePolicy(sizePolicy)

        self.gridLayout_3.addWidget(self.cboxParity, 3, 1, 1, 1)

        self.labStopBit = QLabel(self.frameTop)
        self.labStopBit.setObjectName(u"labStopBit")

        self.gridLayout_3.addWidget(self.labStopBit, 4, 0, 1, 1)

        self.cboxStopBit = QComboBox(self.frameTop)
        self.cboxStopBit.setObjectName(u"cboxStopBit")
        sizePolicy.setHeightForWidth(self.cboxStopBit.sizePolicy().hasHeightForWidth())
        self.cboxStopBit.setSizePolicy(sizePolicy)

        self.gridLayout_3.addWidget(self.cboxStopBit, 4, 1, 1, 1)

        self.btnOpen = QPushButton(self.frameTop)
        self.btnOpen.setObjectName(u"btnOpen")

        self.gridLayout_3.addWidget(self.btnOpen, 5, 0, 1, 2)


        self.horizontalLayout_4.addWidget(self.frameTop)

        self.horizontalSpacer = QSpacerItem(184, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)

        self.horizontalLayout_4.addItem(self.horizontalSpacer)

        self.toolBox_2.addItem(self.page_3, u"\u53c2\u6570\u914d\u7f6e")
        self.page_4 = QWidget()
        self.page_4.setObjectName(u"page_4")
        self.page_4.setGeometry(QRect(0, 0, 189, 179))
        self.verticalLayout_2 = QVBoxLayout(self.page_4)
        self.verticalLayout_2.setObjectName(u"verticalLayout_2")
        self.horizontalLayout = QHBoxLayout()
        self.horizontalLayout.setObjectName(u"horizontalLayout")
        self.textEdit = QTextEdit(self.page_4)
        self.textEdit.setObjectName(u"textEdit")

        self.horizontalLayout.addWidget(self.textEdit)

        self.verticalLayout_3 = QVBoxLayout()
        self.verticalLayout_3.setObjectName(u"verticalLayout_3")
        self.verticalLayout_3.setContentsMargins(12, -1, -1, -1)
        self.pushButton = QPushButton(self.page_4)
        self.pushButton.setObjectName(u"pushButton")
        sizePolicy1 = QSizePolicy(QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Preferred)
        sizePolicy1.setHorizontalStretch(0)
        sizePolicy1.setVerticalStretch(0)
        sizePolicy1.setHeightForWidth(self.pushButton.sizePolicy().hasHeightForWidth())
        self.pushButton.setSizePolicy(sizePolicy1)

        self.verticalLayout_3.addWidget(self.pushButton)

        self.pushButton_2 = QPushButton(self.page_4)
        self.pushButton_2.setObjectName(u"pushButton_2")
        sizePolicy1.setHeightForWidth(self.pushButton_2.sizePolicy().hasHeightForWidth())
        self.pushButton_2.setSizePolicy(sizePolicy1)

        self.verticalLayout_3.addWidget(self.pushButton_2)

        self.pushButton_5 = QPushButton(self.page_4)
        self.pushButton_5.setObjectName(u"pushButton_5")
        sizePolicy1.setHeightForWidth(self.pushButton_5.sizePolicy().hasHeightForWidth())
        self.pushButton_5.setSizePolicy(sizePolicy1)

        self.verticalLayout_3.addWidget(self.pushButton_5)


        self.horizontalLayout.addLayout(self.verticalLayout_3)


        self.verticalLayout_2.addLayout(self.horizontalLayout)

        self.plainTextEdit = QPlainTextEdit(self.page_4)
        self.plainTextEdit.setObjectName(u"plainTextEdit")

        self.verticalLayout_2.addWidget(self.plainTextEdit)

        self.toolBox_2.addItem(self.page_4, u"\u8c03\u8bd5\u754c\u9762")
        self.page = QWidget()
        self.page.setObjectName(u"page")
        self.page.setGeometry(QRect(0, 0, 399, 483))
        self.verticalLayout_4 = QVBoxLayout(self.page)
        self.verticalLayout_4.setObjectName(u"verticalLayout_4")
        self.treeWidget = QTreeWidget(self.page)
        __qtreewidgetitem = QTreeWidgetItem()
        __qtreewidgetitem.setText(0, u"1");
        self.treeWidget.setHeaderItem(__qtreewidgetitem)
        self.treeWidget.setObjectName(u"treeWidget")
        self.treeWidget.header().setVisible(False)

        self.verticalLayout_4.addWidget(self.treeWidget)

        self.toolBox_2.addItem(self.page, u"\u4efb\u52a1\u5217\u8868")

        self.verticalLayout.addWidget(self.toolBox_2)

        self.horizontalLayout_3 = QHBoxLayout()
        self.horizontalLayout_3.setObjectName(u"horizontalLayout_3")
        self.horizontalLayout_3.setContentsMargins(-1, 0, -1, -1)
        self.pushButton_3 = QPushButton(frmComTool)
        self.pushButton_3.setObjectName(u"pushButton_3")

        self.horizontalLayout_3.addWidget(self.pushButton_3)

        self.pushButton_6 = QPushButton(frmComTool)
        self.pushButton_6.setObjectName(u"pushButton_6")

        self.horizontalLayout_3.addWidget(self.pushButton_6)

        self.pushButton_4 = QPushButton(frmComTool)
        self.pushButton_4.setObjectName(u"pushButton_4")

        self.horizontalLayout_3.addWidget(self.pushButton_4)


        self.verticalLayout.addLayout(self.horizontalLayout_3)


        self.horizontalLayout_2.addLayout(self.verticalLayout)

        self.treeWidget_2 = QTreeWidget(frmComTool)
        self.treeWidget_2.setObjectName(u"treeWidget_2")
        self.treeWidget_2.header().setVisible(True)

        self.horizontalLayout_2.addWidget(self.treeWidget_2)


        self.retranslateUi(frmComTool)

        self.toolBox_2.setCurrentIndex(2)


        QMetaObject.connectSlotsByName(frmComTool)
    # setupUi

    def retranslateUi(self, frmComTool):
        self.labPortName.setText(QCoreApplication.translate("frmComTool", u"\u4e32\u53e3\u53f7", None))
        self.labBaudRate.setText(QCoreApplication.translate("frmComTool", u"\u6ce2\u7279\u7387", None))
        self.labDataBit.setText(QCoreApplication.translate("frmComTool", u"\u6570\u636e\u4f4d", None))
        self.labParity.setText(QCoreApplication.translate("frmComTool", u"\u6821\u9a8c\u4f4d", None))
        self.labStopBit.setText(QCoreApplication.translate("frmComTool", u"\u505c\u6b62\u4f4d", None))
        self.btnOpen.setText(QCoreApplication.translate("frmComTool", u"\u6253\u5f00\u4e32\u53e3", None))
        self.toolBox_2.setItemText(self.toolBox_2.indexOf(self.page_3), QCoreApplication.translate("frmComTool", u"\u53c2\u6570\u914d\u7f6e", None))
        self.pushButton.setText(QCoreApplication.translate("frmComTool", u"\u89e3\u6790", None))
        self.pushButton_2.setText(QCoreApplication.translate("frmComTool", u"\u53d1\u9001", None))
        self.pushButton_5.setText(QCoreApplication.translate("frmComTool", u"\u89e3\u6790\u56fe\u50cf", None))
        self.toolBox_2.setItemText(self.toolBox_2.indexOf(self.page_4), QCoreApplication.translate("frmComTool", u"\u8c03\u8bd5\u754c\u9762", None))
        self.toolBox_2.setItemText(self.toolBox_2.indexOf(self.page), QCoreApplication.translate("frmComTool", u"\u4efb\u52a1\u5217\u8868", None))
        self.pushButton_3.setText(QCoreApplication.translate("frmComTool", u"(\u53d1\u9001)\u4efb\u52a1\u786e\u8ba4", None))
        self.pushButton_6.setText(QCoreApplication.translate("frmComTool", u"\u63d2\u5165\u56fe\u7247", None))
        self.pushButton_4.setText(QCoreApplication.translate("frmComTool", u"(\u53d1\u9001)\u6570\u636e\u6587\u4ef6", None))
        ___qtreewidgetitem = self.treeWidget_2.headerItem()
        ___qtreewidgetitem.setText(2, QCoreApplication.translate("frmComTool", u"\u6570\u503c", None));
        ___qtreewidgetitem.setText(1, QCoreApplication.translate("frmComTool", u"\u7c7b\u578b", None));
        ___qtreewidgetitem.setText(0, QCoreApplication.translate("frmComTool", u"\u5b57\u6bb5", None));
        pass
    # retranslateUi

