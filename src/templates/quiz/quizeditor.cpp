/*
  Copyright (c) 2012, BuildmLearn Contributors listed at http://buildmlearn.org/people/
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * Neither the name of the BuildmLearn nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "templates/quiz/quizeditor.h"

#include "miscellaneous/iconfactory.h"
#include "templates/quiz/quizquestion.h"
#include "core/templatefactory.h"
#include "core/templatecore.h"
#include "core/templateentrypoint.h"

#include <QToolTip>
#include <QTimer>
#include <QShowEvent>
#include <QDomDocument>
#include <QDomElement>
#include <QDomAttr>


QuizEditor::QuizEditor(TemplateCore *core, QWidget *parent)
  : TemplateEditor(core, parent), m_firstShow(true), m_ui(new Ui::QuizEditor) {
  m_ui->setupUi(this);

  m_ui->m_txtNumberOfQuestions->lineEdit()->setEnabled(false);

  IconFactory *factory = IconFactory::instance();

  m_ui->m_txtAuthor->lineEdit()->setPlaceholderText(tr("Author of this quiz"));
  m_ui->m_txtName->lineEdit()->setPlaceholderText(tr("Name of this quiz"));

  m_ui->m_btnAnswerOne->setProperty("id", 1);
  m_ui->m_btnAnswerTwo->setProperty("id", 2);
  m_ui->m_btnAnswerThree->setProperty("id", 3);
  m_ui->m_btnAnswerFour->setProperty("id", 4);

  m_ui->m_btnQuestionAdd->setIcon(factory->fromTheme("item-add"));
  m_ui->m_btnQuestionRemove->setIcon(factory->fromTheme("item-remove"));
  m_ui->m_btnQuestionUp->setIcon(factory->fromTheme("move-up"));
  m_ui->m_btnQuestionDown->setIcon(factory->fromTheme("move-down"));

  m_iconNo = factory->fromTheme("dialog-no");
  m_iconYes = factory->fromTheme("dialog-yes");

  m_ui->m_btnAnswerOne->setIcon(m_iconNo);
  m_ui->m_btnAnswerTwo->setIcon(m_iconNo);
  m_ui->m_btnAnswerThree->setIcon(m_iconNo);
  m_ui->m_btnAnswerFour->setIcon(m_iconNo);

  connect(m_ui->m_txtAuthor->lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(updateAuthorStatus()));
  connect(m_ui->m_txtName->lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(updateNameStatus()));

  connect(m_ui->m_btnQuestionAdd, SIGNAL(clicked()), this, SLOT(addQuestion()));
  connect(m_ui->m_btnQuestionRemove, SIGNAL(clicked()), this, SLOT(removeQuestion()));
  connect(m_ui->m_listQuestions, SIGNAL(currentRowChanged(int)), this, SLOT(loadQuestion(int)));

  connect(m_ui->m_btnAnswerOne, SIGNAL(clicked()), this, SLOT(saveQuestion()));
  connect(m_ui->m_btnAnswerTwo, SIGNAL(clicked()), this, SLOT(saveQuestion()));
  connect(m_ui->m_btnAnswerThree, SIGNAL(clicked()), this, SLOT(saveQuestion()));
  connect(m_ui->m_btnAnswerFour, SIGNAL(clicked()), this, SLOT(saveQuestion()));

  connect(m_ui->m_txtAnswerOne, SIGNAL(textEdited(QString)), this, SLOT(saveQuestion()));
  connect(m_ui->m_txtAnswerTwo, SIGNAL(textEdited(QString)), this, SLOT(saveQuestion()));
  connect(m_ui->m_txtAnswerThree, SIGNAL(textEdited(QString)), this, SLOT(saveQuestion()));
  connect(m_ui->m_txtAnswerFour, SIGNAL(textEdited(QString)), this, SLOT(saveQuestion()));
  connect(m_ui->m_txtQuestion, SIGNAL(textChanged()), this, SLOT(saveQuestion()));

  connect(m_ui->m_btnQuestionUp, SIGNAL(clicked()), this, SLOT(moveQuestionUp()));
  connect(m_ui->m_btnQuestionDown, SIGNAL(clicked()), this, SLOT(moveQuestionDown()));

  setEditorsEnabled(false);
  updateQuestionCount();

  m_ui->m_txtAuthor->lineEdit()->setText(tr("John Doe"));
  m_ui->m_txtName->lineEdit()->setText(tr("Greatest quiz"));

  qRegisterMetaType<QuizQuestion>("QuizQuestion");
}

QuizEditor::~QuizEditor() {
  qDebug("Destroying QuizEditor instance.");

  delete m_ui;
}

QList<QuizQuestion> QuizEditor::activeQuestions() const {
  QList<QuizQuestion> questions;

  for (int i = 0; i < m_ui->m_listQuestions->count(); i++) {
    questions.append(m_ui->m_listQuestions->item(i)->data(Qt::UserRole).value<QuizQuestion>());
  }

  return questions;
}

void QuizEditor::showEvent(QShowEvent *e) {
  e->accept();

  if (m_firstShow) {
    m_firstShow = false;

    QToolTip::showText(m_ui->m_btnQuestionAdd->mapToGlobal(QPoint(5, 0)),
                       tr("Add new question by clicking here."),
                       m_ui->m_btnQuestionAdd);
  }
}

void QuizEditor::updateQuestionCount() {
  m_ui->m_txtNumberOfQuestions->lineEdit()->setText(QString::number(m_ui->m_listQuestions->count()));

  if (m_ui->m_listQuestions->count() > 0) {
    m_ui->m_txtNumberOfQuestions->setStatus(WidgetWithStatus::Ok, tr("Quiz contains at least one question."));
  }
  else {
    m_ui->m_txtNumberOfQuestions->setStatus(WidgetWithStatus::Error, tr("Quiz does not contain any questions."));
  }
}

void QuizEditor::addQuestion() {
  int marked_question = m_ui->m_listQuestions->currentRow();
  QuizQuestion new_question;
  QListWidgetItem *new_item = new QListWidgetItem();

  new_question.setQuestion(tr("How many cats do you have?"));
  new_question.setCorrectAnswer(3);
  new_question.setAnswerOne(tr("I hate cats!"));
  new_question.setAnswerTwo(tr("I have two nice kittens."));
  new_question.setAnswerThree(tr("I have seven beasts."));
  new_question.setAnswerFour(tr("Cats? Well, we own eleven dogs."));

  new_item->setText(new_question.question());
  new_item->setData(Qt::UserRole, QVariant::fromValue(new_question));

  if (m_ui->m_listQuestions->count() == 0) {
    // We are adding first question.
    setEditorsEnabled(true);

    m_ui->m_btnQuestionRemove->setEnabled(true);

    m_ui->m_listQuestions->insertItem(0, new_item);
    m_ui->m_listQuestions->setCurrentRow(0);
  }
  else {
    m_ui->m_listQuestions->insertItem(marked_question + 1, new_item);
    m_ui->m_listQuestions->setCurrentRow(marked_question + 1);
  }

  updateQuestionCount();
  launch();
  emit changed();
}

void QuizEditor::loadQuestion(int index) {
  m_ui->m_txtQuestion->blockSignals(true);
  m_ui->m_txtAnswerOne->blockSignals(true);
  m_ui->m_txtAnswerTwo->blockSignals(true);
  m_ui->m_txtAnswerThree->blockSignals(true);
  m_ui->m_txtAnswerFour->blockSignals(true);
  m_ui->m_btnAnswerOne->blockSignals(true);
  m_ui->m_btnAnswerTwo->blockSignals(true);
  m_ui->m_btnAnswerThree->blockSignals(true);
  m_ui->m_btnAnswerFour->blockSignals(true);

  if (index >= 0) {
    QuizQuestion question = m_ui->m_listQuestions->item(index)->data(Qt::UserRole).value<QuizQuestion>();

    m_ui->m_txtQuestion->setText(question.question());
    m_ui->m_txtAnswerOne->setText(question.answerOne());
    m_ui->m_txtAnswerTwo->setText(question.answerTwo());
    m_ui->m_txtAnswerThree->setText(question.answerThree());
    m_ui->m_txtAnswerFour->setText(question.answerFour());
    m_ui->m_btnAnswerOne->setIcon(question.correctAnswer() == 1 ? m_iconYes : m_iconNo);
    m_ui->m_btnAnswerTwo->setIcon(question.correctAnswer() == 2 ? m_iconYes : m_iconNo);
    m_ui->m_btnAnswerThree->setIcon(question.correctAnswer() == 3 ? m_iconYes : m_iconNo);
    m_ui->m_btnAnswerFour->setIcon(question.correctAnswer() == 4 ? m_iconYes : m_iconNo);

    m_activeQuestion = question;
  }
  else {
    m_ui->m_txtQuestion->setText(QString());
    m_ui->m_txtAnswerOne->setText(QString());
    m_ui->m_txtAnswerTwo->setText(QString());
    m_ui->m_txtAnswerThree->setText(QString());
    m_ui->m_txtAnswerFour->setText(QString());
    m_ui->m_btnAnswerOne->setIcon(m_iconNo);
    m_ui->m_btnAnswerTwo->setIcon(m_iconNo);
    m_ui->m_btnAnswerThree->setIcon(m_iconNo);
    m_ui->m_btnAnswerFour->setIcon(m_iconNo);
  }

  m_ui->m_txtQuestion->blockSignals(false);
  m_ui->m_txtAnswerOne->blockSignals(false);
  m_ui->m_txtAnswerTwo->blockSignals(false);
  m_ui->m_txtAnswerThree->blockSignals(false);
  m_ui->m_txtAnswerFour->blockSignals(false);
  m_ui->m_btnAnswerOne->blockSignals(false);
  m_ui->m_btnAnswerTwo->blockSignals(false);
  m_ui->m_btnAnswerThree->blockSignals(false);
  m_ui->m_btnAnswerFour->blockSignals(false);

  QTimer::singleShot(0, this, SLOT(configureUpDown()));
}

void QuizEditor::removeQuestion() {
  int current_row = m_ui->m_listQuestions->currentRow();

  if (current_row >= 0) {
    if (m_ui->m_listQuestions->count() == 1) {
      // We are removing last visible question.
      setEditorsEnabled(false);

      m_ui->m_btnQuestionRemove->setEnabled(false);
    }

    delete m_ui->m_listQuestions->takeItem(current_row);
  }

  updateQuestionCount();
  launch();
  emit changed();
}

void QuizEditor::saveQuestion() {
  PlainToolButton *button_sender = dynamic_cast<PlainToolButton*>(sender());

  if (button_sender != NULL) {
    // User clicked some of the "answer" buttons.
    m_activeQuestion.setCorrectAnswer(button_sender->property("id").toInt());

    // Change icons.
    switch (m_activeQuestion.correctAnswer()) {
      case 1:
        m_ui->m_btnAnswerOne->setIcon(m_iconYes);
        m_ui->m_btnAnswerTwo->setIcon(m_iconNo);
        m_ui->m_btnAnswerThree->setIcon(m_iconNo);
        m_ui->m_btnAnswerFour->setIcon(m_iconNo);
        break;

      case 2:
        m_ui->m_btnAnswerOne->setIcon(m_iconNo);
        m_ui->m_btnAnswerTwo->setIcon(m_iconYes);
        m_ui->m_btnAnswerThree->setIcon(m_iconNo);
        m_ui->m_btnAnswerFour->setIcon(m_iconNo);
        break;

      case 3:
        m_ui->m_btnAnswerOne->setIcon(m_iconNo);
        m_ui->m_btnAnswerTwo->setIcon(m_iconNo);
        m_ui->m_btnAnswerThree->setIcon(m_iconYes);
        m_ui->m_btnAnswerFour->setIcon(m_iconNo);
        break;

      case 4:
        m_ui->m_btnAnswerOne->setIcon(m_iconNo);
        m_ui->m_btnAnswerTwo->setIcon(m_iconNo);
        m_ui->m_btnAnswerThree->setIcon(m_iconNo);
        m_ui->m_btnAnswerFour->setIcon(m_iconYes);
        break;

      default:
        m_ui->m_btnAnswerOne->setIcon(m_iconNo);
        m_ui->m_btnAnswerTwo->setIcon(m_iconNo);
        m_ui->m_btnAnswerThree->setIcon(m_iconNo);
        m_ui->m_btnAnswerFour->setIcon(m_iconNo);
        break;
    }
  }

  m_activeQuestion.setQuestion(m_ui->m_txtQuestion->toPlainText());
  m_activeQuestion.setAnswerOne(m_ui->m_txtAnswerOne->text());
  m_activeQuestion.setAnswerTwo(m_ui->m_txtAnswerTwo->text());
  m_activeQuestion.setAnswerThree(m_ui->m_txtAnswerThree->text());
  m_activeQuestion.setAnswerFour(m_ui->m_txtAnswerFour->text());

  m_ui->m_listQuestions->currentItem()->setData(Qt::UserRole, QVariant::fromValue(m_activeQuestion));
  m_ui->m_listQuestions->currentItem()->setText(m_activeQuestion.question());

  emit changed();
}

void QuizEditor::moveQuestionUp() {
  int index = m_ui->m_listQuestions->currentRow();

  m_ui->m_listQuestions->insertItem(index - 1, m_ui->m_listQuestions->takeItem(index));
  m_ui->m_listQuestions->setCurrentRow(index - 1);

  emit changed();
}

void QuizEditor::moveQuestionDown() {
  int index = m_ui->m_listQuestions->currentRow();

  m_ui->m_listQuestions->insertItem(index + 1, m_ui->m_listQuestions->takeItem(index));
  m_ui->m_listQuestions->setCurrentRow(index + 1);

  emit changed();
}

void QuizEditor::configureUpDown() {
  if (m_ui->m_listQuestions->count() > 1) {
    int index = m_ui->m_listQuestions->currentRow();

    if (index == 0) {
      m_ui->m_btnQuestionUp->setEnabled(false);
      m_ui->m_btnQuestionDown->setEnabled(true);
    }
    else if (index == m_ui->m_listQuestions->count() - 1) {
      m_ui->m_btnQuestionUp->setEnabled(true);
      m_ui->m_btnQuestionDown->setEnabled(false);
    }
    else {
      m_ui->m_btnQuestionUp->setEnabled(true);
      m_ui->m_btnQuestionDown->setEnabled(true);
    }
  }
  else {
    m_ui->m_btnQuestionUp->setEnabled(false);
    m_ui->m_btnQuestionDown->setEnabled(false);
  }
}

void QuizEditor::setEditorsEnabled(bool enabled) {
  m_ui->m_groupQuestionEditor->setEnabled(enabled);
}

void QuizEditor::updateNameStatus() {
  if (m_ui->m_txtName->lineEdit()->text().simplified().isEmpty()) {
    m_ui->m_txtName->setStatus(WidgetWithStatus::Error, tr("Enter the name of the quiz."));
  }
  else {
    m_ui->m_txtName->setStatus(WidgetWithStatus::Ok, tr("Name is okay."));
  }

  launch();
  emit changed();
}

void QuizEditor::updateAuthorStatus() {
  if (m_ui->m_txtAuthor->lineEdit()->text().simplified().isEmpty()) {
    m_ui->m_txtAuthor->setStatus(WidgetWithStatus::Error, tr("Enter the name of the author of the quiz."));
  }
  else {
    m_ui->m_txtAuthor->setStatus(WidgetWithStatus::Ok, tr("Enter the name of the author of the quiz."));
  }

  launch();
  emit changed();
}

bool QuizEditor::canGenerateApplications() {
  return
      !m_ui->m_txtName->lineEdit()->text().simplified().isEmpty() &&
      !m_ui->m_txtAuthor->lineEdit()->text().simplified().isEmpty() &&
      !activeQuestions().isEmpty();
}

void QuizEditor::launch() {  
  if (canGenerateApplications()) {
    issueNewGenereationStatus(true);
  }
  else {
    issueNewGenereationStatus(false,
                              tr("Quiz simulation or mobile application generation cannot be started \n"
                                 "because there is no question added or quiz does not have name."));
  }
}

QString QuizEditor::generateBundleData() {
  if (!canGenerateApplications()) {
    return QString();
  }

  QDomDocument source_document = qApp->templateManager()->generateBundleHeader(core()->entryPoint()->typeIndentifier(),
                                                                               m_ui->m_txtAuthor->lineEdit()->text(),
                                                                               QString(),
                                                                               m_ui->m_txtName->lineEdit()->text(),
                                                                               QString(),
                                                                               "1");
  FIND_DATA_ELEMENT(data_element, source_document);

  foreach (const QuizQuestion &question, activeQuestions()) {
    QDomElement item_element = source_document.createElement("item");

    // Fill in details about question.
    QDomElement question_element = source_document.createElement("question");
    QDomElement answer_one_element = source_document.createElement("option");
    QDomElement answer_two_element = source_document.createElement("option");
    QDomElement answer_three_element = source_document.createElement("option");
    QDomElement answer_four_element = source_document.createElement("option");
    QDomElement answer_number_element = source_document.createElement("answer");

    question_element.appendChild(source_document.createTextNode(question.question()));
    answer_one_element.appendChild(source_document.createTextNode(question.answerOne()));
    answer_two_element.appendChild(source_document.createTextNode(question.answerTwo()));
    answer_three_element.appendChild(source_document.createTextNode(question.answerThree()));
    answer_four_element.appendChild(source_document.createTextNode(question.answerFour()));
    answer_number_element.appendChild(source_document.createTextNode(QString::number(question.correctAnswer() - 1)));

    item_element.appendChild(question_element);
    item_element.appendChild(answer_one_element);
    item_element.appendChild(answer_two_element);
    item_element.appendChild(answer_three_element);
    item_element.appendChild(answer_four_element);
    item_element.appendChild(answer_number_element);

    data_element.appendChild(item_element);
  }

  return source_document.toString(2);
}
