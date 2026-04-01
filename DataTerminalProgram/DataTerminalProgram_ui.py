# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'DataTerminalProgram.ui'
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
from PySide6.QtWidgets import (QApplication, QComboBox, QHBoxLayout, QLabel,
    QMainWindow, QPlainTextEdit, QPushButton, QSizePolicy,
    QSpacerItem, QStatusBar, QTextEdit, QToolBox,
    QVBoxLayout, QWidget)
import DataTerminalProgram_rc

class Ui_DataTerminalProgramClass(object):
    def setupUi(self, DataTerminalProgramClass):
        if not DataTerminalProgramClass.objectName():
            DataTerminalProgramClass.setObjectName(u"DataTerminalProgramClass")
        DataTerminalProgramClass.resize(649, 671)
        self.centralWidget = QWidget(DataTerminalProgramClass)
        self.centralWidget.setObjectName(u"centralWidget")
        self.verticalLayout = QVBoxLayout(self.centralWidget)
        self.verticalLayout.setSpacing(6)
        self.verticalLayout.setContentsMargins(11, 11, 11, 11)
        self.verticalLayout.setObjectName(u"verticalLayout")
        self.toolBox = QToolBox(self.centralWidget)
        self.toolBox.setObjectName(u"toolBox")
        sizePolicy = QSizePolicy(QSizePolicy.Policy.Preferred, QSizePolicy.Policy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.toolBox.sizePolicy().hasHeightForWidth())
        self.toolBox.setSizePolicy(sizePolicy)
        self.page = QWidget()
        self.page.setObjectName(u"page")
        self.page.setGeometry(QRect(0, 0, 631, 212))
        self.verticalLayout_2 = QVBoxLayout(self.page)
        self.verticalLayout_2.setSpacing(0)
        self.verticalLayout_2.setContentsMargins(11, 11, 11, 11)
        self.verticalLayout_2.setObjectName(u"verticalLayout_2")
        self.verticalLayout_2.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout = QHBoxLayout()
        self.horizontalLayout.setSpacing(6)
        self.horizontalLayout.setObjectName(u"horizontalLayout")
        self.textEdit = QTextEdit(self.page)
        self.textEdit.setObjectName(u"textEdit")

        self.horizontalLayout.addWidget(self.textEdit)

        self.pushButton = QPushButton(self.page)
        self.pushButton.setObjectName(u"pushButton")
        sizePolicy1 = QSizePolicy(QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Preferred)
        sizePolicy1.setHorizontalStretch(0)
        sizePolicy1.setVerticalStretch(0)
        sizePolicy1.setHeightForWidth(self.pushButton.sizePolicy().hasHeightForWidth())
        self.pushButton.setSizePolicy(sizePolicy1)

        self.horizontalLayout.addWidget(self.pushButton)


        self.verticalLayout_2.addLayout(self.horizontalLayout)

        self.toolBox.addItem(self.page, u"\u8c03\u8bd5\u754c\u9762")
        self.page_2 = QWidget()
        self.page_2.setObjectName(u"page_2")
        self.page_2.setGeometry(QRect(0, 0, 631, 68))
        self.verticalLayout_3 = QVBoxLayout(self.page_2)
        self.verticalLayout_3.setSpacing(0)
        self.verticalLayout_3.setContentsMargins(11, 11, 11, 11)
        self.verticalLayout_3.setObjectName(u"verticalLayout_3")
        self.verticalLayout_3.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout_2 = QHBoxLayout()
        self.horizontalLayout_2.setSpacing(0)
        self.horizontalLayout_2.setObjectName(u"horizontalLayout_2")
        self.label = QLabel(self.page_2)
        self.label.setObjectName(u"label")
        sizePolicy2 = QSizePolicy(QSizePolicy.Policy.Fixed, QSizePolicy.Policy.Preferred)
        sizePolicy2.setHorizontalStretch(0)
        sizePolicy2.setVerticalStretch(0)
        sizePolicy2.setHeightForWidth(self.label.sizePolicy().hasHeightForWidth())
        self.label.setSizePolicy(sizePolicy2)

        self.horizontalLayout_2.addWidget(self.label)

        self.comboBox = QComboBox(self.page_2)
        self.comboBox.setObjectName(u"comboBox")

        self.horizontalLayout_2.addWidget(self.comboBox)

        self.pushButton_5 = QPushButton(self.page_2)
        self.pushButton_5.setObjectName(u"pushButton_5")

        self.horizontalLayout_2.addWidget(self.pushButton_5)

        self.pushButton_2 = QPushButton(self.page_2)
        self.pushButton_2.setObjectName(u"pushButton_2")

        self.horizontalLayout_2.addWidget(self.pushButton_2)


        self.verticalLayout_3.addLayout(self.horizontalLayout_2)

        self.toolBox.addItem(self.page_2, u"\u53c2\u6570\u914d\u7f6e")

        self.verticalLayout.addWidget(self.toolBox)

        self.plainTextEdit = QPlainTextEdit(self.centralWidget)
        self.plainTextEdit.setObjectName(u"plainTextEdit")

        self.verticalLayout.addWidget(self.plainTextEdit)

        self.horizontalLayout_3 = QHBoxLayout()
        self.horizontalLayout_3.setSpacing(6)
        self.horizontalLayout_3.setObjectName(u"horizontalLayout_3")
        self.horizontalLayout_3.setContentsMargins(-1, 0, -1, -1)
        self.pushButton_3 = QPushButton(self.centralWidget)
        self.pushButton_3.setObjectName(u"pushButton_3")

        self.horizontalLayout_3.addWidget(self.pushButton_3)

        self.pushButton_4 = QPushButton(self.centralWidget)
        self.pushButton_4.setObjectName(u"pushButton_4")

        self.horizontalLayout_3.addWidget(self.pushButton_4)


        self.verticalLayout.addLayout(self.horizontalLayout_3)

        self.verticalSpacer = QSpacerItem(20, 40, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Expanding)

        self.verticalLayout.addItem(self.verticalSpacer)

        DataTerminalProgramClass.setCentralWidget(self.centralWidget)
        self.statusBar = QStatusBar(DataTerminalProgramClass)
        self.statusBar.setObjectName(u"statusBar")
        DataTerminalProgramClass.setStatusBar(self.statusBar)

        self.retranslateUi(DataTerminalProgramClass)

        self.toolBox.setCurrentIndex(1)


        QMetaObject.connectSlotsByName(DataTerminalProgramClass)
    # setupUi

    def retranslateUi(self, DataTerminalProgramClass):
        DataTerminalProgramClass.setWindowTitle(QCoreApplication.translate("DataTerminalProgramClass", u"DataTerminalProgram", None))
        self.pushButton.setText(QCoreApplication.translate("DataTerminalProgramClass", u"\u89e3\u6790", None))
        self.toolBox.setItemText(self.toolBox.indexOf(self.page), QCoreApplication.translate("DataTerminalProgramClass", u"\u8c03\u8bd5\u754c\u9762", None))
        self.label.setText(QCoreApplication.translate("DataTerminalProgramClass", u"\u4e32\u53e3\u5217\u8868", None))
        self.pushButton_5.setText(QCoreApplication.translate("DataTerminalProgramClass", u"\u67e5\u627e", None))
        self.pushButton_2.setText(QCoreApplication.translate("DataTerminalProgramClass", u"\u8fde\u63a5", None))
        self.toolBox.setItemText(self.toolBox.indexOf(self.page_2), QCoreApplication.translate("DataTerminalProgramClass", u"\u53c2\u6570\u914d\u7f6e", None))
        self.pushButton_3.setText(QCoreApplication.translate("DataTerminalProgramClass", u"(\u53d1\u9001)\u4efb\u52a1\u786e\u8ba4", None))
        self.pushButton_4.setText(QCoreApplication.translate("DataTerminalProgramClass", u"(\u53d1\u9001)\u6570\u636e\u6587\u4ef6", None))
    # retranslateUi

