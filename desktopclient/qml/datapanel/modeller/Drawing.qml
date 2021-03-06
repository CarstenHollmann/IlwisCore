import QtQuick 2.0

Canvas {
    id:canvas
    property int paintX
    property int paintY
    property int count: 0
    property int lineWidth: 2
    property string drawColor: "black"
    property var ctx: getContext('2d')
    property variant elements: []
    property bool canvasValid: true
    property bool isDrag: false
     property bool isPressAndHold: false

    property real lastX
    property real lastY

    property BasicModellerObject currentCreateElement
    property BasicModellerObject currentSelectedElement
    property BasicModellerObject onPressedSelectedElement

    Timer {
        interval: 30;
        running: true;
        repeat: true
        onTriggered: canvas.draw()
    }

    function addType(elem) {
        currentCreateElement = elem
    }

    /*
     * Set canvas validation flag to false
     */
    function invalidate() {
        canvasValid = false;
    }

    function draw(force){
        if (canvasValid == false || (force !== null && force)) {
            clear(ctx);
            canvasValid = true
            var l = elements.length
            for (var i = 0; i < l; i++) {
                elements[i].draw(ctx);
            }
            if (currentCreateElement != null && currentCreateElement.nameText == "Connector") {
                currentCreateElement.draw(ctx);
            }
            canvas.requestPaint();
        }
    }

    function clear() {
        ctx.reset();
        ctx.clearRect(0, 0, width, height);
        ctx.stroke();
        canvas.requestPaint();
    }

    function clearModeller() {
        clearSelections();
        elements = [];
        clear();
        invalidate();
    }

    function clearSelections() {
        var l = elements.length;
        for (var i = l-1; i >= 0; i--) {
            elements[i].selected = false;
        }
        currentSelectedElement = null;
        isDrag = false;
        invalidate();
    }

    function checkForSelectedElement(mouseX, mouseY) {
        currentSelectedElement = checkForElementAt(mouseX, mouseY);
        if (currentSelectedElement != null) {
            return true;
        }
        return false;

    }

    function checkForElementAt(mouseX, mouseY) {
        var l = elements.length;
        var element = null;
        for (var i = l-1; i >= 0; i--) {
            elements[i].selected = false;
            if (elements[i].isSelected(mouseX, mouseY)) {
                element = elements[i];
            }
        }
        return element;
    }

    function createNewElement(mouseX, mouseY) {
        currentCreateElement.setCoordinates(mouseX, mouseY);
        resetCurrentSelectedElement();
        currentSelectedElement = currentCreateElement;
        currentSelectedElement.selected = true;
        var t = elements;
        t.push(currentCreateElement);
        elements = t;
        currentCreateElement = null;
        invalidate();
    }

    function createNewConnector(createElem) {
        var t = elements;
        t.push(createElem);
        elements = t;
        currentCreateElement = null;
        invalidate();
    }

    function resetCurrentSelectedElement() {
        if (currentSelectedElement !== null) {
            currentSelectedElement.selected = false;
            currentSelectedElement = null;
        }
        invalidate();
    }

    onWidthChanged: {
        // force re-draw if the ModellerPanel width has changed
        invalidate();
    }

    onHeightChanged: {
        // force re-draw if the ModellerPanel height has changed
        canvas.draw(true);
    }

    /*
      * click: onPressed, onReleased, onClicked
      * click'n'hold: onPressed, onPressAndHold, onReleased
      * click'n'hold and move: onPressed, onPositionChanged, onPressAndHold, onPositionChanged, onReleased
      */

    DropArea {
        id: dropArea
        anchors.fill: parent
        onDropped: {
            var element = checkForElementAt(drag.x, drag.y);
            if (element !== null && element.objectContainer === null) {
                element.objectContainer = Qt.createComponent("ModellerObjectContainer.qml").createObject();
                element.objectContainer.ilwisObjectId = drag.source.ilwisobjectid;
                element.objectContainer.imagePath = drag.source.message
                element.objectContainer.dataSource = drag.source.source
                invalidate();
            }
        }
    }

    MouseArea {
        id: area
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onPressed: {
            if (mouse.button == Qt.LeftButton) {
                var element = checkForElementAt(mouseX, mouseY)
                if(element !== null) {
                    if (currentCreateElement != null && currentCreateElement.nameText == "Connector") {
                        if (!currentCreateElement.hasParentObject()) {
                            element.childObject = currentCreateElement;
                            currentCreateElement.parentObject = element;
                            currentCreateElement.setStartCoordinate(element.getOutputConnector().x, element.getOutputConnector().y);
                        }
                    } else {
                        onPressedSelectedElement = element;
                        isDrag = true;
                    }
                }
            }
        }

        onPositionChanged: {
            if (currentCreateElement != null && currentCreateElement.nameText == "Connector") {
                currentCreateElement.setEndCoordinate(mouseX, mouseY);
                invalidate();
            } else {
                if (onPressedSelectedElement != null) {
                    if (isDrag) {
                        onPressedSelectedElement.setCoordinates(mouseX, mouseY);
                        invalidate();
                    }
                }
            }
        }

        onPressAndHold: {
            isPressAndHold = true;
        }

        onReleased: {
            if (currentCreateElement != null && currentCreateElement.nameText == "Connector") {
                var element = checkForElementAt(mouseX, mouseY);
                if(element !== null) {
                    if (currentCreateElement.hasParentObject() && !currentCreateElement.hasChildObject() && currentCreateElement.parentObject !== element) {
                        element.parentObject = currentCreateElement;
                        currentCreateElement.childObject = element;
                        currentCreateElement.setEndCoordinate(element.getInputConnector().x, element.getInputConnector().y);
                        createNewConnector(currentCreateElement);
                        currentCreateElement = null;
                    }
                }
            }
            isDrag = false;
            if (isPressAndHold) {
                clearSelections();
            }
            onPressedSelectedElement = null;
            if (mouse.button == Qt.LeftButton) {

            }
        }

        onClicked: {
            isDrag = false;
            if (mouse.button == Qt.LeftButton) {
                var element = checkForElementAt(mouseX, mouseY);
                if (currentCreateElement !== null) {
                    resetCurrentSelectedElement();
//                    if (currentCreateElement.nameText == "Connector") {
//                        if(element !== null) {
//                            if (!currentCreateElement.hasParentObject()) {
//                                element.childObject = currentCreateElement;
//                                currentCreateElement.parentObject = element;
//                                currentCreateElement.setStartCoordinate(element.getOutputConnector().x, element.getOutputConnector().y);
//                            } else if (currentCreateElement.hasParentObject() && !currentCreateElement.hasChildObject() && currentCreateElement.parentObject !== element) {
//                                element.parentObject = currentCreateElement;
//                                currentCreateElement.childObject = element;
//                                currentCreateElement.setEndCoordinate(element.getInputConnector().x, element.getInputConnector().y);
//                            }
//                        }
//                    } else {
                        createNewElement(mouseX, mouseY);
//                    }
                } else {
                    if (element !== null) {
                        currentSelectedElement = element;
                        currentSelectedElement.selected = true;
                        invalidate();
                    }
                }
            } else if (mouse.button == Qt.RightButton) {
                clearSelections();
            }
        }

        onDoubleClicked: {
            if (mouse.button == Qt.LeftButton) {
                // open metadata?
            }
        }
    }
}
