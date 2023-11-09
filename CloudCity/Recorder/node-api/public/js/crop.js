<<<<<<< Updated upstream
var stage = new createjs.Stage("canvas");
createjs.Ticker.on("tick", tick);
var coordinates={
  xStart:0,
  xEnd:0,
  yStart:0,
  yEnd:0
}
var maxX = 800;
var maxY = 600;

var canvas = document.getElementById("canvas#canvas");
var video = document.querySelector("video#videoshare");

var selection = new createjs.Shape(),
  g = selection.graphics.setStrokeStyle(1).beginStroke("#f20").beginFill("rgba(0,3,4,0.3)"),
  //sd = g.setStrokeDash([20, 5], 0).command,
  r = g.drawRect(0, 0, 100, 100).command,
  moveListener;


stage.on("stagemousedown", dragStart);
stage.on("stagemouseup", dragEnd);

function dragStart(event) {
  stage.addChild(selection).set({
    x: event.stageX,
    y: event.stageY
    
  });
  r.w = 30;
  r.h = 30;
  //console.log("x"+event.stageX)
  //console.log("y"+event.stageY)
 console.log("start"+event  )
 coordinates.xStart=event.stageX;
 coordinates.yStart=event.stageY;
moveListener = stage.on("stagemousemove", drag);
};

function drag(event) {
  r.w = event.stageX - selection.x;
  r.h = event.stageY - selection.y;
  console.log(`adjusted height: ${r.h}, ${aspectRatio}`);
  if (r.w > r.h){
	  r.h = r.w * aspectRatio;
  } else {
	  r.w=r.h / aspectRatio;
	  console.log(`adjusted width: ${r.w}, ${aspectRatio}`);
  }
  //console.log("r.w"+r.w  )
  //console.log("r.h"+event  )
}

function dragEnd(event) {
  stage.off("stagemousemove", moveListener);
  console.log("end"+event)
  maxX = localStream.getVideoTracks()[0].getSettings().width;
  maxY = localStream.getVideoTracks()[0].getSettings().height;
  coordinates.xStart = maxX * (coordinates.xStart/100);
  coordinates.yStart = maxY * (coordinates.yStart/100);
  
  coordinates.xEnd=maxX * (event.stageX/100);
  coordinates.yEnd=maxY * (event.stageY/100);


  
  // canvas.width=s
  // canvas.height=s
  // video.height=s
  // video.width=ss
  
  
  console.log(`coordinates: ${JSON.stringify(coordinates,0,1)}, maxX:${maxX}, maxY:${maxY}`);



}



function tick(event) {
  stage.update(event);
  //sd.offset--;
=======
var SIZE = 50
var stage = new createjs.Stage('canvas')

createjs.Touch.enable(stage)
stage.mouseMoveOutside = true

createjs.Ticker.on('tick', tick)
var coordinates = {
  xStart: 0,
  xEnd: 240,
  yStart: 0,
  yEnd: 240
}

var selection,
  g,
  r,
  moveListener,
  resizeHandle,
  resizing = false,
  maxX, maxY

function init_canvas() {
  selection = new createjs.Shape()
  g = selection.graphics
    .setStrokeStyle(3) 
    .beginStroke('#FEFACD')
    .beginFill('rgb(255,255,255,0.05)')
  r = g.drawRect(0, 0, 40, 40).command

   r.w = r.h
   selection.setBounds(coordinates.xStart, coordinates.yStart, r.w, r.h)

  resizeHandle = new createjs.Shape()
  resizeHandle.graphics.beginFill('#B6B6B6').drawRect(0, 0, 10, 10)

  const getLocalStorage = JSON.parse(localStorage.getItem('coordinates'))

  if (!getLocalStorage) {
    coordinates = {
      xStart: 0,
      xEnd: 240,
      yStart: 0,
      yEnd: 240
    }

    localStorage.setItem('coordinates', JSON.stringify(coordinates))
  } else {
    coordinates = getLocalStorage
  }

  r.w = coordinates.xEnd - coordinates.xStart
  r.h = coordinates.yEnd - coordinates.yStart
	
  stage.addChild(selection).set({
    x: coordinates.xStart,
    y: coordinates.yStart
  })

  stage.addChild(resizeHandle).set({
    x: coordinates.xEnd - 10,
    y: coordinates.yEnd - 10
    // resizeHandle.x = selection.x + r.w - 5
    // resizeHandle.y = selection.y + r.h - 5
  })

  resizeHandle.on('mousedown', function (evt) {
    resizing = true
    stage.on('stagemousemove', drag)
  })

  stage.on('stagemousedown', function (evt) {
    if (
      !selection.hitTest(evt.stageX - selection.x, evt.stageY - selection.y)
    ) {
      if (!resizing) {
        return
      }
    } else {
      moveListener = stage.on('stagemousemove', function (evt) {
        if (!resizing) {
			
		  console.log(  );
          selection.x = ( evt.stageX - r.w ) / 2
          selection.y = ( evt.stageY - r.h ) / 2
          resizeHandle.x = selection.x + r.w - 10
          resizeHandle.y = selection.y + r.h - 10

          coordinates.xStart = selection.x
          coordinates.yStart = selection.y
        }
      })
    }
  })

  stage.on('stagemouseup', function (evt) {
    stage.off('stagemousemove', moveListener)
    resizing = false
	  
    coordinates.xEnd = Math.ceil( selection.x + r.w )
    coordinates.yEnd = Math.ceil( selection.y + r.h )
	
	if( ( coordinates.xEnd - coordinates.xStart ) < 240 ) 
	{ coordinates.xEnd = coordinates.xStart + 240 }
	if( ( coordinates.yEnd - coordinates.yStart ) < 240 ) 
	{ coordinates.yEnd = coordinates.yStart + 240 }

	coordinates.xStart = Math.ceil( coordinates.xStart )
	coordinates.yStart = Math.ceil( coordinates.yStart )
	coordinates.xEnd = Math.ceil( coordinates.xEnd )
	coordinates.yEnd = Math.ceil( coordinates.yEnd )
	  
    console.log(
      "Boundary moved/sized: ",
      "xStart ", coordinates.xStart,
      ", yStart ", coordinates.yStart,
	  ", xLength ", coordinates.xEnd - coordinates.xStart,
	  ", yLength ", coordinates.yEnd - coordinates.yStart
    )

    // Saving to localStorage after resizing or dragging ends
    localStorage.setItem(
      'coordinates',
      JSON.stringify({
        xStart: coordinates.xStart.toString(),
        yStart: coordinates.yStart.toString(),
        xEnd: coordinates.xEnd.toString(),
        yEnd: coordinates.yEnd.toString()
      })
    )
  })

  stage.update()
}

function drag(event) {
  if (resizing) {
    var width = Math.max(event.stageX - selection.x, 20)
    var height = Math.max(event.stageY - selection.y, 20)

    var size = Math.max(width, height) // always maintain a square shape

    r.w = size
    r.h = size * 1.2
    resizeHandle.x = selection.x + r.w - 10
    resizeHandle.y = selection.y + r.h - 10
  }
}

function tick(event) {
  stage.update(event)
>>>>>>> Stashed changes
}
