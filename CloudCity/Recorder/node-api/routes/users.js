// const common = require('./common');
var express = require('express');
var router = express.Router();
const path = require('path');
const { exec } = require('child_process');

const UPLOAD_TMP_DIR = 'uploads/tmp'; 
const FINAL_PATH = 'uploads/final';
// const TAR_PATH = '/home/ec2-user/cloudCity/files';
const TAR_PATH = '/home/fcohen/files';
const AUDIO_CODEC = 'mp3';
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


const common = {
  exec: (command) => {
    return new Promise((resolve, reject) => {
      exec(command, (error, stdout, stderr) => {
        if (error) {
          console.error(`exec error: ${error}`);
          reject(error);
          return;
        }
        console.log(`stdout: ${stdout}`);
        console.error(`stderr: ${stderr}`);
        resolve(stdout);
      });
    });
  },
}


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
  
  const tmpPath = path.join(UPLOAD_TMP_DIR, req.file.filename); 
  const finalPathAudio = path.join(FINAL_PATH, req.file.filename) + "." + AUDIO_CODEC; 
  const finalPathVideo = path.join(FINAL_PATH, req.file.filename) + "." + VIDEO_CODEC;
  const finalPathJson = path.join(FINAL_PATH, req.file.filename) + ".json";
  const tempPathVideo = path.join(FINAL_PATH, req.file.filename) + "_t." + VIDEO_CODEC;
  const finalPathArchive = path.join(TAR_PATH, req.file.filename) + "." + TAR_FORMAT;

  const x = Math.ceil( xStart * 3.07 );
  const y = Math.ceil( yStart * 2.10 );
  const w = Math.ceil( ( frameWidth * 2.4 ) * 1.2 );
  const h = Math.ceil( frameHeight * 2.4 );
	
  console.log(`Width: ${w}, Height: ${h}, X: ${x}, Y: ${y}`);

  // A great quick tutorial on ffmpeg scale and crop
  // https://www.youtube.com/watch?v=oT1ywwoUrbU
	
  // scale=240:240:flags=lanczos,fps=15 scales to 240x240 pixels, 15 frames
  // per second and no compression
  // -ac 1 combines audio channels into 1 mono channel
  // -ar 16000 down samples audio to 16,000 Mhz

  const ffmpegCmd = `/usr/local/bin/ffmpeg/ffmpeg-git-20230313-amd64-static/ffmpeg -i ${tmpPath} -ac 1 -ar 16000 -map 0:a ${finalPathAudio} -vf "crop=${w}:${h}:${x}:${y},scale=240:240:flags=lanczos,fps=15"  -q:v 9 -map 0:v ${finalPathVideo}`;
	
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
