#include <QtGui>
#include "window_deckeditor.h"
#include "carddatabase.h"
#include "carddatabasemodel.h"
#include "decklistmodel.h"
#include "cardinfowidget.h"

WndDeckEditor::WndDeckEditor(CardDatabase *_db, QWidget *parent)
	: QMainWindow(parent), db(_db)
{
	QLabel *searchLabel = new QLabel(tr("&Search for:"));
	searchEdit = new QLineEdit;
	searchLabel->setBuddy(searchEdit);
	connect(searchEdit, SIGNAL(textChanged(const QString &)), this, SLOT(updateSearch(const QString &)));
	connect(searchEdit, SIGNAL(returnPressed()), this, SLOT(actAddCard()));

	QHBoxLayout *searchLayout = new QHBoxLayout;
	searchLayout->addWidget(searchLabel);
	searchLayout->addWidget(searchEdit);

	databaseModel = new CardDatabaseModel(db);
	databaseView = new QTreeView();
	databaseView->setModel(databaseModel);
	databaseView->setUniformRowHeights(true);
	databaseView->setSortingEnabled(true);
	connect(databaseView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(updateCardInfoLeft(const QModelIndex &, const QModelIndex &)));
	connect(databaseView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(actAddCard()));

	QVBoxLayout *leftFrame = new QVBoxLayout;
	leftFrame->addLayout(searchLayout);
	leftFrame->addWidget(databaseView);

	cardInfo = new CardInfoWidget(db);
	cardInfo->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

	QToolBar *verticalToolBar = new QToolBar;
	verticalToolBar->setOrientation(Qt::Vertical);
	QHBoxLayout *verticalToolBarLayout = new QHBoxLayout;
	verticalToolBarLayout->addStretch();
	verticalToolBarLayout->addWidget(verticalToolBar);
	verticalToolBarLayout->addStretch();

	QVBoxLayout *middleFrame = new QVBoxLayout;
	middleFrame->addWidget(cardInfo);
	middleFrame->addLayout(verticalToolBarLayout);
	middleFrame->addStretch();

	deckModel = new DeckListModel(db, this);
	deckView = new QTreeView();
	deckView->setModel(deckModel);
	deckView->setUniformRowHeights(true);
	deckView->setRootIsDecorated(false);
	connect(deckView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(updateCardInfoRight(const QModelIndex &, const QModelIndex &)));

	QLabel *nameLabel = new QLabel(tr("Deck &name:"));
	nameEdit = new QLineEdit;
	nameLabel->setBuddy(nameEdit);
	connect(nameEdit, SIGNAL(textChanged(const QString &)), deckModel->getDeckList(), SLOT(setName(const QString &)));
	QLabel *commentsLabel = new QLabel(tr("&Comments:"));
	commentsEdit = new QLineEdit;
	commentsLabel->setBuddy(commentsEdit);
	connect(commentsEdit, SIGNAL(textChanged(const QString &)), deckModel->getDeckList(), SLOT(setComments(const QString &)));
	QGridLayout *grid = new QGridLayout;
	grid->addWidget(nameLabel, 0, 0);
	grid->addWidget(nameEdit, 0, 1);
	grid->addWidget(commentsLabel, 1, 0);
	grid->addWidget(commentsEdit, 1, 1);

	QVBoxLayout *rightFrame = new QVBoxLayout;
	rightFrame->addLayout(grid);
	rightFrame->addWidget(deckView);

	QHBoxLayout *mainLayout = new QHBoxLayout;
	mainLayout->addLayout(leftFrame, 10);
	mainLayout->addLayout(middleFrame);
	mainLayout->addLayout(rightFrame, 10);

	QWidget *centralWidget = new QWidget;
	centralWidget->setLayout(mainLayout);
	setCentralWidget(centralWidget);

	setWindowTitle(tr("Card database"));

	aNewDeck = new QAction(tr("&New deck"), this);
	connect(aNewDeck, SIGNAL(triggered()), this, SLOT(actNewDeck()));
	aLoadDeck = new QAction(tr("&Load deck..."), this);
	aLoadDeck->setShortcut(tr("Ctrl+L"));
	connect(aLoadDeck, SIGNAL(triggered()), this, SLOT(actLoadDeck()));
	aSaveDeck = new QAction(tr("&Save deck"), this);
	aSaveDeck->setShortcut(tr("Ctrl+S"));
	connect(aSaveDeck, SIGNAL(triggered()), this, SLOT(actSaveDeck()));
	aSaveDeckAs = new QAction(tr("&Save deck as..."), this);
	connect(aSaveDeckAs, SIGNAL(triggered()), this, SLOT(actSaveDeckAs()));

	deckMenu = menuBar()->addMenu(tr("&Deck"));
	deckMenu->addAction(aNewDeck);
	deckMenu->addAction(aLoadDeck);
	deckMenu->addAction(aSaveDeck);
	deckMenu->addAction(aSaveDeckAs);

	aAddCard = new QAction(tr("Add card to &maindeck"), this);
	connect(aAddCard, SIGNAL(triggered()), this, SLOT(actAddCard()));
	aAddCard->setShortcut(tr("Ctrl+M"));
	aAddCardToSideboard = new QAction(tr("Add card to &sideboard"), this);
	connect(aAddCardToSideboard, SIGNAL(triggered()), this, SLOT(actAddCardToSideboard()));
	aAddCardToSideboard->setShortcut(tr("Ctrl+N"));
	aRemoveCard = new QAction(tr("&Remove row"), this);
	connect(aRemoveCard, SIGNAL(triggered()), this, SLOT(actRemoveCard()));
	aRemoveCard->setShortcut(tr("Ctrl+R"));
	aIncrement = new QAction(tr("&Increment number"), this);
	connect(aIncrement, SIGNAL(triggered()), this, SLOT(actIncrement()));
	aIncrement->setShortcut(tr("+"));
	aDecrement = new QAction(tr("&Decrement number"), this);
	connect(aDecrement, SIGNAL(triggered()), this, SLOT(actDecrement()));
	aDecrement->setShortcut(tr("-"));

	verticalToolBar->addAction(aAddCard);
	verticalToolBar->addAction(aAddCardToSideboard);
	verticalToolBar->addAction(aRemoveCard);
	verticalToolBar->addAction(aIncrement);
	verticalToolBar->addAction(aDecrement);
}

