#include "sendcoinsentry.h"
#include "ui_sendcoinsentry.h"

#include "guiutil.h"
#include "bitcoinunits.h"
#include "addressbookpage.h"
#include "walletmodel.h"
#include "optionsmodel.h"
#include "addresstablemodel.h"

#include <QApplication>
#include <QClipboard>

SendCoinsEntry::SendCoinsEntry(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::SendCoinsEntry),
    model(0)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    ui->payToLayout->setSpacing(4);
#endif
#if QT_VERSION >= 0x040700
    /* Do not move this to the XML file, Qt before 4.7 will choke on it */
    ui->addAsLabel->setPlaceholderText(tr("Enter a label for this address to add it to your address book"));
    ui->payTo->setPlaceholderText(tr("Enter a Ghostprotocol address or name (e.g. N1KHAL5C1CRzy58NdJwp1tbLze3XrkFxx9 or id/ghostprotocol)"));
#endif
    setFocusPolicy(Qt::TabFocus);
    setFocusProxy(ui->payTo);
}

SendCoinsEntry::~SendCoinsEntry()
{
    delete ui;
}

void SendCoinsEntry::on_pasteButton_clicked()
{
    // Paste text from clipboard into recipient field
    ui->payTo->setText(QApplication::clipboard()->text());
}

void SendCoinsEntry::on_addressBookButton_clicked()
{
    if(!model)
        return;
    AddressBookPage dlg(AddressBookPage::ForSending, AddressBookPage::SendingTab, this);
    dlg.setModel(model->getAddressTableModel());
    if(dlg.exec())
    {
        ui->payTo->setText(dlg.getReturnValue());
        ui->payAmount->setFocus();
    }
}

void
SendCoinsEntry::on_payTo_textChanged(const QString& value)
{
    if(!model)
        return;

    /* Resolve recipient.  */
    const SendCoinsRecipient rv = getValue ();
    const QString address = rv.getAddress (*model);
    if (address == "")
      return;

    // Fill in label from address book, if address has an associated label
    QString associatedLabel = model->getAddressTableModel()->labelForAddress(address);
    if(!associatedLabel.isEmpty())
        ui->addAsLabel->setText(associatedLabel);
}

void SendCoinsEntry::setModel(WalletModel *model)
{
    this->model = model;

    if(model && model->getOptionsModel())
        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));

    clear();
}

void SendCoinsEntry::setRemoveEnabled(bool enabled)
{
    ui->deleteButton->setEnabled(enabled);
}

void SendCoinsEntry::clear()
{
    ui->payTo->clear();
    ui->addAsLabel->clear();
    ui->payAmount->clear();
    ui->payTo->setFocus();
    // update the display unit, to not use the default ("BTC")
    updateDisplayUnit();
}

void SendCoinsEntry::on_deleteButton_clicked()
{
    emit removeEntry(this);
}

bool SendCoinsEntry::validate()
{
    // Check input validity
    bool retval = true;

    if(!ui->payAmount->validate())
    {
        retval = false;
    }
    else
    {
        if(ui->payAmount->value() <= 0)
        {
            // Cannot send 0 coins or less
            ui->payAmount->setValid(false);
            retval = false;
        }
    }

    const SendCoinsRecipient rv = getValue ();
    if (rv.getAddress (*model) == "")
      {
        ui->payTo->setValid(false);
        retval = false;
      }

    return retval;
}

SendCoinsRecipient SendCoinsEntry::getValue()
{
    SendCoinsRecipient rv;

    rv.recipient = ui->payTo->text();
    rv.label = ui->addAsLabel->text();
    rv.amount = ui->payAmount->value();

    return rv;
}

QWidget *SendCoinsEntry::setupTabChain(QWidget *prev)
{
    QWidget::setTabOrder(prev, ui->payTo);
    QWidget::setTabOrder(ui->payTo, ui->addressBookButton);
    QWidget::setTabOrder(ui->addressBookButton, ui->pasteButton);
    QWidget::setTabOrder(ui->pasteButton, ui->deleteButton);
    QWidget::setTabOrder(ui->deleteButton, ui->addAsLabel);
    return ui->payAmount->setupTabChain(ui->addAsLabel);
}

void SendCoinsEntry::setValue(const SendCoinsRecipient &value)
{
    ui->payTo->setText(value.recipient);
    ui->addAsLabel->setText(value.label);
    ui->payAmount->setValue(value.amount);
}

void SendCoinsEntry::setAddress(const QString &address)
{
    ui->payTo->setText(address);
    ui->payAmount->setFocus();
}

bool SendCoinsEntry::isClear()
{
    return ui->payTo->text().isEmpty();
}

void SendCoinsEntry::setFocus()
{
    ui->payTo->setFocus();
}

void SendCoinsEntry::updateDisplayUnit()
{
    if(model && model->getOptionsModel())
    {
        // Update payAmount with the current unit
        ui->payAmount->setDisplayUnit(model->getOptionsModel()->getDisplayUnit());
    }
}
