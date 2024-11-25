/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef NodeEditor_h
#define NodeEditor_h

#include "mitkImage.h"
#include "nodebinder.h"
#include "surfaceregistraion.h"
#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_NodeEditorControls.h"

/**
  \brief NodeEditor

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class NodeEditor : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  static const std::string VIEW_ID;

protected:
  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void SetFocus() override;

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
                                  const QList<mitk::DataNode::Pointer> &nodes) override;

  /// \brief Called when the user clicks the GUI button
  //void DoImageProcessing();

  Ui::NodeEditorControls m_Controls;


    // landmark ICP transform
  void onSourcePsetChanged(QmitkSingleNodeSelectionWidget::NodeList /*nodes*/);
  void onTargetPsetChanged(QmitkSingleNodeSelectionWidget::NodeList /*nodes*/);
  void onMovingNodeChanged(QmitkSingleNodeSelectionWidget::NodeList /*nodes*/);
  void onIcpPsetChanged(QmitkSingleNodeSelectionWidget::NodeList /*nodes*/);
  void OnPushButtonApplyLandMarks();
  void OnPushButtonApplyICP();
  void OnPushButtonUndo();

  // landmark trans
  mitk::DataNode *m_sourcePset{nullptr};
  mitk::DataNode *m_targetPset{nullptr};
  mitk::DataNode *m_registSrc{nullptr};
  mitk::DataNode *m_icpPset{nullptr};

  mitk::SurfaceRegistration::Pointer m_surfaceRegistration;

  void InitPointSetSelector(QmitkSingleNodeSelectionWidget *widget);

};

inline void NodeEditor::onMovingNodeChanged(QmitkSingleNodeSelectionWidget::NodeList)
{
  m_registSrc = m_Controls.widget_MovingNode->GetSelectedNode();
}

inline void NodeEditor::onTargetPsetChanged(QmitkSingleNodeSelectionWidget::NodeList)
{
  m_targetPset = m_Controls.widget_TargetPset->GetSelectedNode();
}

inline void NodeEditor::onIcpPsetChanged(QmitkSingleNodeSelectionWidget::NodeList)
{
  m_icpPset = m_Controls.widget_IcpPset->GetSelectedNode();
}

inline void NodeEditor::onSourcePsetChanged(QmitkSingleNodeSelectionWidget::NodeList /*nodes*/)
{
  m_sourcePset = m_Controls.widget_SourcePset->GetSelectedNode();
}


#endif // NodeEditor_h
