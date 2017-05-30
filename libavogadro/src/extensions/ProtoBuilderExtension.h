//
// Created by jbarnett8 on 5/21/17.
//

#ifndef AVOGADRO_PROTOBUILDEREXTENSION_H
#define AVOGADRO_PROTOBUILDEREXTENSION_H

#include <avogadro/extension.h>
#include <openbabel/obconversion.h>
#include <QTableWidgetItem>

namespace Avogadro {

    class ProtoBuilderDialog;
    class ProtoBuilderExtension : public Extension
    {
    Q_OBJECT
    AVOGADRO_EXTENSION("ProtoBuilder", tr("Proto-Nucleic-Acid Builder"),
                       tr("Proto-Nucleic-Acid Builder"))

    public:
        //! Constructor
        ProtoBuilderExtension(QObject *parent=0);
        //! Destructor
        ~ProtoBuilderExtension();

        //! Perform Action
        QList<QAction *> actions() const;
        QUndoCommand* performAction(QAction *action, GLWidget *widget);
        QString menuPath(QAction *action) const;
        void setMolecule(Molecule *molecule);

        void writeSettings(QSettings &settings) const;
        void readSettings(QSettings &settings);

    public Q_SLOTS:
        void performInsert();

        void updateText();
        void updateBPTurns(int type);
        void changeNucleicType(int type);

        void dialogDestroyed();
        void importCoreFile();
        void loadCoreFile();

    private:
        QList<QAction *> m_actions;
        GLWidget* m_widget;
        Molecule *m_molecule;

        ProtoBuilderDialog *m_dialog;

        void constructDialog();
    };

    class ProtoBuilderExtensionFactory : public QObject, public PluginFactory
    {
    Q_OBJECT
        Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_EXTENSION_FACTORY(ProtoBuilderExtension)
    };

} // end namespace Avogadro

#endif //AVOGADRO_PROTOBUILDEREXTENSION_H
