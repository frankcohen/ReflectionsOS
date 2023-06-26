# Reflections Recorder version 4
## Notes on installation, server operation, and code

June 4, 2023
fcohen@starlingwatch.com

This readme provides an overview of the **modifications** made to the Reflections Recorder project. It covers the enhancements and changes implemented to improve the functionality and user experience of the project.

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

## Nginx

In the file `/etc/nginx/conf.d/reflections-ssl.conf`, I made changes to the **NGINX** configuration. Specifically, I configured the application to work with the file `/var/www/node-api/public/index.html` as the main page. For API calls, I set up a `proxy_pass` directive to redirect them to `https://cloudcity.starlingwatch.com/api/`.

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
        client_max_body_size 200M;#
```

In /etc/nginx/conf.d/ssl.conf, I did this configuration is setting up a simple HTTP to HTTPS redirect on your Nginx server for the domain cloudcity.starlingwatch.com. Let's go over each directive:

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

## API

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

Upon interacting with this endpoint, you'll receive a message indicating the status of the operation. The possible responses include `File deleted` if the operation was successful, or `File not found` if the specified file does not exist.

The rest of the server configuration remains unchanged from the previous setup.

## PM2

The root of our server is located at /var/www/node-api. From this directory, we can start the server, as well as monitor the logs to troubleshoot any errors that may arise during the operation of the server.

### Starting the Server

To start the server, you can use the pm2 command, a production process manager for Node.js applications. You'll need to run the following command in your terminal:

```bash
    pm2 start ecosystem.config.js
```

This command instructs pm2 to launch the application described in the ecosystem.config.js configuration file.

### Checking the Server Status

To check the status of the server, including information about runtime, CPU usage, memory usage, etc., you can use the following pm2 command:

```bash
    pm2 status
```

This command will display the current operational state of all applications being managed by pm2.

### Monitoring Application Logs

To view the logs of the application, which can provide valuable insights into its operation and any errors that might have occurred, use the following pm2 command:

```bash
    pm2 logs
```

This command displays the real-time stdout and stderr outputs of the applications managed by pm2, which can be especially useful for debugging issues.

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

## Summary

This documentation provides a comprehensive overview of the significant modifications made to the Reflections Recorded project, each aimed at enhancing the system's performance, reliability, and user experience.

The updates to the NGINX configuration ensure a smooth transition between the server's main page and API calls, bolstering security by redirecting HTTP traffic to HTTPS. We've also enhanced the API endpoints, providing straightforward access to file operations such as viewing, downloading, and deleting files from the server.

With the use of pm2, we have a robust, streamlined, and efficient way to manage our Node.js applications. The server's status, logs, and other vital operational details are easily accessible, contributing to effective problem diagnosis and system monitoring.

The introduction of an Elastic IP to our EC2 instance has added stability and flexibility to our server access, safeguarding against potential DNS issues, facilitating rapid failover scenarios, and providing a consistent public IP address.

In essence, these modifications have significantly elevated the Reflections Recorded project's operational capacity. They not only optimize the system's current performance but also set a foundation for future enhancements and scalability.

## Notes
Recorder main page is in node-api/public/index.html
Selection mechanism is in node-api/public/cropv2.js

