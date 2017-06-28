## Install CentOS

Initial screen

- Set enp7s0 to `ON`
- Set enp8s0 to `ON`
- Set hostname `me344-cluster-1.stanford.edu`

*enp7s0*

- General Tab
    - Enable "Automatically Connect to this network..."
- IPv4 Settings
    - IP Address = 171.64.116.61
    - Mask = 255.255.254.0
    - GW = 171.64.116.1
    - DNS = 171.64.1.234

*enp8s0*

- General Tab
    - Enable "Automatically Connect to this network..."
- IPv4 Settings
    - IP Address = 10.1.1.1
    - Mask = 255.0.0.0

## Cluster Setup

Check SELinux

    sestatus

Disable SELinux

    perl -pi -e "s/SELINUX=enforcing/SELINUX=disabled/" /etc/selinux/config
    reboot

Verify SElinux is disabled

    sestatus

Update system

    yum -y update

Install OHPC repo (it also installs EPEL repo)

    yum -y install http://build.openhpc.community/OpenHPC:/1.3/CentOS_7/x86_64/ohpc-release-1.3-1.el7.x86_64.rpm
    yum repolist

Add provisioning services on master node

    yum -y groupinstall ohpc-base
    yum -y groupinstall ohpc-warewulf

Disable firewall

    systemctl disable firewalld
    systemctl stop firewalld

Add resource manager

    yum -y install pbspro-server-ohpc

Configure warewulf provisioning internal interface

    perl -pi -e "s/device = eth1/device = enp8s0/" /etc/warewulf/provision.conf

Enable tftp service for compute node image distribution

    perl -pi -e "s/^\s+disable\s+= yes/ disable = no/" /etc/xinetd.d/tftp

Enable http access for Warewulf cgi-bin directory to support newer apache syntax

    export MODFILE=/etc/httpd/conf.d/warewulf-httpd.conf
    perl -pi -e "s/cgi-bin>\$/cgi-bin>\n Require all granted/" $MODFILE
    perl -pi -e "s/Allow from all/Require all granted/" $MODFILE
    perl -ni -e "print unless /^\s+Order allow,deny/" $MODFILE

Restart/enable relevant services to support provisioning

    systemctl restart xinetd
    systemctl enable mariadb.service
    systemctl restart mariadb
    systemctl enable httpd.service
    systemctl restart httpd

Define chroot location

    export CHROOT=/opt/ohpc/admin/images/centos7.3
     echo "export CHROOT=/opt/ohpc/admin/images/centos7.3" >> /root/.bashrc

Build initial chroot image

    wwmkchroot centos-7 $CHROOT

Update CentOS

    yum -y --installroot=$CHROOT update

Install the base package group

    yum -y --installroot=$CHROOT groupinstall base

Add OpenHPC components

    yum -y --installroot=$CHROOT groupinstall ohpc-base
    yum -y --installroot=$CHROOT groupinstall ohpc-base-compute

Enable DNS on nodes

    cp -p /etc/resolv.conf $CHROOT/etc/resolv.conf

Edit /etc/hosts -- Add in entry for public/private hostname mapping to IP address [APPEND!]

    echo "171.64.116.61 me344-cluster-1.stanford.edu" >> /etc/hosts
    echo "10.1.1.1 me344-cluster-1.localdomain me344-cluster-1" >> /etc/hosts

Add PBS Professional client support

    yum -y --installroot=$CHROOT install pbspro-execution-ohpc
    perl -pi -e "s/PBS_SERVER=\S+/PBS_SERVER=me344-cluster-1.stanford.edu/" /etc/pbs.conf
    perl -pi -e "s/PBS_SERVER=\S+/PBS_SERVER=me344-cluster-1.stanford.edu/" $CHROOT/etc/pbs.conf
    perl -pi -e "s/\$clienthost \S+/\$clienthost me344-cluster-1.stanford.edu/" $CHROOT/var/spool/pbs/mom_priv/config
    echo "\$usecp *:/home /home" >> $CHROOT/var/spool/pbs/mom_priv/config
    chroot $CHROOT opt/pbs/libexec/pbs_habitat
    chroot $CHROOT systemctl enable pbs

