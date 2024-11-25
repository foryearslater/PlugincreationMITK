/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include <sstream>
// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "NodeEditor.h"

// Qt
#include <QDoubleSpinBox>
#include <QPushButton>

// mitk image
#include "basic.h"
#include "mitkImagePixelReadAccessor.h"
#include "mitkImagePixelWriteAccessor.h"
#include "mitkNodePredicateAnd.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateNot.h"
#include "mitkNodePredicateOr.h"
#include "mitkNodePredicateProperty.h"
#include "mitkSurfaceOperation.h"
#include "physioModelFactory.h"
#include <QComboBox>
#include <itkBSplineInterpolateImageFunction.h>
#include <itkResampleImageFilter.h>
#include <mitkApplyTransformMatrixOperation.h>
#include <mitkBoundingShapeCropper.h>
#include <mitkImage.h>
#include <mitkInteractionConst.h>
#include <mitkPadImageFilter.h>
#include <mitkPointOperation.h>
#include <mitkPointSet.h>
#include <mitkPointSetShapeProperty.h>
#include <mitkRotationOperation.h>
#include <mitkSurface.h>
#include <mitkSurfaceToImageFilter.h>
#include <mitkVector.h>
#include <vtkClipPolyData.h>
#include <vtkImageStencil.h>
#include <vtkLandmarkTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkPlane.h>
#include <vtkSTLReader.h>

#include <eigen3/Eigen/Eigen>
#include <mitkPadImageFilter.h>
#include <drr.h>
#include <nodebinder.h>
#include <surfaceregistraion.h>

#define PI acos(-1)
const std::string NodeEditor::VIEW_ID = "org.mitk.views.nodeeditor";

void NodeEditor::SetFocus()
{
  m_Controls.pushButton_applyLandMark->setFocus();
}

void NodeEditor::InitPointSetSelector(QmitkSingleNodeSelectionWidget *widget)
{
  widget->SetDataStorage(GetDataStorage());
  widget->SetNodePredicate(mitk::NodePredicateAnd::New(
    mitk::TNodePredicateDataType<mitk::PointSet>::New(),
    mitk::NodePredicateNot::New(mitk::NodePredicateOr::New(mitk::NodePredicateProperty::New("helper object"),
                                                           mitk::NodePredicateProperty::New("hidden object")))));

  widget->SetSelectionIsOptional(true);
  widget->SetAutoSelectNewNodes(true);
  widget->SetEmptyInfo(QString("Please select a point set"));
  widget->SetPopUpTitel(QString("Select point set"));
}


void NodeEditor::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  //connect(m_Controls.buttonPerformImageProcessing, &QPushButton::clicked, this, &NodeEditor::DoImageProcessing);

  // Set Node Selection Widget
  // source
  InitPointSetSelector(m_Controls.widget_SourcePset);
  // target
  InitPointSetSelector(m_Controls.widget_TargetPset);
  // target
  InitPointSetSelector(m_Controls.widget_IcpPset);

// moving node
  m_Controls.widget_MovingNode->SetDataStorage(GetDataStorage());
  m_Controls.widget_MovingNode->SetNodePredicate(mitk::NodePredicateNot::New(mitk::NodePredicateOr::New(
    mitk::NodePredicateProperty::New("helper object"), mitk::NodePredicateProperty::New("hidden object"))));

  m_Controls.widget_MovingNode->SetSelectionIsOptional(true);
  m_Controls.widget_MovingNode->SetAutoSelectNewNodes(true);
  m_Controls.widget_MovingNode->SetEmptyInfo(QString("Please select a node"));
  m_Controls.widget_MovingNode->SetPopUpTitel(QString("Select node"));

  connect(m_Controls.widget_SourcePset,
          &QmitkSingleNodeSelectionWidget::CurrentSelectionChanged,
          this,
          &NodeEditor::onSourcePsetChanged);
  connect(m_Controls.widget_TargetPset,
          &QmitkSingleNodeSelectionWidget::CurrentSelectionChanged,
          this,
          &NodeEditor::onTargetPsetChanged);
  connect(m_Controls.widget_IcpPset,
          &QmitkSingleNodeSelectionWidget::CurrentSelectionChanged,
          this,
          &NodeEditor::onIcpPsetChanged);
  connect(m_Controls.widget_MovingNode,
          &QmitkSingleNodeSelectionWidget::CurrentSelectionChanged,
          this,
          &NodeEditor::onMovingNodeChanged);
  connect(m_Controls.pushButton_applyLandMark, &QPushButton::clicked, this, &NodeEditor::OnPushButtonApplyLandMarks);
  connect(m_Controls.pushButton_applyIcp, &QPushButton::clicked, this, &NodeEditor::OnPushButtonApplyICP);
  connect(m_Controls.pushButton_undo, &QPushButton::clicked, this, &NodeEditor::OnPushButtonUndo);


}

