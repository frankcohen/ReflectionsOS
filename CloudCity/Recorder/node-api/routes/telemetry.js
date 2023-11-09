// Importing the module
const express=require("express")
  
// Creating express Router
const router=express.Router()

router.get('/telemetry', (req, res, next) => {
        //res.json({success: 'success'});
	res.sendFile(path.join(__dirname, "../index.html"));
})

module.exports=router
