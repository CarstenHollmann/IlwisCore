import QtQuick 2.0

BasicModellerObject {

    nameText: "Operation"

   property int defaultWidth : 50
   property int defaultHeight : 50

    width: defaultWidth
    height: defaultHeight

    function draw(ctx) {
        ctx.save();
        ctx.beginPath();
        if (selected) {
            ctx.lineWidth = 2
            ctx.strokeStyle = "red"
            ctx.strokeRect(x-width/2, y-height/2, width, height);
        } else {
             ctx.rect(x-width/2, y-height/2, width, height);
        }
        ctx.stroke();
        ctx.restore();
        ctx.save();
        ctx.beginPath();
        ctx.text(nameText, (x - width/2), y);
        ctx.stroke();
        ctx.restore();
    }

}