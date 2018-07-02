## Install CentOS

Initial screen

- Set eno1 to `ON`
- Set eno2 to `ON`

Interface **eno1**

- General Tab
    - Enable "Automatically Connect to this network..."

Interface **eno2**

- General Tab
    - Enable "Automatically Connect to this network..."
- IPv4 Settings
    - IP Address = 10.1.1.1
    - Mask = 255.0.0.0

## Cluster Setup

Log onto cluster 

    # ssh root@me344-cluster-[N].stanford.edu

Modify /etc/hosts -- Add entry for public/private hostname mapping to IP address 

    # echo "171.64.116.[XXX] me344-cluster-[X].stanford.edu" >> /etc/hosts
    # echo "10.1.1.1 me344-cluster-[X].localdomain me344-cluster-[X]" >> /etc/hosts

    171.64.116.61 me344-cluster-1.stanford.edu
    171.64.116.154 me344-cluster-2.stanford.edu
    171.64.116.159 me344-cluster-3.stanford.edu
    171.64.116.190 me344-cluster-4.stanford.edu
    171.64.116.195 me344-cluster-5.stanford.edu

Check SELinux 

    # sestatus

Disable SELinux and default route on internal network

    # perl -pi -e "s/SELINUX=enforcing/SELINUX=disabled/" /etc/selinux/config
    # perl -pi -e "s/DEFROUTE=yes/DEFROUTE=no/" /etc/sysconfig/network-scripts/ifcfg-eno2
    # reboot

Verify SElinux is disabled 

    # sestatus

Disable firewall 

    # systemctl disable firewalld
    # systemctl stop firewalld

Update system 

    # yum -y update

Install OHPC repo (it also installs EPEL repo) 

    # yum -y install http://build.openhpc.community/OpenHPC:/1.3/CentOS_7/x86_64/ohpc-release-1.3-1.el7.x86_64.rpm 
    # yum repolist

Add provisioning services on master node 

    # yum -y groupinstall ohpc-base
    # yum -y groupinstall ohpc-warewulf

Configure time services 

    # systemctl enable ntpd.service
    # echo "server time.stanford.edu" >> /etc/ntp.conf
    # systemctl restart ntpd

Install slurm server meta-package 

    # yum -y install ohpc-slurm-server

Identify resource manager hostname on master host 

    # perl -pi -e "s/ControlMachine=\S+/ControlMachine=me344-cluster-[X]/" /etc/slurm/slurm.conf

Change default node state for nodes returning to service (automatically return to service if all resources report correctly)

    # perl -pi -e "s/ReturnToService=1/ReturnToService=2/" /etc/slurm/slurm.conf

Enabled Slurm controller and Munge

    # systemctl enable slurmctld
    # systemctl start munge

Configure warewulf provisioning internal interface 

    # perl -pi -e "s/device = eth1/device = eno2/" /etc/warewulf/provision.conf

Enable tftp service for compute node image distribution 

    # perl -pi -e "s/^\s+disable\s+= yes/ disable = no/" /etc/xinetd.d/tftp

Restart/enable relevant services to support provisioning 

    # systemctl restart xinetd
    # systemctl enable mariadb.service
    # systemctl restart mariadb
    # systemctl enable httpd.service
    # systemctl restart httpd
    # systemctl enable dhcpd.service

Define chroot location 

    # export CHROOT=/opt/ohpc/admin/images/centos7.5

Add to your local environment for future use 

    # echo "export CHROOT=/opt/ohpc/admin/images/centos7.5" >> /root/.bashrc

Build initial chroot image 

    # wwmkchroot centos-7 $CHROOT

Update CentOS 

    # yum -y --installroot=$CHROOT update

Install compute node base meta-package 

    # yum -y --installroot=$CHROOT install ohpc-base-compute

Enable DNS on nodes 

    # cp -p /etc/resolv.conf $CHROOT/etc/resolv.conf