void NodeEditor::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
                                                const QList<mitk::DataNode::Pointer> &nodes)
{
  //// iterate all selected objects, adjust warning visibility
  //foreach (mitk::DataNode::Pointer node, nodes)
  //{
  //  if (node.IsNotNull() && dynamic_cast<mitk::Image *>(node->GetData()))
  //  {
  //    m_Controls.labelWarning->setVisible(false);
  //    m_Controls.buttonPerformImageProcessing->setEnabled(true);
  //    return;
  //  }
  //}

  //m_Controls.labelWarning->setVisible(true);
  //m_Controls.buttonPerformImageProcessing->setEnabled(false);
}

//void NodeEditor::DoImageProcessing()
//{
//  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
//  if (nodes.empty())
//    return;
//
//  mitk::DataNode *node = nodes.front();
//
//  //if (!node)
//  //{
//  //  // Nothing selected. Inform the user and return
//  //  QMessageBox::information(nullptr, "Template", "Please load and select an image before starting image processing.");
//  //  return;
//  //}
//
//  // here we have a valid mitk::DataNode
//
//  // a node itself is not very useful, we need its data item (the image)
//  mitk::BaseData *data = node->GetData();
//  if (data)
//  {
//    // test if this data item is an image or not (could also be a surface or something totally different)
//    mitk::Image *image = dynamic_cast<mitk::Image *>(data);
//    if (image)
//    {
//      std::stringstream message;
//      std::string name;
//      message << "Performing image processing for image ";
//      if (node->GetName(name))
//      {
//        // a property called "name" was found for this DataNode
//        message << "'" << name << "'";
//      }
//      message << ".";
//      MITK_INFO << message.str();
//
//      // actually do something here...
//    }
//  }
//}


