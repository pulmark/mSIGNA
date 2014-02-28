///////////////////////////////////////////////////////////////////////////////
//
// CoinVault
//
// scriptmodel.cpp
//
// Copyright (c) 2013 Eric Lombrozo
//
// All Rights Reserved.

#include "scriptmodel.h"

#include "settings.h"

#include <CoinQ_script.h>

#include <QStandardItemModel>

using namespace CoinDB;
using namespace CoinQ::Script;
using namespace std;

ScriptModel::ScriptModel(QObject* parent)
    : QStandardItemModel(parent)
{
    initColumns();
}

ScriptModel::ScriptModel(CoinDB::Vault* vault, const QString& accountName, QObject* parent)
    : QStandardItemModel(parent)
{
    initColumns();
    setVault(vault);
    setAccount(accountName);
}

void ScriptModel::initColumns()
{
    QStringList columns;
    columns << tr("Address") << tr("Type") << tr("Description");
    setHorizontalHeaderLabels(columns);
}

void ScriptModel::setVault(CoinDB::Vault* vault)
{
    this->vault = vault;
    accountName.clear();
}

void ScriptModel::setAccount(const QString& accountName)
{
    if (!vault) {
        throw std::runtime_error("Vault is not open.");
    }

    if (!vault->accountExists(accountName.toStdString())) {
        throw std::runtime_error("Account not found.");
    }

    this->accountName = accountName;
}

void ScriptModel::update()
{
    removeRows(0, rowCount());

    if (!vault || accountName.isEmpty()) return;

    std::vector<std::shared_ptr<SigningScriptView>> scripts = vault->getSigningScriptViews(accountName.toStdString(), SigningScript::CHANGE | SigningScript::REQUEST | SigningScript::RECEIPT);
    for (auto& script: scripts) {
        QList<QStandardItem*> row;
        QString address = QString::fromStdString(getAddressForTxOutScript(script->txoutscript, getDefaultSettings().getBase58Versions()));

        QString type;
        switch (script->status) {
        case SigningScript::CHANGE:
            type = tr("Change");
            break;

        case SigningScript::REQUEST:
            type = tr("Payment Requested");
            break;

        case SigningScript::RECEIPT:
            type = tr("Payment Received");
            break;

        default:
            type = tr("Unknown");
        }

        QString description = QString::fromStdString(script->label);

        row.append(new QStandardItem(address));
        row.append(new QStandardItem(type));
        row.append(new QStandardItem(description));

        appendRow(row);
    }    
}
