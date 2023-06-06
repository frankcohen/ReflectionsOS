var express = require('express');
var router = express.Router();
const path = require('path');
const common = require('../common');

const UPLOAD_TMP_DIR = 'uploads/tmp'; 
const FINAL_PATH = 'uploads/final';
// const TAR_PATH = '/home/ec2-user/cloudCity/files';
const TAR_PATH = '/home/fcohen/files';
const AUDIO_CODEC = 'm4a';
const VIDEO_CODEC = 'mjpeg';
const TAR_FORMAT = 'tar';
const TAR_CMD = 'tar'
const TAR_OPTS = '-cf'
const CLEAN_UPLOADS = true;

const multer = require('multer');
const { xml } = require('jade/lib/doctypes');

const upload = multer({
  dest:UPLOAD_TMP_DIR
})
/* GET users listing. */
router.post('/', upload.single('file'), async function(req, res, next) {
  console.log(req.file)
  console.log(`CROP: ${req.body.crop}`);

  
  const {xStart, xEnd, yStart, yEnd, maxX, maxY} = JSON.parse(req.body.crop);
  const frameWidth = parseFloat(xEnd) - parseFloat(xStart);
  const frameHeight = parseFloat(yEnd) - parseFloat(yStart);
  const videoWidth = parseFloat(maxX);
  const videoHeight = parseFloat(maxY);

  const scaleX = videoWidth / frameWidth;
  const scaleY = videoHeight / frameHeight;
  
  console.log(`Width: ${frameWidth}, Height: ${frameHeight}`);


  /*
  const ffmpegCmd = `ffmpeg -i ${tmpPath} -map 0:a ${finalPathAudio} -filter:v "crop=${width}:${height}:${xStart}:${yStart},fps=15,scale=240:240:flags=lanczos,crop=240:in_h:(in_w-240)/2:0"  -q:v 9 -map 0:v ${tempPathVideo}`
  const ffmpegCropCmd = `ffmpeg -i ${tempPathVideo} -vf "fps=15,scale=240:240:flags=lanczos,crop=240:in_h:(in_w-240)/2:0" -q:v 9 ${finalPathVideo}`

  const ffmpegCmd = `ffmpeg -i ${tmpPath} -map 0:a ${finalPathAudio} -vf  "crop=${width}:${height}:${xStart}:${yStart},scale=240:240:flags=lanczos,fps=15" -q:v 9 -map 0:v ${finalPathVideo}`;

*/
  
  const tmpPath = path.join(UPLOAD_TMP_DIR, req.file.filename); 
  const finalPathAudio = path.join(FINAL_PATH, req.file.filename) + "." + AUDIO_CODEC;  
  const finalPathVideo = path.join(FINAL_PATH, req.file.filename) + "." + VIDEO_CODEC;
  const finalPathJson = path.join(FINAL_PATH, req.file.filename) + ".json";
  const tempPathVideo = path.join(FINAL_PATH, req.file.filename) + "_t." + VIDEO_CODEC;
  const finalPathArchive = path.join(TAR_PATH, req.file.filename) + "." + TAR_FORMAT;

  const w = Math.round(frameWidth*scaleX);
  const h = Math.round(frameHeight*scaleY);
  const x = Math.round(xStart*scaleX);
  const y = Math.round(yStart*scaleY);

  console.log(`Width: ${w}, Height: ${h}, X: ${x}, Y: ${y}`);

    
  const ffmpegCmd = `/usr/local/bin/ffmpeg/ffmpeg-git-20230313-amd64-static/ffmpeg -i ${tmpPath} -map 0:a ${finalPathAudio} -vf  "crop=${w}:${h}:${x}:${y},scale=240:240:flags=lanczos,fps=15" -q:v 9 -map 0:v ${finalPathVideo}`;
  
  console.log(`Executing ffmpegCmd command: ${ffmpegCmd}`);
  try{
    let execResp = await common.exec(ffmpegCmd);
    //console.log(`Executing ffmpegCropCmd command: ${ffmpegCropCmd}`);
  //let execRespCrop = await common.exec(ffmpegCropCmd);

  const metaJson = {
        "ReflectionsShow": {
                "title": req.file.filename,
                "showname": req.file.filename,
                "version": "1",
                "events": {
                        "1": {
                                "type": "OnStart",
                                "name": "Startup sequence",
                                "comment": "Welcome to Reflections",
        
        "sequence": [{
                                        "playaudio": `${req.file.filename}.${AUDIO_CODEC}`,
                                        "playvideo": `${req.file.filename}.${VIDEO_CODEC}`
                                }]
                        }
                }
        }
};
    const fs = require('fs').promises;
    await fs.writeFile(finalPathJson, JSON.stringify(metaJson,0,4));
    const tarCmd = `${TAR_CMD} -C ${FINAL_PATH} ${TAR_OPTS} ${finalPathArchive} ${req.file.filename}.${AUDIO_CODEC} ${req.file.filename}.${VIDEO_CODEC} ${req.file.filename}.json`;
    console.log(`executing tar command: ${tarCmd}`);
    execResp = await common.exec(tarCmd);
    if (CLEAN_UPLOADS){
          const cleanCmd = `rm -f ${finalPathAudio} ${finalPathVideo} ${finalPathJson} ${tempPathVideo} ${tmpPath}`
          execResp = await common.exec(cleanCmd);
    }
    console.log('/api/users SUCCESS!')
    res.setHeader('Content-Type', 'application/json');
    res.end(JSON.stringify({"status":"success","path":`rec/${req.file.filename}.${TAR_FORMAT}`}));
  } catch (e) {
    console.error(`Exception in ffmpeg: ${e}`);
    res.status(500).json({"status":"failed"});
    return;
  }



});

module.exports = router;
