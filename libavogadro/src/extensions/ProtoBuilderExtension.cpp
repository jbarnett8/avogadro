//
// Created by jbarnett8 on 5/21/17.
//

#include "ProtoBuilderExtension.h"

#include "ProtoBuilderExtension.h"
#include "insertcommand.h"

#include "ui_ProtoBuilderDialog.h"

#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>

#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

#include <QDebug>
#include <QFileDialog>

using namespace std;
using namespace OpenBabel;

namespace Avogadro {

    class ProtoBuilderDialog : public QDialog, public Ui::ProtoBuilderDialog
    {
    public:
        ProtoBuilderDialog(QWidget *parent=0) : QDialog(parent) {
            setWindowFlags(Qt::Dialog | Qt::Tool);
            setupUi(this);
        }
    };


    ProtoBuilderExtension::ProtoBuilderExtension(QObject *parent) :
            Extension(parent),
            m_molecule(0),
            m_dialog(0)
    {
        QAction *action = new QAction(this);
        action->setText(tr("Proto-Nucelic-Acid Builder..."));
        m_actions.append(action);

        m_widget = qobject_cast<GLWidget *>(parent);
    }

    ProtoBuilderExtension::~ProtoBuilderExtension()
    {
    }

    QList<QAction *> ProtoBuilderExtension::actions() const
    {
        return m_actions;
    }

    QString ProtoBuilderExtension::menuPath(QAction *) const
    {
        return tr("&Build") + '>' + tr("&Insert");
    }

    void ProtoBuilderExtension::setMolecule(Molecule *molecule)
    {
        m_molecule = molecule;
    }

    QUndoCommand* ProtoBuilderExtension::performAction(QAction *,
                                                    GLWidget *widget)
    {
        if (m_molecule == NULL)
            return NULL; // nothing we can do

        m_widget = widget; // save for delayed response

        if (m_dialog == NULL) {
            constructDialog();
        }
        m_dialog->show();

        return NULL; // delayed action on user clicking the Insert button
    }

    void ProtoBuilderExtension::performInsert()
    {
//        if (!m_dialog)
//            return; // nothing we can do
//
//        QString sequence = m_dialog->sequenceText->toPlainText().toLower();
//        bool dna = (m_dialog->typeComboBox->currentIndex() == 0);
//        if (sequence.isEmpty())
//            return; // also nothing to do
//        // Add DNA/RNA tag for FASTA
//        sequence = '>' + m_dialog->typeComboBox->currentText() + '\n'
//                   + sequence;
//
//        OBConversion conv;
//        if (!conv.SetInFormat("fasta"))
//            return; // need the format to do structure generation for us
//        // if DNA, check if the user wants single-strands
//        if (dna && m_dialog->singleStrandRadio->isChecked())
//            conv.AddOption("1", OBConversion::INOPTIONS);
//        // Add the number of turns
//        QString turns = QString("%1").arg(m_dialog->bpTurnsSpin->value());
//        conv.AddOption("t", OBConversion::INOPTIONS, turns.toAscii().data());
//
//        OBMol obfragment;
//        if (!conv.ReadString(&obfragment, sequence.toStdString()) )
//            return; // failed conversion

//        Molecule fragment;
//        fragment.setOBMol(&obfragment);
//        emit performCommand(new InsertFragmentCommand(m_molecule, fragment,
//                                                      m_widget, tr("Insert DNA")));
    }

    void ProtoBuilderExtension::writeSettings(QSettings &settings) const
    {
//        Extension::writeSettings(settings);
//
//        if (!m_dialog)
//            return; // nothing to save
//
//        settings.setValue("nucleicType", m_dialog->typeComboBox->currentIndex());
//        settings.setValue("basePairType", m_dialog->bpCombo->currentIndex());
//        settings.setValue("basePairPerTurn", m_dialog->bpTurnsSpin->value());
//        settings.setValue("singleStrand", m_dialog->singleStrandRadio->isChecked());
    }

    void ProtoBuilderExtension::readSettings(QSettings &settings)
    {
//        Extension::readSettings(settings);
//
//        if (!m_dialog)
//            constructDialog();
//
//        m_dialog->typeComboBox->setCurrentIndex(settings.value("nucleicType", 0).toInt());
//        m_dialog->bpCombo->setCurrentIndex(settings.value("basePairType", 1).toInt());
//        m_dialog->bpTurnsSpin->setValue(settings.value("basePairPerTurn", 10.5).toDouble());
//        m_dialog->singleStrandRadio->setChecked(settings.value("singleStrand", 0).toBool());
    }