Add Slurm client support meta-package 

    # yum -y --installroot=$CHROOT install ohpc-slurm-client

Add Network Time Protocol (NTP) support 

    # yum -y --installroot=$CHROOT install ntp

Add kernel drivers 

    # yum -y --installroot=$CHROOT install kernel

Include modules user environment and ipmitool 

    # yum -y --installroot=$CHROOT install lmod-ohpc ipmitool

Initialize warewulf database and ssh_keys 

    # wwinit database
    # wwinit ssh_keys
    # cat ~/.ssh/cluster.pub >> $CHROOT/root/.ssh/authorized_keys

Add NFS client mounts of /home and /opt/ohpc/pub to base image 

    # echo "10.1.1.1:/home /home nfs nfsvers=3,nodev,nosuid,noatime 0 0" >> $CHROOT/etc/fstab
    # echo "10.1.1.1:/opt/ohpc/pub /opt/ohpc/pub nfs nfsvers=3,nodev,noatime 0 0" >> $CHROOT/etc/fstab

Export /home and OpenHPC public packages from master server 

    # echo "/home *(rw,no_subtree_check,fsid=10,no_root_squash)" >> /etc/exports
    # echo "/opt/ohpc/pub *(ro,no_subtree_check,fsid=11)" >> /etc/exports
    # exportfs -a
    # systemctl restart nfs-server
    # systemctl enable nfs-server

Enable NTP time service on computes and identify master host as local NTP server 

    # chroot $CHROOT systemctl enable ntpd
    # echo "server 10.1.1.1" >> $CHROOT/etc/ntp.conf

Define compute node forwarding destination 

    # echo "*.* @10.1.1.1:514" >> $CHROOT/etc/rsyslog.conf

Disable most local logging on computes. Emergency and boot logs will remain on the compute nodes 

    # perl -pi -e "s/^\*\.info/\\#\*\.info/" $CHROOT/etc/rsyslog.conf
    # perl -pi -e "s/^authpriv/\\#authpriv/" $CHROOT/etc/rsyslog.conf
    # perl -pi -e "s/^mail/\\#mail/" $CHROOT/etc/rsyslog.conf
    # perl -pi -e "s/^cron/\\#cron/" $CHROOT/etc/rsyslog.conf
    # perl -pi -e "s/^uucp/\\#uucp/" $CHROOT/etc/rsyslog.conf

Import files 

    # wwsh file import /etc/passwd
    # wwsh file import /etc/group
    # wwsh file import /etc/shadow
    # wwsh file import /etc/slurm/slurm.conf
    # wwsh file import /etc/munge/munge.key

Set provisioning interface as the default networking device 

    # echo "GATEWAYDEV=eth0" > /tmp/network.$$
    # echo "GATEWAY=10.1.1.1" >> /tmp/network.$$
    # wwsh -y file import /tmp/network.$$ --name network
    # wwsh -y file set network --path /etc/sysconfig/network --mode=0644 --uid=0

Build bootstrap image 

    # wwbootstrap `uname -r`

Assemble VNFS image 

    # wwvnfs --chroot $CHROOT

Add nodes to Warewulf data store: 

CLUSTER 1: 

    wwsh -y node new compute-1-1 --ipaddr=10.1.1.10 --hwaddr=00:a0:d1:ee:b5:9c
    wwsh -y node new compute-1-2 --ipaddr=10.1.2.10 --hwaddr=00:a0:d1:ee:ba:ac
    wwsh -y node new compute-1-3 --ipaddr=10.1.3.10 --hwaddr=00:a0:d1:ee:b5:f4
    wwsh -y node new compute-1-4 --ipaddr=10.1.4.10 --hwaddr=00:a0:d1:ee:bb:e0
    
    wwsh -y node new ipmi-compute-1-1 --ipaddr=10.1.1.11 --hwaddr=00:a0:d1:ee:b5:9f
    wwsh -y node new ipmi-compute-1-2 --ipaddr=10.1.2.11 --hwaddr=00:a0:d1:ee:ba:af
    wwsh -y node new ipmi-compute-1-3 --ipaddr=10.1.3.11 --hwaddr=00:a0:d1:ee:b5:f7
    wwsh -y node new ipmi-compute-1-4 --ipaddr=10.1.4.11 --hwaddr=00:a0:d1:ee:bb:e3

