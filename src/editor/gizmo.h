/**************************************************************************
This file is part of JahshakaVR, VR Authoring Toolkit
http://www.jahshaka.com
Copyright (c) 2016  GPLv3 Jahshaka LLC <coders@jahshaka.com>

This is free software: you may copy, redistribute
and/or modify it under the terms of the GPLv3 License

For more information see the LICENSE file
*************************************************************************/

#ifndef GIZMOHANDLE_H
#define GIZMOHANDLE_H

#include <QtMath>
#include <QVector3D>
#include <QColor>
#include <QQuaternion>
#include "irisgl/src/irisglfwd.h"

class QOpenGLFunctions_3_2_Core;

enum class GizmoAxis
{
	X,
	Y,
	Z
};

enum class AxisHandle
{
    X = 0,
    Y,
    Z
};

enum class GizmoTransformMode
{
    Translate,
    Rotate,
    Scale
};

enum class GizmoTransformSpace
{
    Local,
    Global
};

enum class GizmoTransformAxis
{
    NONE,
    X,
    Y,
    Z,
    XY,
    XZ,
    YZ
};

enum class GizmoPivot
{
    CENTER,
    OBJECT_PIVOT
};

class GizmoHandle
{
private:

    QColor      handleColor;
    QString     handleName;
    QVector3D   handlePosition;
    QVector3D   handleScale;
    QQuaternion handleRotation;

public:

    GizmoHandle()
    {

    }

    void setHandleColor(const QColor& color) {
        this->handleColor = color;
    }

    QColor getHandleColor() const {
        return this->handleColor;
    }

    void setHandleScale(const QVector3D& scale) {
        this->handleScale = scale;
    }

    QVector3D getHandleScale() const {
        return this->handleScale;
    }

    void setHandleName(const QString& name) {
        this->handleName = name;
    }

    QString getHandleName() const {
        return this->handleName;
    }
};

class Gizmo
{
protected:
	iris::SceneNodePtr selectedNode;
	GizmoTransformSpace transformSpace;
	float gizmoScale;

public:
	Gizmo();
	virtual void updateSize(iris::CameraNodePtr camera);
	float getGizmoScale();

	virtual void setTransformSpace(GizmoTransformSpace transformSpace);
	virtual void setSelectedNode(iris::SceneNodePtr node);
	void clearSelectedNode();

	// returns transform of the gizmo, not the scene node
	// the transform is calculated based on the transform's space (local or global)
	virtual QMatrix4x4 getTransform();
	virtual bool isHit(QVector3D rayPos, QVector3D rayDir);

	virtual bool isDragging() = 0;
	virtual void startDragging(QVector3D rayPos, QVector3D rayDir) = 0;
	virtual void endDragging() = 0;
	virtual void drag(QVector3D rayPos, QVector3D rayDir) = 0;

	virtual void render(QOpenGLFunctions_3_2_Core* gl, QMatrix4x4& viewMatrix, QMatrix4x4& projMatrix) = 0;
};

#endif // GIZMOHANDLE_H