    void ProtoBuilderExtension::constructDialog()
    {
        if (m_dialog == NULL) {
            m_dialog = new ProtoBuilderDialog(m_widget);

            connect(m_dialog->import_base_button, SIGNAL(clicked()),
                    this, SLOT(importCoreFile()));

//            QButtonGroup* numStrands = new QButtonGroup(m_dialog);
//            numStrands->addButton(m_dialog->singleStrandRadio, 0);
//            numStrands->addButton(m_dialog->doubleStrandRadio, 1);
//            numStrands->setExclusive(true);
//
//            connect(m_dialog->insertButton, SIGNAL(clicked()),
//                    this, SLOT(performInsert()));
//
//            connect(m_dialog->bpCombo, SIGNAL(currentIndexChanged(int)),
//                    this, SLOT(updateBPTurns(int)));
//
//            connect(m_dialog->typeComboBox, SIGNAL(currentIndexChanged(int)),
//                    this, SLOT(changeNucleicType(int)));
//
//            // Set the nucleic buttons to update the sequence
//                    foreach(const QToolButton *child, m_dialog->findChildren<QToolButton*>()) {
//                    connect(child, SIGNAL(clicked()), this, SLOT(updateText()));
//                }
//            connect(m_dialog, SIGNAL(destroyed()), this, SLOT(dialogDestroyed()));
        }
//        m_dialog->sequenceText->setPlainText(QString());
    }

    void ProtoBuilderExtension::updateText()
    {
//        QToolButton *button = qobject_cast<QToolButton*>(sender());
//        if (button) {
//            QString sequenceText = m_dialog->sequenceText->toPlainText();
//            sequenceText += button->text();
//
//            m_dialog->sequenceText->setPlainText(sequenceText);
//        }
    }

    void ProtoBuilderExtension::updateBPTurns(int type)
    {
//        switch(type) {
//            case 0: // A-DNA
//                m_dialog->bpTurnsSpin->setValue(11.0);
//                break;
//            case 1: // B-DNA
//                m_dialog->bpTurnsSpin->setValue(10.5);
//                break;
//            case 2: // Z-DNA
//                m_dialog->bpTurnsSpin->setValue(12.0);
//                break;
//            default:
//                // anything the user wants
//                break;
//        }
    }

    void ProtoBuilderExtension::changeNucleicType(int type)
    {
//        if (type == 1) { // RNA
//            m_dialog->bpCombo->setCurrentIndex(3); // other
//            m_dialog->bpTurnsSpin->setValue(11.0); // standard RNA
//            m_dialog->singleStrandRadio->setChecked(true);
//            m_dialog->singleStrandRadio->setEnabled(false);
//            m_dialog->doubleStrandRadio->setEnabled(false);
//            m_dialog->toolButton_TU->setText(tr("U", "uracil"));
//            m_dialog->toolButton_TU->setToolTip(tr("Uracil"));
//            return;
//        }
//        // DNA
//        m_dialog->singleStrandRadio->setEnabled(true);
//        m_dialog->doubleStrandRadio->setEnabled(true);
//        m_dialog->toolButton_TU->setText(tr("T", "thymine"));
//        m_dialog->toolButton_TU->setToolTip(tr("Thymine"));
    }

    void ProtoBuilderExtension::dialogDestroyed()
    {
        m_dialog = 0;
    }

    void ProtoBuilderExtension::importCoreFile() {
        QString fileName = QFileDialog::getOpenFileName(m_dialog, tr("Open Image"), "", tr("Molecule Files (*.cml)"));
        m_dialog->import_file_text->setText(fileName);
    }

    void ProtoBuilderExtension::loadCoreFile() {
        OBConversion conv;
        conv.SetInFormat("CML");
        OBMol *mol = new OBMol;
        if (!conv->ReadFile(mol, m_dialog->import_file_text->getText())) {
            cerr << "Error reading backbone file. Exiting..." << endl;
            exit(1);
        }
    }

} // end namespace Avogadro

Q_EXPORT_PLUGIN2(ProtoBuilderExtension, Avogadro::ProtoBuilderExtensionFactory)