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
}