Add Network Time Protocol (NTP) support

    yum -y --installroot=$CHROOT install ntp

Add kernel drivers

    yum -y --installroot=$CHROOT install kernel

add new cluster key to base image

    wwinit database
    wwinit ssh_keys
    cat ~/.ssh/cluster.pub >> $CHROOT/root/.ssh/authorized_keys

add NFS client mounts of /home and /opt/ohpc/pub to base image

    echo "10.1.1.1:/home /home nfs nfsvers=3,rsize=1024,wsize=1024,cto 0 0" >> $CHROOT/etc/fstab
    echo "10.1.1.1:/opt/ohpc/pub /opt/ohpc/pub nfs nfsvers=3 0 0" >> $CHROOT/etc/fstab

Export /home and OpenHPC public packages from master server to cluster compute nodes

    echo "/home *(rw,no_subtree_check,fsid=10,no_root_squash)" >> /etc/exports
    echo "/opt/ohpc/pub *(ro,no_subtree_check,fsid=11)" >> /etc/exports
    exportfs -a
    systemctl restart nfs
    systemctl enable nfs-server

Enable NTP time service on computes and identify master host as local NTP server

    chroot $CHROOT systemctl enable ntpd
    echo "server 10.1.1.1" >> $CHROOT/etc/ntp.conf

Update memlock settings on master

    perl -pi -e 's/# End of file/\* soft memlock unlimited\n$&/s' /etc/security/limits.conf
    perl -pi -e 's/# End of file/\* hard memlock unlimited\n$&/s' /etc/security/limits.conf

Update memlock settings within compute image

    perl -pi -e 's/# End of file/\* soft memlock unlimited\n$&/s' $CHROOT/etc/security/limits.conf
    perl -pi -e 's/# End of file/\* hard memlock unlimited\n$&/s' $CHROOT/etc/security/limits.conf

Enable forwarding of system logs Configure SMS to receive messages and reload rsyslog configuration

    perl -pi -e "s/\\#\\\$ModLoad imudp/\\\$ModLoad imudp/" /etc/rsyslog.conf
    perl -pi -e "s/\\#\\\$UDPServerRun 514/\\\$UDPServerRun 514/" /etc/rsyslog.conf
    systemctl restart rsyslog

Define compute node forwarding destination

    echo "*.* @10.1.1.1:514" >> $CHROOT/etc/rsyslog.conf

Disable most local logging on computes. Emergency and boot logs will remain on the compute nodes

    perl -pi -e "s/^\*\.info/\\#\*\.info/" $CHROOT/etc/rsyslog.conf
    perl -pi -e "s/^authpriv/\\#authpriv/" $CHROOT/etc/rsyslog.conf
    perl -pi -e "s/^mail/\\#mail/" $CHROOT/etc/rsyslog.conf
    perl -pi -e "s/^cron/\\#cron/" $CHROOT/etc/rsyslog.conf
    perl -pi -e "s/^uucp/\\#uucp/" $CHROOT/etc/rsyslog.conf

Import files

    wwsh file import /etc/passwd
    wwsh file import /etc/group
    wwsh file import /etc/shadow

Build bootstrap image

    wwbootstrap `uname -r`

Assemble Virtual Node File System (VNFS) image

    wwvnfs --chroot $CHROOT

Set provisioning interface as the default networking device

    echo "GATEWAYDEV=eth0" > /tmp/network.$$
    wwsh -y file import /tmp/network.$$ --name network
    wwsh -y file set network --path /etc/sysconfig/network --mode=0644 --uid=0