void NodeEditor::OnPushButtonApplyLandMarks()
{
  if (m_surfaceRegistration == nullptr)
  {
    m_surfaceRegistration = mitk::SurfaceRegistration::New();
  }
  MITK_INFO << "OnPushButtonApplyLandMark";
  if (m_sourcePset != nullptr && m_targetPset != nullptr)
  {
    auto sourcePointset = dynamic_cast<mitk::PointSet *>(m_sourcePset->GetData());
    auto targetPointset = dynamic_cast<mitk::PointSet *>(m_targetPset->GetData());
    m_surfaceRegistration->SetLandmarksSrc(sourcePointset);
    m_surfaceRegistration->SetLandmarksTarget(targetPointset);
    m_surfaceRegistration->ComputeLandMarkResult();
  }
  std::ostringstream os;
  m_surfaceRegistration->GetResult()->Print(os);
  m_Controls.textBrowser_trans_info->append(QString::fromStdString(os.str()));

  if (m_registSrc != nullptr)
  {
    // create a copy node of regist src
    auto name = m_registSrc->GetName() + "_regist";
    if (GetDataStorage()->GetNamedNode(name) == nullptr)
    {
      mitk::DataNode::Pointer tmpNode = mitk::DataNode::New();
      tmpNode->SetName(name);
      mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface *>(m_registSrc->GetData())->Clone();
      tmpNode->SetData(surface);
      tmpNode->SetVisibility(true);
      GetDataStorage()->Add(tmpNode);
    }
    // reinit copy node geometry to src geometry
    auto copyNode = GetDataStorage()->GetNamedNode(name);
    copyNode->GetData()->SetGeometry(m_registSrc->GetData()->GetGeometry());

    vtkTransform *trans = vtkTransform::New();
    trans->SetMatrix(copyNode->GetData()->GetGeometry()->GetVtkMatrix());
    trans->Concatenate(m_surfaceRegistration->GetResult());
    trans->Update();
    mitk::Point3D ref;
    auto *doOp = new mitk::ApplyTransformMatrixOperation(mitk::OpAPPLYTRANSFORMMATRIX, trans->GetMatrix(), ref);
    // execute the Operation
    // here no undo is stored, because the movement-steps aren't interesting.
    // only the start and the end is interisting to store for undo.
    copyNode->GetData()->GetGeometry()->ExecuteOperation(doOp);
    delete doOp;

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void NodeEditor::OnPushButtonApplyICP()
{
  if (m_surfaceRegistration == nullptr)
  {
    m_surfaceRegistration = mitk::SurfaceRegistration::New();
  }
  MITK_INFO << "OnPushButtonApplyICP";

  if (m_icpPset != nullptr && m_registSrc != nullptr)
  {
    auto IcpPointset = dynamic_cast<mitk::PointSet *>(m_icpPset->GetData());
    auto surface = dynamic_cast<mitk::Surface *>(m_registSrc->GetData());
    m_surfaceRegistration->SetIcpPoints(IcpPointset);
    m_surfaceRegistration->SetSurfaceSrc(surface);
    m_surfaceRegistration->ComputeIcpResult();
  }

  std::ostringstream os;
  m_surfaceRegistration->GetResult()->Print(os);
  m_Controls.textBrowser_trans_info->append(QString::fromStdString(os.str()));

  if (m_registSrc != nullptr)
  {
    // create a copy node of regist src
    auto name = m_registSrc->GetName() + "_regist";
    if (GetDataStorage()->GetNamedNode(name) == nullptr)
    {
      mitk::DataNode::Pointer tmpNode = mitk::DataNode::New();
      tmpNode->SetName(name);
      mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface *>(m_registSrc->GetData())->Clone();
      tmpNode->SetData(surface);
      tmpNode->SetVisibility(true);
      GetDataStorage()->Add(tmpNode);
    }
    // reinit copy node geometry to src geometry
    auto copyNode = GetDataStorage()->GetNamedNode(name);
    copyNode->GetData()->SetGeometry(m_registSrc->GetData()->GetGeometry());

    vtkTransform *trans = vtkTransform::New();
    trans->SetMatrix(copyNode->GetData()->GetGeometry()->GetVtkMatrix());
    trans->Concatenate(m_surfaceRegistration->GetResult());
    trans->Update();
    mitk::Point3D ref;
    auto *doOp = new mitk::ApplyTransformMatrixOperation(mitk::OpAPPLYTRANSFORMMATRIX, trans->GetMatrix(), ref);
    // execute the Operation
    // here no undo is stored, because the movement-steps aren't interesting.
    // only the start and the end is interisting to store for undo.
    copyNode->GetData()->GetGeometry()->ExecuteOperation(doOp);
    delete doOp;

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void NodeEditor::OnPushButtonUndo()
{
  if (m_surfaceRegistration != nullptr)
  {
    m_surfaceRegistration->Undo();
    std::ostringstream os;
    m_surfaceRegistration->GetResult()->Print(os);
    m_Controls.textBrowser_trans_info->append(QString::fromStdString(os.str()));

    if (m_registSrc != nullptr)
    {
      // create a copy node of regist src
      auto name = m_registSrc->GetName() + "_regist";
      if (GetDataStorage()->GetNamedNode(name) == nullptr)
      {
        mitk::DataNode::Pointer tmpNode = mitk::DataNode::New();
        tmpNode->SetName(name);
        mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface *>(m_registSrc->GetData())->Clone();
        tmpNode->SetData(surface);
        tmpNode->SetVisibility(true);
        GetDataStorage()->Add(tmpNode);
      }
      // reinit copy node geometry to src geometry
      auto copyNode = GetDataStorage()->GetNamedNode(name);
      copyNode->GetData()->SetGeometry(m_registSrc->GetData()->GetGeometry());

      vtkTransform *trans = vtkTransform::New();
      trans->SetMatrix(copyNode->GetData()->GetGeometry()->GetVtkMatrix());
      trans->Concatenate(m_surfaceRegistration->GetResult());
      trans->Update();
      mitk::Point3D ref;
      auto *doOp = new mitk::ApplyTransformMatrixOperation(mitk::OpAPPLYTRANSFORMMATRIX, trans->GetMatrix(), ref);
      // execute the Operation
      // here no undo is stored, because the movement-steps aren't interesting.
      // only the start and the end is interisting to store for undo.
      copyNode->GetData()->GetGeometry()->ExecuteOperation(doOp);
      delete doOp;

      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}