CLUSTER 2: 

    wwsh -y node new compute-1-1 --ipaddr=10.1.1.10 --hwaddr=00:a0:d1:ee:97:14
    wwsh -y node new compute-1-2 --ipaddr=10.1.2.10 --hwaddr=00:a0:d1:ee:cf:88
    wwsh -y node new compute-1-3 --ipaddr=10.1.3.10 --hwaddr=00:a0:d1:ee:c8:78
    wwsh -y node new compute-1-4 --ipaddr=10.1.4.10 --hwaddr=00:a0:d1:ee:cc:00
    
    wwsh -y node new ipmi-compute-1-1 --ipaddr=10.1.1.11 --hwaddr=00:a0:d1:ee:97:17
    wwsh -y node new ipmi-compute-1-2 --ipaddr=10.1.2.11 --hwaddr=00:a0:d1:ee:cf:8b
    wwsh -y node new ipmi-compute-1-3 --ipaddr=10.1.3.11 --hwaddr=00:a0:d1:ee:c8:7b
    wwsh -y node new ipmi-compute-1-4 --ipaddr=10.1.4.11 --hwaddr=00:a0:d1:ee:cc:03

CLUSTER 3:

    wwsh -y node new compute-1-1 --ipaddr=10.1.1.10 --hwaddr=00:a0:d1:ee:cd:70
    wwsh -y node new compute-1-2 --ipaddr=10.1.2.10 --hwaddr=00:a0:d1:ee:c9:b4
    wwsh -y node new compute-1-3 --ipaddr=10.1.3.10 --hwaddr=00:a0:d1:ee:de:b0
    wwsh -y node new compute-1-4 --ipaddr=10.1.4.10 --hwaddr=00:a0:d1:ee:de:00
    
    wwsh -y node new ipmi-compute-1-1 --ipaddr=10.1.1.11 --hwaddr=00:a0:d1:ee:cd:73
    wwsh -y node new ipmi-compute-1-2 --ipaddr=10.1.2.11 --hwaddr=00:a0:d1:ee:c9:b7
    wwsh -y node new ipmi-compute-1-3 --ipaddr=10.1.3.11 --hwaddr=00:a0:d1:ee:de:b3
    wwsh -y node new ipmi-compute-1-4 --ipaddr=10.1.4.11 --hwaddr=00:a0:d1:ee:de:03

CLUSTER 4:

    wwsh -y node new compute-1-1 --ipaddr=10.1.1.10 --hwaddr=00:a0:d1:ee:cd:00
    wwsh -y node new compute-1-2 --ipaddr=10.1.2.10 --hwaddr=00:a0:d1:ee:c9:bc
    wwsh -y node new compute-1-3 --ipaddr=10.1.3.10 --hwaddr=00:a0:d1:ee:cb:1c
    wwsh -y node new compute-1-4 --ipaddr=10.1.4.10 --hwaddr=00:a0:d1:ee:ca:b4
    
    wwsh -y node new ipmi-compute-1-1 --ipaddr=10.1.1.11 --hwaddr=00:a0:d1:ee:cd:03
    wwsh -y node new ipmi-compute-1-2 --ipaddr=10.1.2.11 --hwaddr=00:a0:d1:ee:c9:bf
    wwsh -y node new ipmi-compute-1-3 --ipaddr=10.1.3.11 --hwaddr=00:a0:d1:ee:cb:1f
    wwsh -y node new ipmi-compute-1-4 --ipaddr=10.1.4.11 --hwaddr=00:a0:d1:ee:ca:b7

