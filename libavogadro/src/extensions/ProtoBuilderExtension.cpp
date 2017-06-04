//
// Created by jbarnett8 on 5/21/17.
//

#include "ProtoBuilderExtension.h"

#include "insertcommand.h"

#include "ui_ProtoBuilderDialog.h"

#include <avogadro/glwidget.h>

#include <openbabel/mol.h>

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

    void ProtoBuilderExtension::constructDialog() {
        if (m_dialog == NULL) {
            m_dialog = new ProtoBuilderDialog(m_widget);

            connect(m_dialog->import_core_button, SIGNAL(clicked()),
                    this, SLOT(importCoreFile()));
            connect(m_dialog->import_backbone_button, SIGNAL(clicked()),
                    this, SLOT(importBackboneFile()));

            m_dialog->NA_core_table->setColumnWidth(0, 100);
            m_dialog->NA_core_table->setColumnWidth(1, 195);
            m_dialog->backbone_table->setColumnWidth(0, 100);
            m_dialog->backbone_table->setColumnWidth(1, 195);

            connect(m_dialog->load_from_file_button_core, SIGNAL(clicked()),
                    this, SLOT(loadCoreFile()));
            connect(m_dialog->load_from_file_button_backbone, SIGNAL(clicked()),
                    this, SLOT(loadBackboneFile()));

            connect(m_molecule, SIGNAL(primitiveAdded(Primitive * )), this, SLOT(moleculeUpdated()));
            connect(m_molecule, SIGNAL(primitiveUpdated(Primitive * )), this, SLOT(moleculeUpdated()));
            connect(m_molecule, SIGNAL(primitiveRemoved(Primitive * )), this, SLOT(moleculeUpdated()));

            connect(m_dialog->NA_core_table, SIGNAL(itemSelectionChanged()),
                    this, SLOT(moleculeSelectionChangedCore()));
            connect(m_dialog->backbone_table, SIGNAL(itemSelectionChanged()),
                    this, SLOT(moleculeSelectionChangedBackbone()));

            connect(m_dialog->include_excluded_torsion_button, SIGNAL(clicked()),
                    this, SLOT(addTorsionExclusionList()));
            connect(m_dialog->clear_torsions_button, SIGNAL(clicked()),
                    this, SLOT(clearTorsionExclusionList()));
        }
    }

    void ProtoBuilderExtension::dialogDestroyed()
    {
        m_dialog = 0;
    }

    void ProtoBuilderExtension::importCoreFile() {
        QString fileName = QFileDialog::getOpenFileName(m_dialog, tr("Open CML File"), "", tr("Molecule Files (*.cml)"));
        m_dialog->import_file_text_2->setText(fileName);
    }

    void ProtoBuilderExtension::loadCoreFile() {
        OBConversion conv;
        conv.SetInFormat("CML");
        OBMol mol;
        if (!conv.ReadFile(&mol, m_dialog->import_file_text_2->text().toStdString())) {
            cerr << "Error reading core file." << endl;
            QMessageBox::critical(m_dialog, "Error", "There was an error loading the cml file. Please make sure you specified the file correctly.");
        }
        OBAtom *a;
        mol.Center();
        m_dialog->NA_core_table->setRowCount(mol.NumAtoms());
//        m_dialog->backbone_table->setTorizontalHeaderItem(0, new QTableWidgetItem("Atom Index"));
//        m_dialog->backbone_table->setTorizontalHeaderItem(1, new QTableWidgetItem("Element"));
        for (int i = 1; i < mol.NumAtoms()+1; i++) {
            a = mol.GetAtom(i);
            QTableWidgetItem *item = new QTableWidgetItem;
            item->setTextAlignment(Qt::AlignCenter);
            QTableWidgetItem *num = item->clone(), *element = item->clone();
            num->setData(Qt::EditRole, i);
            element->setData(Qt::EditRole, OpenBabel::etab.GetSymbol(a->GetAtomicNum()));
            m_dialog->NA_core_table->setItem(i-1, 0, num);
            m_dialog->NA_core_table->setItem(i-1, 1, element);
        }
        Molecule avogadro_mol, empty;
        avogadro_mol.setOBMol(&mol);
        base_mol = avogadro_mol;
        m_molecule->clear();
        emit performCommand(new InsertFragmentCommand(m_molecule, avogadro_mol, m_widget, tr("Insert Base")));
        focus = BaseFocus;
        m_dialog->connect_sbox_2->setMaximum(mol.NumAtoms());
        m_dialog->vector_sbox_2->setMaximum(mol.NumAtoms());
    }

    void ProtoBuilderExtension::moleculeUpdated() {
        focus = NoFocus;
    }

    void ProtoBuilderExtension::moleculeSelectionChangedCore() {
        if (focus != BaseFocus) {
            m_molecule->clear();
            emit performCommand(new InsertFragmentCommand(m_molecule, base_mol, m_widget, tr("Insert Base")));
            focus = BaseFocus;
        }
        QList<Primitive *> matchedPrimitives;
        QTableWidgetItem *item = m_dialog->NA_core_table->item(m_dialog->NA_core_table->currentRow(), 0);
        matchedPrimitives.append((Primitive*)m_molecule->atom(item->text().toInt() - 1));
        m_widget->clearSelected();
        m_widget->setSelected(matchedPrimitives, true);
        m_widget->update();
    }

    void ProtoBuilderExtension::clearTorsionExclusionList() {
        m_dialog->torsion_list->clear();
        excludeTorsionQString.clear();
    }

    void ProtoBuilderExtension::addTorsionExclusionList() {
        int atomIndex1, atomIndex2;
        atomIndex1 = m_dialog->torsion_exclusion_atom_index_1->value();
        atomIndex2 = m_dialog->torsion_exclusion_atom_index_2->value();
        excludeTorsionQString.append(tr("(%1,%2)\n").arg(atomIndex1).arg(atomIndex2));
        m_dialog->torsion_list->setText(excludeTorsionQString);
    }

    void ProtoBuilderExtension::importBackboneFile() {
        QString fileName = QFileDialog::getOpenFileName(m_dialog, tr("Open CML File"), "", tr("Molecule Files (*.cml)"));
        m_dialog->import_file_text->setText(fileName);
    }

    void ProtoBuilderExtension::loadBackboneFile() {
        OBConversion conv;
        conv.SetInFormat("CML");
        OBMol mol;
        if (!conv.ReadFile(&mol, m_dialog->import_file_text->text().toStdString())) {
            cerr << "Error reading backbone file." << endl;
            QMessageBox::critical(m_dialog, "Error", "There was an error loading the cml file. Please make sure you specified the file correctly.");
        }
        OBAtom *a;
        mol.Center();
        m_dialog->backbone_table->setRowCount(mol.NumAtoms());
//        m_dialog->backbone_table->setTorizontalHeaderItem(0, new QTableWidgetItem("Atom Index"));
//        m_dialog->backbone_table->setTorizontalHeaderItem(1, new QTableWidgetItem("Element"));
        for (int i = 1; i < mol.NumAtoms()+1; i++) {
            a = mol.GetAtom(i);
            QTableWidgetItem *item = new QTableWidgetItem;
            item->setTextAlignment(Qt::AlignCenter);
            QTableWidgetItem *num = item->clone(), *element = item->clone();
            num->setData(Qt::EditRole, i);
            element->setData(Qt::EditRole, OpenBabel::etab.GetSymbol(a->GetAtomicNum()));
            m_dialog->backbone_table->setItem(i-1, 0, num);
            m_dialog->backbone_table->setItem(i-1, 1, element);
        }
        Molecule avogadro_mol, empty;
        avogadro_mol.setOBMol(&mol);
        backbone_mol = avogadro_mol;
        m_molecule->clear();
        emit performCommand(new InsertFragmentCommand(m_molecule, avogadro_mol, m_widget, tr("Insert Backbone")));
        focus = BackboneFocus;
        m_dialog->connect_sbox->setMaximum(mol.NumAtoms());
        m_dialog->connect_sbox2->setMaximum(mol.NumAtoms());
        m_dialog->a_to_base_sbox->setMaximum(mol.NumAtoms());
        m_dialog->a_vector_sbox->setMaximum(mol.NumAtoms());
        m_dialog->torsion_exclusion_atom_index_1->setMaximum(mol.NumAtoms());
        m_dialog->torsion_exclusion_atom_index_2->setMaximum(mol.NumAtoms());
    }

    void ProtoBuilderExtension::moleculeSelectionChangedBackbone() {
        if (focus != BackboneFocus) {
            m_molecule->clear();
            emit performCommand(new InsertFragmentCommand(m_molecule, backbone_mol, m_widget, tr("Insert Backbone")));
            focus = BackboneFocus;
        }
        QList<Primitive *> matchedPrimitives;
        QTableWidgetItem *item = m_dialog->backbone_table->item(m_dialog->backbone_table->currentRow(), 0);
        matchedPrimitives.append((Primitive*)m_molecule->atom(item->text().toInt() - 1));
        m_widget->clearSelected();
        m_widget->setSelected(matchedPrimitives, true);
        m_widget->update();
    }

} // end namespace Avogadro

Q_EXPORT_PLUGIN2(ProtoBuilderExtension, Avogadro::ProtoBuilderExtensionFactory)