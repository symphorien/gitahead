//
//          Copyright (c) 2016, Scientific Toolworks, Inc.
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//
// Author: Jason Haslam
//

#include "TagDialog.h"
#include "git/Repository.h"
#include "git/TagRef.h"
#include "ui/ExpandButton.h"
#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>

TagDialog::TagDialog(
  const git::Repository &repo,
  const QString &id,
  QWidget *parent)
  : QDialog(parent)
{
  setAttribute(Qt::WA_DeleteOnClose);

  setWindowTitle(tr("Create Tag"));
  QString text = tr("Add a new tag at %1").arg(id);
  QLabel *label = new QLabel(QString("<b>%1:</b>").arg(text), this);

  mNameField = new QLineEdit(this);
  mForce = new QCheckBox(tr("Force (replace existing tag)"), this);

  QCheckBox *annotated = new QCheckBox(tr("Annotated"), this);
  ExpandButton *expand = new ExpandButton(this);
  connect(annotated, &QCheckBox::toggled, expand, &ExpandButton::setChecked);

  QHBoxLayout *annotatedLayout = new QHBoxLayout;
  annotatedLayout->addWidget(annotated);
  annotatedLayout->addWidget(expand);
  annotatedLayout->addStretch();

  mMessage = new QTextEdit(this);
  mMessage->setEnabled(false);
  mMessage->setVisible(false);
  connect(annotated, &QCheckBox::toggled, mMessage, &QTextEdit::setEnabled);
  connect(expand, &ExpandButton::toggled, [this](bool checked) {
    mMessage->setVisible(checked);
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    resize(minimumSizeHint());
  });

  QDialogButtonBox *buttons =
    new QDialogButtonBox(QDialogButtonBox::Cancel, this);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

  QPushButton *create =
    buttons->addButton(tr("Create Tag"), QDialogButtonBox::AcceptRole);
  create->setEnabled(false);

  auto updateButton = [this, repo, create, annotated] {
    QString name = mNameField->text();
    bool force = mForce->isChecked();
    create->setEnabled(
      !name.isEmpty() && (force || !repo.lookupTag(name).isValid()) &&
      (!annotated->isChecked() || !mMessage->toPlainText().isEmpty()));
  };

  connect(mNameField, &QLineEdit::textChanged, updateButton);
  connect(mForce, &QCheckBox::toggled, updateButton);
  connect(annotated, &QCheckBox::toggled, updateButton);
  connect(mMessage, &QTextEdit::textChanged, updateButton);

  QFormLayout *layout = new QFormLayout(this);
  layout->addRow(label);
  layout->addRow(tr("Name"), mNameField);
  layout->addRow(mForce);
  layout->addRow(annotatedLayout);
  layout->addRow(mMessage);
  layout->addRow(buttons);
}

bool TagDialog::force() const
{
  return mForce->isChecked();
}

QString TagDialog::name() const
{
  return mNameField->text();
}

QString TagDialog::message() const
{
  return mMessage->toPlainText();
}
