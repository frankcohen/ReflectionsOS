
# Reflections Recorder Operations Notes

Updated: November 8, 2023
(c) 2023 Frank Cohen. Published under Creative Commons license.

Reflections Recorder records video and audio using WebRTC in a browser window, uploads it to a server over HTTPS, processes the video into 240x240 mjpeg movie and audio formats, packs the video and audio into a TAR file, and downloads the TAR to a Reflections board over Wifi to play.

This readme provides an overview of modifications made to the Reflections Recorder 4 project. It covers the enhancements and changes implemented to improve the functionality and user experience of the project.

## Design Documents

[Reflections Recorder 5 requirements](https://github.com/frankcohen/ReflectionsOS/blob/main/CloudCity/Recorder/Reflections%20Recorder%205%20requirements.pdf)

[Reflections Recorder Design](https://github.com/frankcohen/ReflectionsOS/blob/main/CloudCity/Recorder/Reflections%20Recorder%20design.pdf)

## Changes Needed

The Selection Box correctly highlights the selection zone. Reflections is tested using YouTube.com.

![Recorder captures YouTube video](https://github.com/frankcohen/ReflectionsOS/blob/main/Docs/images/Recorder.jpg)

However, some Selection Box location and size values deliver video that is wrongly clipped.

![Recorder captures YouTube video](https://github.com/frankcohen/ReflectionsOS/blob/main/Docs/images/Recorder.jpg)

This may be the constant ratio values fault. Look in crop.js

node-api/public/js/crop.js, lines 143 to 144:
```
r.w = size
r.h = size * 1.2
```

and 

node-api/routes/users.js, lines 65 to 68:
```
const x = Math.ceil( xStart * 3.07 );
const y = Math.ceil( yStart * 2.10 );
const w = Math.ceil( ( frameWidth * 2.4 ) * 1.2 );
const h = Math.ceil( frameHeight * 2.4 );
```

Also, Recorder throws an error when I share a running QuickTime movie and drag the selection box. The inputs to ffmjpeg on the server are too large.

## Service Start/Stop

pm2 start index.js

```
sudo pm2 delete 0
sudo pm2 start

sudo systemctl restart nginx
sudo systemctl stop nginx
sudo systemctl start nginx
sudo systemctl enable nginx
sudo systemctl status nginx

Mongo start command
sudo mongod --port 27017 --dbpath ~/mongodb-data

Mongo connect command: 
mongosh --port 27017
```

## Paths

- /home/fcohen/files
- /home/fcohen
- /etc/nginx
- /var/www/node-api
- /etc/ssl

### Recorder UX

- Client side /var/www/node-api/public/js/main.js
- Server side to ffmpeg process video at: /var/www/node-api/routes/users.js

Temporary for logger
/home/ec2-user/.pm2/logs

### Logger

```
/var/www/node-api/index.js at /logger sends logger/dist/index.html which is react
/var/www/node-api/logger/src/text.txt appears to be the server-side tail service,
a post to https://cloudcity.starlingwatch.com/api/logging/ returns the log contents

/var/www/node-api/logger/src/
App.jsx - emits html of buttons and logger panel
LogViewer.jsx - emits html of logger panel and tests for notifications (uneeded)
index.css
text.txt - socket.io to serve the log files to the react-viewer
```

When you need to clear the log file contents, to free disk space:
```
sudo rm /home/ec2-home/.pm2/logs/index-out.log
sudo vi /home/ec2-home/.pm2/logs/index-out.log
sudo chown ec2-user index-out.log 
sudo chgrp ec2-user index-out.log 
cd /var/www/node-api
pm2 start ecosystem.config.js
```

### Telemetry with MongoDB

Telemetry is a service to save sensor data to a MongoDB collection. Access it through the Node.js APIs:

```
https://cloudcity.starlingwatch.com/telemetry/find?db=test1&field3=1
```
Executes /var/www/node-js/telemetry/scripts/find.js making the key/value pairs
available to the script. Returns 200 plus html response created in find.js, or
returns 500 error code plus error description in the body of the html.
Telemetry app locates find.js dynamically by file name, so that I can write my
own scripts and store them in telemetry/scripts.

```
https://cloudcity.starlingwatch.com/telemetry/log?db=starlingdb&collection=watches&firstname=frank&lastname=cohen&price=340
```
Inserts a document into named db and named collection using one or more URL 
encoded key/value pairs. Returns 200 response or 500 error code plus error 
description in the body of the html.

Mongo connect command: 
```
mongosh --port 27017
```

## Permissions

```
sudo groupadd mygroup
sudo usermod -a -G mygroup root
sudo usermod -a -G mygroup ec2-user
sudo chgrp -R mygroup /var/www/node-api/node_modules
sudo chgrp -R mygroup /var/www/node-api
sudo chmod -R 777 /var/www/node-api/node_modules
sudo chmod -R 777 /var/www/node-api
```

From https://stackoverflow.com/questions/48798760/npm-install-on-aws-ec2-instance-throw-eacces

## Node log timestamp utility

npm install log-timestamp
adds date/time stamp to console logs automatically

Changed code to show local time zone in time stamp:
sudo vi ./node_modules/log-timestamp/log-timestamp.js

// the default date format to print
function timestamp() {
  return '[' + new Date().toLocaleString( 'sv', { timeZoneName: 'short' } ) + ']';
}

## Nginx

In the file `/etc/nginx/conf.d/reflections-ssl.conf`, I made changes to the **NGINX** configuration. Specifically, I configured the application to work with the file `/var/www/node-api/public/index.html` as the main page. For API calls, I set up a `proxy_pass` directive to redirect them to `https://cloudcity.starlingwatch.com/api/`.

Log files at
/var/log/nginx/

Here is an example snippet from the configuration file:

```bash
#server {
    # location block for your frontend
    location / {
        root   /var/www/node-api/public/;
        index  index.html;
    }

    # location block for your API endpoints
    location /api {
        proxy_pass http://localhost:3000;
        client_max_body_size 200M;
		}
		
	# location block for your API endpoints
    location /telemetry {
        proxy_pass http://localhost:3000;
        client_max_body_size 200M;
    }		
}		
```

In /etc/nginx/conf.d/ssl.conf, HTTP to HTTPS redirect on Nginx server for the domain cloudcity.starlingwatch.com:

```bash
server {
        listen 80;
        listen [::]:80;
        server_name cloudcity.starlingwatch.com;
        return 301 https://$host$request_uri;
}
```

- `server`: This block defines a server configuration. Nginx can host multiple servers (websites) and this block of code outlines the settings for one particular server.

- `listen 80`; and `listen [::]:80;`: These two lines tell Nginx to listen for incoming connections on port 80, the default port for HTTP, on both IPv4 and IPv6 addresses.

- `server_name cloudcity.starlingwatch.com`;: This line specifies the domain name of the server that this block of configuration will apply to. In this case, the server name is set to `cloudcity.starlingwatch.com`.

- `return 301 https://$host$request_uri;`: This line handles all requests to this server by issuing a 301 redirect to the HTTPS (secure) version of the same URL. The $host variable represents the hostname from the request, and the `$request_uri` variable is the URI of the request.

To summarize, this configuration redirects all HTTP traffic to the HTTPS equivalent for the `cloudcity.starlingwatch.com` domain, improving security by ensuring all traffic to that domain is encrypted.

Front-end Javascript functions are implemented in
/var/www/node-api/public/js
cropv2.js and main.js

## APIs

Backend APIs are implemented in
/var/www/node-api/routes
index.js

The server offers various endpoints that allow you to interact with the files on the server.

#### Endpoint: `/files`

Initially, the `/files` endpoint was directed towards an `index.html` file that was non-existent in the project's root directory. This has been rectified by creating the `index.html` file in the appropriate location.

**1-** The `/files` endpoint provides a list of all files that have been uploaded to the server. Through this interface, you can download any of the available files, or delete them if needed.

**2-** The `index.html` file can be found at `/var/www/node-api/index.html`. You can access the `/files` endpoint through this [link](https://cloudcity.starlingwatch.com/api/files).

#### Endpoint: `/delete/`

A new endpoint, /delete/, has been created for file management purposes. This endpoint allows for file deletion from the /home/fcohen/ directory. There are two methods to use this endpoint:

**1-** Navigate to the /files endpoint, identify the file you want to delete, and click the 'Delete' button.

**2-** Alternatively, a file can be deleted by sending a query to the endpoint. For example:

```bash
https://cloudcity.starlingwatch.com/api/delete?name=4eadea855c363ebbcac60902dc4b8cb1.tar
```

Upon interacting with this endpoint, receive a message indicating the status of the operation. The possible responses include `File deleted` if the operation was successful, or `File not found` if the specified file does not exist.

The rest of the server configuration remains unchanged from the previous setup.

## PM2

The root of our server is /var/www/node-api. Start the node server from this directory. And, monitor the logs to troubleshoot any errors that may arise during the operation of the server.

### Starting the Server

To start the server, you can use the pm2 command, a production process manager for Node.js applications. You'll need to run the following command in your terminal:

```bash
    pm2 start ecosystem.config.js
```

This command instructs pm2 to launch the application described in the ecosystem.config.js configuration file.

### ecosysytem.config.js

```
module.exports = {
  apps : [{
    name   : "Recorder",
    script : "./index.js",
    watch: false,
    watch_delay: 1000,
    ignore_watch : ["*.jpg", "*.mjpeg", "*.m4a", "*.tar" ]
  },
  {
    script: 'index.js',
    watch: 'false',
    ignore_watch: ["*.mjpeg","*.m4a","*.tar"]
  }
  ]
}
```

### Checking the Server Status

To check the status of the server, including information about runtime, CPU usage, memory usage, etc., you can use the following pm2 command:

```bash
    pm2 status
```

This command will display the current operational state of all applications being managed by pm2.

### Monitoring Application Logs

To view the logs of the application, which can provide valuable insights into its operation and any errors that might have occurred, use the following pm2 command:

```
pm2 logs
```

This command displays the real-time stdout and stderr outputs of the applications managed by pm2, which can be especially useful for debugging issues.

## Browsing Logs using react-logviewer

pm2 keeps the logs in /home/ec2-user/.pm2/logs

Recorder-error.log, Recorder-out.log, index-error.log, index-out.log

CloudCity uses [react-logviewer](https://github.com/melloware/react-logviewer) to make the logs available from a browser at [https://cloudcity.starlingwatch.com/logger](https://cloudcity.starlingwatch.com/logger).

Wraps react-log viewer in the Reflections Recorder node.js application at /var/www/node-api/index.js

Serves from https://cloudcity.starlingwatch.com/logger. Displays logs from /home/ec2-user/.pm2/logs. Clickable tabs change view between log files. Expands to fill browser window width/height. Lazy loads. Live/streamed view of logs, no reload of page needed.

Logs support clickable URLs, search (case insensitive), multi-line copy, search next/previous.

## EC2 Instance and Elastic IP

While managing the Amazon Elastic Compute Cloud (EC2) instance, no modifications were made to the existing configurations. However, an important enhancement was made by introducing an Elastic IP.

### Elastic IP

An Elastic IP address is a static, IPv4 address designed for dynamic cloud computing. It's associated with your AWS account, not a specific instance, making it easier to mask instance or availability zone failures.

The primary purpose of assigning an Elastic IP to our EC2 instance is to enable a persistent public IP address that doesn't change every time the instance restarts. Here are the key reasons and benefits:

**1.** Stability for DNS Purpose:
Internet-based applications, especially those served via domain names, require stable IP addresses for DNS purposes. Without a stable IP, we'd need to update DNS records every time the instance is restarted, which could lead to downtime due to DNS propagation delay. With an Elastic IP, we can ensure our DNS records remain valid and stable, leading to consistent application access.

**2.** Failover Setup and Redundancy:
An Elastic IP can be quickly re-associated from a failing instance to a healthy one in the same account, ensuring high availability. It facilitates rapid failover by remapping the address to an alternate instance.

**3.** Future-proofing and Flexibility:
Elastic IPs can be allocated to your account, and you can choose when to associate them with instances, offering flexibility. Even if your requirements change, you can quickly reassign the Elastic IP to new instances as needed, ensuring your public IP address remains constant.

In our setup, by utilizing Elastic IP, we've ensured a reliable and stable connection to our EC2 instance, enhancing our system's robustness and availability.

## Conclusion

This documentation provides a comprehensive overview of the significant modifications made to the Reflections Recorded project, each aimed at enhancing the system's performance, reliability, and user experience.

The updates to the NGINX configuration ensure a smooth transition between the server's main page and API calls, bolstering security by redirecting HTTP traffic to HTTPS. We've also enhanced the API endpoints, providing straightforward access to file operations such as viewing, downloading, and deleting files from the server.

With the use of pm2, we have a robust, streamlined, and efficient way to manage our Node.js applications. The server's status, logs, and other vital operational details are easily accessible, contributing to effective problem diagnosis and system monitoring.

The introduction of an Elastic IP to our EC2 instance has added stability and flexibility to our server access, safeguarding against potential DNS issues, facilitating rapid failover scenarios, and providing a consistent public IP address.

In essence, these modifications have significantly elevated the Reflections Recorded project's operational capacity. They not only optimize the system's current performance but also set a foundation for future enhancements and scalability.

## Set Time and Date

AWS Linux instances come with [chronyd ntp service](https://docs.aws.amazon.com/AWSEC2/latest/UserGuide/set-time.html) pre-installed.

```
sudo service chronyd restart
vi /etc/chrony.conf
sudo chkconfig chronyd on
sudo timedatectl set-timezone America/Los_Angeles
timedatectl
```