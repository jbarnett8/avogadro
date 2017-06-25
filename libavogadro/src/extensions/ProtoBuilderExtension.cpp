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

#include <string>

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
        focus = NoFocus;
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
            connect(m_dialog->save_input_file_button, SIGNAL(clicked()),
                    this, SLOT(saveDataFile()));

            connect(m_dialog->import_conformers_button, SIGNAL(clicked()),
                    this, SLOT(importResultsFile()));
            connect(m_dialog->load_conformers_button, SIGNAL(clicked()),
                    this, SLOT(loadResults()));
            connect(m_dialog->conformer_table, SIGNAL(itemSelectionChanged()),
                    this, SLOT(ConformerSelectionChanged()));
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
//        m_dialog->NA_core_table->setHorizontalHeaderItem(0, new QTableWidgetItem("Atom Index"));
//        m_dialog->NA_core_table->setHorizontalHeaderItem(1, new QTableWidgetItem("Element"));
        QTableWidgetItem *item = new QTableWidgetItem;
        item->setTextAlignment(Qt::AlignCenter);
        for (unsigned long i = 1; i < mol.NumAtoms()+1; i++) {
            a = mol.GetAtom(i);
            QTableWidgetItem *num = item->clone(), *element = item->clone();
            num->setData(Qt::EditRole, (int)i);
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
//        m_dialog->backbone_table->setHorizontalHeaderItem(0, new QTableWidgetItem("Atom Index"));
//        m_dialog->backbone_table->setHorizontalHeaderItem(1, new QTableWidgetItem("Element"));
        QTableWidgetItem *item = new QTableWidgetItem;
        item->setTextAlignment(Qt::AlignCenter);
        for (unsigned long i = 1; i < mol.NumAtoms()+1; i++) {
            a = mol.GetAtom(i);
            QTableWidgetItem *num = item->clone(), *element = item->clone();
            num->setData(Qt::EditRole, (int)i);
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

    void ProtoBuilderExtension::loadResults() {
        ifstream file(m_dialog->import_file_text_3->text().toStdString().c_str());
        string line, val, path;
        int row = -1, column = 0;
        QStringList headerList;
        OBConversion conv;
        conv.SetInFormat("CML");
        vector<string> headers;
        getline(file, line, '\n');
        stringstream lineStream(line);
        OBMol tempMol;
        double *coord;
        Eigen::Vector3d xyz;
        vector<Eigen::Vector3d> *coordE;

        path = m_dialog->import_file_text_3->text().toStdString();
        path = path.substr(0, path.find_last_of("/\\") + 1);

        m_dialog->conformer_table->setRowCount(5);
        m_dialog->conformer_table->setColumnCount(9);

        while (getline(lineStream,val,',')) {
            headers.push_back(val.substr(val.find_first_not_of("\t\n ")));
            headerList << tr(headers.back().c_str());
        }
        m_dialog->conformer_table->setColumnCount(headers.size());
        QTableWidgetItem *item = new QTableWidgetItem;
        item->setTextAlignment(Qt::AlignLeft);
        while (getline(file,line,'\n')) {
            lineStream.clear();
            lineStream.str(line);
            row++;
            column = 0;
            while (getline(lineStream, val, ',')) {
                QTableWidgetItem *num = item->clone();
                if (column == 0) {
                    num->setData(Qt::EditRole, atoi(val.c_str()));
                    if (!conv.ReadFile(&tempMol, path + "conformer" + val + ".cml")) {
                        cerr << "Error reading backbone file." << endl;
                        QMessageBox::critical(m_dialog, "Error", "There was an error loading the cml file. Please make sure you specified the file correctly.");
                        break;
                    }
                    coord = tempMol.GetCoordinates();

                    coordE = new vector<Eigen::Vector3d>;
                    for (unsigned i = 0; i < tempMol.NumAtoms()*3; i+=3) {
                        xyz << coord[i], coord[i + 1], coord[i + 2];
                        coordE->push_back(xyz);
                    }
                    coords.push_back(coordE);
                } else {
                    num->setData(Qt::EditRole, atof(val.c_str()));
                }
                m_dialog->conformer_table->setItem(row, column, num);
                column++;
            }
        }
        m_dialog->conformer_table->setHorizontalHeaderLabels(headerList);
//        m_dialog->conformer_table->setRowCount(row);

        if (!tempMol.Empty()) {
            conformer_mol.setOBMol(&tempMol);
            m_molecule->clear();
            emit performCommand(new InsertFragmentCommand(m_molecule, conformer_mol, m_widget, tr("Insert Conformers")));
            m_molecule->setAllConformers(coords);
            m_widget->clearSelected();
            focus = ConformerFocus;
        }
    }

    void ProtoBuilderExtension::ConformerSelectionChanged() {
        if (focus != ConformerFocus) {
            m_molecule->clear();
            emit performCommand(new InsertFragmentCommand(m_molecule, conformer_mol, m_widget, tr("Insert Conformers")));
            m_widget->clearSelected();
            m_molecule->setAllConformers(coords);
            focus = ConformerFocus;
            m_widget->update();
        }
        m_molecule->setConformer(m_dialog->conformer_table->currentRow());
        m_widget->update();
    }

    void ProtoBuilderExtension::importResultsFile() {
        QString fileName = QFileDialog::getOpenFileName(m_dialog, tr("Open CSV File"), "", tr("Conformer Energy Data File (*.csv)"));
        m_dialog->import_file_text_3->setText(fileName);
    }

    void ProtoBuilderExtension::saveDataFile() {
        validate();
        ofstream data;
        QString fileName = QFileDialog::getSaveFileName(m_dialog, tr("Save Data File"), "", tr("Data Input File (*.dat)"));
        data.open(fileName.toStdString().c_str());

        data << "BACKBONE" << endl;
        data << "\tFILE=" << m_dialog->import_file_text->text().toStdString() << endl;
        data << "\tLINKERS=(" << m_dialog->connect_sbox->value() << "," << m_dialog->connect_sbox2->value() << ")" << endl;
        data << "\tBASE=" << m_dialog->a_to_base_sbox->text().toStdString() << endl;
        data << "\tVECTOR=" << m_dialog->a_vector_sbox->text().toStdString() << endl;
        if (!m_dialog->torsion_list->toPlainText().isEmpty())
            data << "\tEXCLUDE_TORSIONS=" << m_dialog->torsion_list->toPlainText().replace("\n",",").toStdString() << endl;

        data << "\nNA_CORE" << endl;
        data << "\tFILE=" << m_dialog->import_file_text_2->text().toStdString() << endl;
        data << "\tCONNECT=" << m_dialog->connect_sbox_2->value() << endl;
        data << "\tVECTOR=" << m_dialog->vector_sbox_2->value() << endl;
        data << "\tSYMMETRY=" << m_dialog->symmetry_sbox_2->value() << endl;
        data << "\tNAME=" << m_dialog->core_name->text().toStdString() << endl;
        data << "\tCODE=" << m_dialog->base_code_text_2->text().toStdString() << endl;
        data << "\tPAIR=" << m_dialog->base_pair_code_text_2->text().toStdString() << endl;

        data << "\nFORCEFIELD" << endl;
        data << "\tTYPE=" << m_dialog->comboBox->currentText().toStdString() << endl;
        data << "\tPARAMETER_FILE=" << m_dialog->parameter_file_text->text().toStdString() << endl;
        if (m_dialog->base_to_back_bond_length_sbox->value() > 0.00)
            cout << "BASE_TO_BACK_BOND_LENGTH=" << m_dialog->base_to_back_bond_length_sbox->value() << endl;

        data << "\nMISC" << endl;

        data << "\nHELICAL_PARAMS" << endl;
        data << "\tRISE=" << m_dialog->rise_sbox->value() << endl;
        data << "\tTWIST=" << m_dialog->twist_sbox->value() << endl;
        data << "\tX_DISP=" << m_dialog->x_disp_sbox->value() << endl;
        data << "\tY_DISP=" << m_dialog->y_disp_sbox->value() << endl;
        data << "\tINCLINE=" << m_dialog->incline_sbox->value() << endl;

        data << "\nRUN" << endl;
        data << "\tALGORITHM=";
        QString algorithm = m_dialog->algorithm_comboBox->currentText();
        if (algorithm.contains("Systematic Search"))
            data << "SSS";
        else if (algorithm.contains("Normal Random Search"))
            data << "NRS";
        else if (algorithm.contains("Weighted Random Search"))
            data << "WRS";
        else if (algorithm.contains("Monte Carlo Search"))
            data << "MCS";
        else if (algorithm.contains("Weighted Monte Carlo"))
            data << "WMC";
        data << endl;
        data << "\tNUM_STEPS=" << m_dialog->num_steps_text->text().toStdString() << endl;
        data << "\tDIST_TOL=" << m_dialog->dist_tol_sbox->text().toStdString() << endl;
        data << "\tTOTENERGY_TOL=" << m_dialog->tot_energy_tol_text->text().toStdString() << endl;
        data << "\tBOND_TOL=" << m_dialog->bond_energy_tol_text->text().toStdString() << endl;
        data << "\tANGLE_TOL=" << m_dialog->angle_energy_tol_text->text().toStdString() << endl;
        data << "\tDIHEDRAL_TOL=" << m_dialog->dihedral_energy_tol_text->text().toStdString() << endl;
        data << "\tVDW_TOL=" << m_dialog->vdw_energy_tol_text->text().toStdString() << endl;
        data << "\tDIHEDRAL_DISCRETIZATION=" << m_dialog->dihedral_discretization_text->text().toStdString() << endl;
        data << "\tANGLE_STEP_SIZE=" << m_dialog->angle_step_size_text->text().toStdString() << endl;
        data << "\tCHAIN_LENGTH=" << m_dialog->chain_length_sbox->value() << endl;
        data.close();
    }

    void ProtoBuilderExtension::validate() {

    }

} // end namespace Avogadro

Q_EXPORT_PLUGIN2(ProtoBuilderExtension, Avogadro::ProtoBuilderExtensionFactory)