CLUSTER 5:

    wwsh -y node new compute-1-1 --ipaddr=10.1.1.10 --hwaddr=00:a0:d1:ee:de:24
    wwsh -y node new compute-1-2 --ipaddr=10.1.2.10 --hwaddr=00:a0:d1:ee:b0:c4
    wwsh -y node new compute-1-3 --ipaddr=10.1.3.10 --hwaddr=00:a0:d1:ee:d1:7c
    wwsh -y node new compute-1-4 --ipaddr=10.1.4.10 --hwaddr=00:a0:d1:ee:ca:d4
    
    wwsh -y node new ipmi-compute-1-1 --ipaddr=10.1.1.11 --hwaddr=00:a0:d1:ee:de:27
    wwsh -y node new ipmi-compute-1-2 --ipaddr=10.1.2.11 --hwaddr=00:a0:d1:ee:b0:c7
    wwsh -y node new ipmi-compute-1-3 --ipaddr=10.1.3.11 --hwaddr=00:a0:d1:ee:d1:7f
    wwsh -y node new ipmi-compute-1-4 --ipaddr=10.1.4.11 --hwaddr=00:a0:d1:ee:ca:d7

Define provisioning image for hosts 

    # wwsh -y provision set "compute-*" --vnfs=centos7.5 --bootstrap=`uname -r` --files=dynamic_hosts,passwd,group,shadow,network,slurm.conf,munge.key

Restart dhcp / update PXE 

    # systemctl restart dhcpd
    # wwsh pxe update

Start compute nodes with IPMITOOL 

    # ipmitool -H 10.1.1.11 -U admin -P superuser chassis power cycle
    # ipmitool -H 10.1.2.11 -U admin -P superuser chassis power cycle
    # ipmitool -H 10.1.3.11 -U admin -P superuser chassis power cycle
    # ipmitool -H 10.1.4.11 -U admin -P superuser chassis power cycle

You can view progress of the installation with the following command line option: 

    # tail -f /var/log/messages

To end, use CTRL+C 

Kerberos Auth 

    # yum -y install pam_krb5
    # scp [SUNETID]@knl-login:/etc/krb5.conf /etc/
    # authconfig --enablekrb5 --update

Add users 

    # useradd -m [SUNETID]

Push changes to compute nodes: 

    # wwsh file resync passwd shadow group

Shouldn't be necessary, but this command pulls changes from the master node: 

    # pdsh -w compute-1-[1-4] /warewulf/bin/wwgetfiles

Quick checks... 

Switch to a user account: 

    # su - [SUNETID]

Run the uptime command on 4 compute nodes: 

    $ pdsh -w compute-1-[1-4] uptime
 
 Output should resemble this:
 
    compute-1-3:  12:40:53 up 11 min,  0 users,  load average: 0.00, 0.10, 0.13
    compute-1-4:  12:40:12 up 10 min,  0 users,  load average: 0.00, 0.08, 0.11
    compute-1-1:  12:41:00 up 11 min,  0 users,  load average: 0.01, 0.09, 0.11
    compute-1-2:  12:40:53 up 11 min,  0 users,  load average: 0.00, 0.09, 0.12

# Exercise #1
Add Ganglia Monitoring to your cluster. Instructions are located in the Open HPC Installation Guide for CentOS using Werewolf and Slurm, located here:
https://github.com/openhpc/ohpc/releases/download/v1.3.5.GA/Install_guide-CentOS7-Warewulf-SLURM-1.3.5-x86_64.pdf 

# Exercise #2
Verify Slurm is installed and reporting all nodes in an "Idle" state. You can execute the following command to show nodes in an "Idle" state: 

    # sinfo -t idle

You can also invoke the sinfo command with no arguments 

Output should be similar to this: 

    sinfo 
    PARTITION AVAIL  TIMELIMIT  NODES  STATE NODELIST
    normal*      up   infinite      4   idle compute-1-[1-4]

