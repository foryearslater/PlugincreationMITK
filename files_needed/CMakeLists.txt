project(org_mitk_lancet_nodeeditor)

mitk_create_plugin(
  EXPORT_DIRECTIVE NODEEDITOR_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkBoundingShape MitkLancetAlgo MitkSurfaceRegistration MitkLancetGeoUtil MitkDRR
)
