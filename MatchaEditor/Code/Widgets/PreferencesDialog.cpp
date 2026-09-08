#include "PreferencesDialog.h"

#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QSettings>
#include <QFileDialog>

namespace MatchaEditor
{
PreferencesDialog::PreferencesDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Preferences");
    setMinimumWidth(500);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Label and Path Row
    QLabel* label = new QLabel("External Script Editor Executable:", this);
    mainLayout->addWidget(label);

    QHBoxLayout* pathLayout = new QHBoxLayout();
    m_EditorPathInput = new QLineEdit(this);
    m_EditorPathInput->setPlaceholderText("Leave empty to use OS default handler");

    QPushButton* browseButton = new QPushButton("Browse...", this);

    pathLayout->addWidget(m_EditorPathInput);
    pathLayout->addWidget(browseButton);
    mainLayout->addLayout(pathLayout);

    // Load saved settings
    QSettings settings("MatchaEditor");
    QString currentPath = settings.value("ExternalEditor/Path", "").toString();
    m_EditorPathInput->setText(currentPath);

    // Dialog Buttons (OK / Cancel)
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox);

    // Wire up events
    connect(browseButton, &QPushButton::clicked, this, &PreferencesDialog::OnBrowseClicked);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &PreferencesDialog::OnAccepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void PreferencesDialog::OnBrowseClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Select External Script Editor Executable", QString(), "Executables (*.exe);;All Files (*)");
    if (!filePath.isEmpty())
    {
        m_EditorPathInput->setText(filePath);
    }
}

void PreferencesDialog::OnAccepted()
{
    // Save the path to settings
    QSettings settings("MatchaEditor");
    settings.setValue("ExternalEditor/Path", m_EditorPathInput->text());

    accept();
}
}  // namespace MatchaEditor