If not working, here are some things to consider... 

    1. Is slurm enabled and started on master and compute nodes?
    2. If nodes are in a "Drain" state, have you checked the following
       a. /var/log/slurmctld.log
       b. scontrol show node [nodename]
    3. Have you defined everything correctly in the Slurm configuration file?

# Exercise #3
When you have completed the installation of your cluster, including exercise #1 and #2, re-install CentOS on your master node and complete an automated installation of Open HPC. The intent of this project is to re-create the cluster built in the steps above, including steps from exercise #1 and #2, using the automated cluster build process. The general steps are: 

Log onto cluster 

    # ssh root@me344-cluster-[N].stanford.edu

Modify /etc/hosts -- Add entry for public/private hostname mapping to IP address 

    # echo "171.64.116.[XXX] me344-cluster-[X].stanford.edu" >> /etc/hosts
    # echo "10.1.1.1 me344-cluster-[X].localdomain me344-cluster-[X]" >> /etc/hosts

Disable SELinux and default route on internal network

    # perl -pi -e "s/SELINUX=enforcing/SELINUX=disabled/" /etc/selinux/config
    # perl -pi -e "s/DEFROUTE=yes/DEFROUTE=no/" /etc/sysconfig/network-scripts/ifcfg-eno2
    # reboot

Install OHPC repo (it also installs EPEL repo) 

    # yum -y install http://build.openhpc.community/OpenHPC:/1.3/CentOS_7/x86_64/ohpc-release-1.3-1.el7.x86_64.rpm 

Install OHPC docs RPM (includes input and recipe for automated installation) 

    # yum -y install docs-ohpc

The input file for the automated installation is located at: 

     /opt/ohpc/pub/doc/recipes/centos7/input.local

Fix issue in recipe.sh for IPMI password

    # sed -i -e "s/chassis power reset/-P \${bmc_password} chassis power reset/g" /opt/ohpc/pub/doc/recipes/centos7/x86_64/warewulf/slurm/recipe.sh

Once you have modified input.local with correct values for your cluster, you can execute the automated installation file located at 

    # sh /opt/ohpc/pub/doc/recipes/centos7/x86_64/warewulf/slurm/recipe.sh

You can compare items in recipe.sh and the instructions used to build your initial cluster to validate values used for input.local

NOTE: There are issues with recipe.sh related to Slurm. You need to correctly define compute nodes in slurm.conf, then something along the following (after completion of running recipe.sh)

    # chroot /opt/ohpc/admin/images/centos7.5 systemctl enable slurmd
    # wwvnfs --chroot=/opt/ohpc/admin/images/centos7.5

Make appropriate changes to slurm.conf

    # wwsh file sync slurm.conf
    # systemctl restart slurmctld
    # wwsh ssh c* reboot

# Bonus Exercise - earn up to 5% to apply towards your grade!

You can earn a bonus of up to 5% to apply towards your grade by submitting a working fix for the above IPMI and Slurm issue in recipe.sh

The fix needs to be implemented in a copy of /opt/ohpc/pub/doc/recipes/centos7/x86_64/warewulf/slurm/recipe.sh with a difference file created and submitted. For example, you may do the following:

    $ cd
    $ cp /opt/ohpc/pub/doc/recipes/centos7/x86_64/warewulf/slurm/recipe.sh recipe-fix.sh

Make changes to recipe-fix.sh and print differences to screen

    $ diff /opt/ohpc/pub/doc/recipes/centos7/x86_64/warewulf/slurm/recipe.sh recipe-fix.sh

Your differences may look similar to this if you find the username "test" and replace with "sunetid"

    $ diff /opt/ohpc/pub/doc/recipes/centos7/x86_64/warewulf/slurm/recipe.sh recipe-fix.sh
    503c503
    < useradd -m test
    ---
    > useradd -m sunetid

This is an example of what to include with your submission for consideration of bonus to apply towards your grade.