Add nodes to Warewulf data store:

    wwsh -y node new compute-1-1 --ipaddr=10.1.1.10 --hwaddr=A4:BF:01:15:36:38
    wwsh -y node new compute-1-2 --ipaddr=10.1.2.10 --hwaddr=A4:BF:01:15:37:7D
    wwsh -y node new compute-1-3 --ipaddr=10.1.3.10 --hwaddr=A4:BF:01:15:36:A6
    wwsh -y node new compute-1-4 --ipaddr=10.1.4.10 --hwaddr=A4:BF:01:15:34:B7

    wwsh -y node new ipmi-compute-1-1 --ipaddr=10.1.1.11 --hwaddr=A4:BF:01:15:36:3A
    wwsh -y node new ipmi-compute-1-2 --ipaddr=10.1.2.11 --hwaddr=A4:BF:01:15:37:7F
    wwsh -y node new ipmi-compute-1-3 --ipaddr=10.1.3.11 --hwaddr=A4:BF:01:15:36:A8
    wwsh -y node new ipmi-compute-1-4 --ipaddr=10.1.4.11 --hwaddr=A4:BF:01:15:34:B9

Define provisioning image for hosts

    wwsh -y provision set "compute-*" --vnfs=centos7.3 --bootstrap=`uname -r` --files=dynamic_hosts,passwd,group,shadow,network

Restart dhcp / update PXE

    systemctl restart dhcpd
    wwsh pxe update

Start compute nodes with IPMITOOL

    ipmitool -H 10.1.1.11 -U root -P superuser chassis power cycle
    ipmitool -H 10.1.2.11 -U root -P superuser chassis power cycle
    ipmitool -H 10.1.3.11 -U root -P superuser chassis power cycle
    ipmitool -H 10.1.4.11 -U root -P superuser chassis power cycle

You can view progress of the installation with the following command line option:

    tail -f /var/log/messages

To end, use CTRL+C

Install Development Tools

    yum -y groupinstall ohpc-autotools
    yum -y install valgrind-ohpc

Install Compilers

    yum -y install gnu-compilers-ohpc

Install MPI stacks

    yum -y install openmpi-gnu-ohpc mvapich2-gnu-ohpc mpich-gnu-ohpc

Install performance tools

    yum -y groupinstall ohpc-perf-tools-gnu

Setup default development environment

    yum -y install lmod-defaults-gnu-mvapich2-ohpc

Start pbspro daemons on master host

    systemctl enable pbs
    systemctl start pbs

Initialize PBS path

    . /etc/profile.d/pbs.sh

Enable user environment propagation (needed for modules support)

    qmgr -c "set server default_qsub_arguments= -V"

Enable uniform multi-node MPI task distribution

    qmgr -c "set server resources_default.place=scatter"

Register compute hosts with pbspro

    qmgr -c "create node compute-1-1"
    qmgr -c "create node compute-1-2"
    qmgr -c "create node compute-1-3"
    qmgr -c "create node compute-1-4"

Kerberos Auth

    scp [SUNETID]@knl-login-a:/etc/krb5.conf /etc/
    authconfig --enablekrb5 --update
    ntpdate time.stanford.edu

Add users

    useradd -m [SUNETID]

Push changes to compute nodes:

    wwsh file resync passwd shadow group

Shouldn't be necessary, but this command pulls changes from the master node:

    pdsh -w compute-1-[1-4] /warewulf/bin/wwgetfiles

## Quick checks...

Switch to a user account:

    su - [SUNETID]

Run the uptime command on 4 compute nodes:

    pdsh -w compute-1-[1-4] uptime

Output should resemble this:

    compute-1-3:  12:40:53 up 11 min,  0 users,  load average: 0.00, 0.10, 0.13
    compute-1-4:  12:40:12 up 10 min,  0 users,  load average: 0.00, 0.08, 0.11
    compute-1-1:  12:41:00 up 11 min,  0 users,  load average: 0.01, 0.09, 0.11
    compute-1-2:  12:40:53 up 11 min,  0 users,  load average: 0.00, 0.09, 0.12

Check resource manager:

    qsub -I -lnodes=4:ppn=272
    cat $PBS_NODEFILE |uniq

Output should look like this:

    compute-1-1.localdomain
    compute-1-2.localdomain
    compute-1-3.localdomain
    compute-1-4.localdomain
