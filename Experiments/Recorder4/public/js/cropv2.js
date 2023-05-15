var SIZE = 50;
var stage = new createjs.Stage("canvas");

// For mobile devices.
createjs.Touch.enable(stage);

// this lets our drag continue to track the mouse even when it leaves the canvas:
// play with commenting this out to see the difference.
stage.mouseMoveOutside = true;

createjs.Ticker.on("tick", tick);
var coordinates = {
    xStart: 0,
    xEnd: 0,
    yStart: 0,
    yEnd: 0
}
var maxX = 800;
var maxY = 800;

var canvas = document.getElementsByTagName('canvas')[0];
var video = document.querySelector("video#videoshare");

var selection ,
    g, //= selection.graphics.setStrokeStyle(1).beginStroke("#f20").beginFill("rgba(0,3,4,0.3)"),
    //sd = g.setStrokeDash([20, 5], 0).command,
    r,// = g.drawRect(10, 10, 100, 100).command,
    moveListener;

function init_canvas() {		
	selection = new createjs.Shape();
    g = selection.graphics.setStrokeStyle(4).beginStroke("#f20").beginFill("rgba(0,3,4,0.3)");
    r = g.drawRect(0, 0, 50, 50).command;

    stage.on("stagemousedown", dragStart);
    stage.on("stagemouseup", dragEnd);
    r.w = 50;
    r.h = 50;
    stage.addChild(selection).set({
        x: 0,
        y: 0
    });

    // var label = new createjs.Text("drag me", "bold 14px Arial", "#00FFFF");
    // label.textAlign = "top";
    // label.y = 1;
    // label.x = 1;

    // var dragger = new createjs.Container();
    // dragger.x = dragger.y = 100;
    // dragger.addChild(selection, label);
    // stage.addChild(dragger);

    // dragger.on("pressmove", function (evt) {
    //     // currentTarget will be the container that the event listener was added to:
    //     evt.currentTarget.x = evt.stageX;
    //     evt.currentTarget.y = evt.stageY;
    //     // make sure to redraw the stage to show the change:
    //     stage.update();
    // });

    stage.update();
}

function addRoundedSquare(x, y, s, r, fill) {
    
    // selection.graphics.beginFill(fill).drawRoundRect(0, 0, s, s, r);
    selection.x = x - s / 2;
    selection.y = y - s / 2;
    // selection.name = "square";
    selection.on("pressmove", drag);
    //square.on("stagemousedown", dragStart);
    //square.on("stagemouseup", dragEnd);

    stage.addChild(selection);
}

function dragStart(event) {
    stage.addChild(selection).set({
        x: event.stageX,
        y: event.stageY

    });
    r.w = 0;
    r.h = 0;
    //console.log("x"+event.stageX)
    //console.log("y"+event.stageY)
    console.log("start" + event)
    coordinates.xStart = event.stageX;
    coordinates.yStart = event.stageY;
    moveListener = stage.on("stagemousemove", drag);
};

function drag(event) {
	
    r.w = event.stageX - selection.x;
    r.h = event.stageY - selection.y;
    if ( r.w > ( r.h * aspectRatio ) ) {
        r.h = r.w * ( aspectRatio );
    } else {
        r.w = r.h / aspectRatio;
    }
	
	console.log( "r.w="+r.w + " stageX=" + event.stageX + " r.h=" + r.h + " stageY=" + event.stageY + "  aspectRatio=" + aspectRatio );

	
	
    //console.log(`drag ${r.h}, ${r.w}, ${aspectRatio}`)
    //console.log("r.w"+r.w  )
    //console.log("r.h"+event  )
}

function dragEnd(event) {
    stage.off("stagemousemove", moveListener);
    
	//console.log("end" + event)
    //maxX = localStream.getVideoTracks()[0].getSettings().width;
    //maxY = localStream.getVideoTracks()[0].getSettings().height;

	//console.log( `end r.w = ${r.w}, r.h = ${r.h}, ${ JSON.stringify(coordinates, 0, 1) } `);

	/*
	maxX = r.w;
    maxY = r.h;

    coordinates.xStart = maxX * (coordinates.xStart / 100);
    coordinates.yStart = maxY * (coordinates.yStart / 100);

    coordinates.xEnd = maxX * (event.stageX / 100);
    coordinates.yEnd = maxY * (event.stageY / 100);
	*/
	
    // canvas.width=s
    // canvas.height=s
    // video.height=s
    // video.width=s
	
	coordinates.xEnd = coordinates.xStart + r.w;
	coordinates.yEnd = ( coordinates.xEnd - coordinates.xStart ) + coordinates.yStart ;
	
	coordinates.xStart = coordinates.xStart.toFixed();
	coordinates.yStart = coordinates.yStart.toFixed();
	coordinates.xEnd = coordinates.xEnd.toFixed();
	coordinates.yEnd = coordinates.yEnd.toFixed();
	
    console.log(`end coordinates: ${JSON.stringify(coordinates, 0, 1)}, maxX:${maxX}, maxY:${maxY}`);
}

function tick(event) {
    stage.update(event);
    //sd.offset--;
}