WndDeckEditor::~WndDeckEditor()
{

}

void WndDeckEditor::updateCardInfoLeft(const QModelIndex &current, const QModelIndex &/*previous*/)
{
	cardInfo->setCard(current.sibling(current.row(), 0).data().toString());
}

void WndDeckEditor::updateCardInfoRight(const QModelIndex &current, const QModelIndex &/*previous*/)
{
	cardInfo->setCard(current.sibling(current.row(), 1).data().toString());
}

void WndDeckEditor::updateSearch(const QString &search)
{
	QModelIndexList matches = databaseModel->match(databaseModel->index(0, 0), Qt::DisplayRole, search);
	if (matches.isEmpty())
		return;
	databaseView->selectionModel()->setCurrentIndex(matches[0], QItemSelectionModel::SelectCurrent);
}

void WndDeckEditor::actNewDeck()
{
	deckModel->cleanList();
	nameEdit->setText(QString());
	commentsEdit->setText(QString());
	lastFileName = QString();
}

void WndDeckEditor::actLoadDeck()
{
	DeckList *l = deckModel->getDeckList();
	if (l->loadDialog(this)) {
		lastFileName = l->getLastFileName();
		lastFileFormat = l->getLastFileFormat();
		nameEdit->setText(l->getName());
		commentsEdit->setText(l->getComments());
		deckView->expandAll();
	}
}

void WndDeckEditor::actSaveDeck()
{
	if (lastFileName.isEmpty())
		actSaveDeckAs();
	else
		deckModel->getDeckList()->saveToFile(lastFileName, lastFileFormat);
;
}

void WndDeckEditor::actSaveDeckAs()
{
	DeckList *l = deckModel->getDeckList();
	if (l->saveDialog(this)) {
		lastFileName = l->getLastFileName();
		lastFileFormat = l->getLastFileFormat();
	}
}

void WndDeckEditor::addCardHelper(int baseRow)
{
	const QModelIndex currentIndex = databaseView->selectionModel()->currentIndex();
	if (!currentIndex.isValid())
		return;
	const QString cardName = databaseModel->index(currentIndex.row(), 0).data().toString();
	QModelIndex zoneRoot = deckModel->index(baseRow, 0);
	deckView->expand(zoneRoot);
	QModelIndexList matches = deckModel->match(deckModel->index(0, 1, zoneRoot), Qt::EditRole, cardName);
	if (matches.isEmpty()) {
		int row = deckModel->rowCount(zoneRoot);
		deckModel->insertRow(row, zoneRoot);
		deckModel->setData(deckModel->index(row, 1, zoneRoot), cardName, Qt::EditRole);
	} else {
		const QModelIndex numberIndex = deckModel->index(matches[0].row(), 0, zoneRoot);
		const int count = deckModel->data(numberIndex, Qt::EditRole).toInt();
		deckModel->setData(numberIndex, count + 1, Qt::EditRole);
	}
}

void WndDeckEditor::actAddCard()
{
	addCardHelper(0);
}

void WndDeckEditor::actAddCardToSideboard()
{
	addCardHelper(1);
}

void WndDeckEditor::actRemoveCard()
{
	const QModelIndex currentIndex = deckView->selectionModel()->currentIndex();
	if (!currentIndex.isValid())
		return;
	deckModel->removeRow(currentIndex.row(), currentIndex.parent());
}

void WndDeckEditor::actIncrement()
{
	const QModelIndex currentIndex = deckView->selectionModel()->currentIndex();
	if (!currentIndex.isValid())
		return;
	const QModelIndex numberIndex = currentIndex.sibling(currentIndex.row(), 0);
	const int count = deckModel->data(numberIndex, Qt::EditRole).toInt();
	deckModel->setData(numberIndex, count + 1, Qt::EditRole);
}

void WndDeckEditor::actDecrement()
{
	const QModelIndex currentIndex = deckView->selectionModel()->currentIndex();
	if (!currentIndex.isValid())
		return;
	const QModelIndex numberIndex = currentIndex.sibling(currentIndex.row(), 0);
	const int count = deckModel->data(numberIndex, Qt::EditRole).toInt();
	if (count == 1)
		deckModel->removeRow(currentIndex.row(), currentIndex.parent());
	else
		deckModel->setData(numberIndex, count - 1, Qt::EditRole);
}