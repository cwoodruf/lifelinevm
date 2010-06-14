<?php
$schema['boxes'] = array(
  'box' => array( 'datatype' => "varchar(32) NOT NULL default ''" ),
  'seccode' => array( 'datatype' => "varchar(32) default NULL" ),
  'email' => array( 'datatype' => "varchar(64) default NULL" ),
  'paidto' => array( 'datatype' => "date default NULL" ),
  'inuse' => array( 'datatype' => "int(11) default NULL" ),
  'new_msgs' => array( 'datatype' => "int(11) default NULL" ),
  'status' => array( 'datatype' => "varchar(32) default ''" ),
  'vid' => array( 'datatype' => "int(11) default NULL" ),
  'login' => array( 'datatype' => "varchar(32) default NULL" ),
  'created' => array( 'datatype' => "timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP" ),
  'modified' => array( 'datatype' => "timestamp NOT NULL default '0000-00-00 00:00:00'" ),
);

$schema['calls'] = array(
  'box' => array( 'datatype' => "varchar(32) default NULL" ),
  'call_time' => array( 'datatype' => "timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP" ),
  'action' => array( 'datatype' => "varchar(32) default NULL" ),
  'status' => array( 'datatype' => "varchar(32) default NULL" ),
  'message' => array( 'datatype' => "text" ),
);

$schema['invoices'] = array(
  'invoice' => array( 'datatype' => "int(11) NOT NULL auto_increment" ),
  'created' => array( 'datatype' => "timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP" ),
  'paidon' => array( 'datatype' => "timestamp NOT NULL default '0000-00-00 00:00:00'" ),
  'months' => array( 'datatype' => "int(11) default NULL" ),
  'gst' => array( 'datatype' => "float default NULL" ),
  'pst' => array( 'datatype' => "float default NULL" ),
  'total' => array( 'datatype' => "float default NULL" ),
  'notes' => array( 'datatype' => "mediumtext" ),
  'vid' => array( 'datatype' => "int(11) default NULL" ),
  'login' => array( 'datatype' => "varchar(32) default NULL" ),
);

$schema['transactions'] = array(
  'trans' => array( 'datatype' => "varchar(128) NOT NULL default ''" ),
  'time' => array( 'datatype' => "timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP" ),
  'vid' => array( 'datatype' => "int(11) default NULL" ),
);

$schema['updates'] = array(
  'box' => array( 'datatype' => "varchar(32) default NULL" ),
  'months' => array( 'datatype' => "int(11) default NULL" ),
  'oldpaidto' => array( 'datatype' => "date default NULL" ),
  'newpaidto' => array( 'datatype' => "date default NULL" ),
  'paycode' => array( 'datatype' => "varchar(32) default NULL" ),
  'updated' => array( 'datatype' => "timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP" ),
  'login' => array( 'datatype' => "varchar(32) default NULL" ),
  'vid' => array( 'datatype' => "int(11) default NULL" ),
  'action' => array( 'datatype' => "varchar(128) default NULL" ),
  'app' => array( 'datatype' => "varchar(128) default NULL" ),
);

$schema['users'] = array(
  'login' => array( 'datatype' => "varchar(32) default NULL" ),
  'password' => array( 'datatype' => "varchar(32) default NULL" ),
  'created' => array( 'datatype' => "timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP" ),
  'vid' => array( 'datatype' => "int(11) default NULL" ),
  'perms' => array( 'datatype' => "varchar(32) default NULL" ),
);

$schema['vendors'] = array(
  'vid' => array( 'datatype' => "int(11) NOT NULL auto_increment" ),
  'vendor' => array( 'datatype' => "varchar(128) NOT NULL default ''" ),
  'created' => array( 'datatype' => "timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP" ),
  'parent' => array( 'datatype' => "varchar(128) default NULL" ),
  'address' => array( 'datatype' => "varchar(128) default NULL" ),
  'phone' => array( 'datatype' => "varchar(128) default NULL" ),
  'contact' => array( 'datatype' => "varchar(128) default NULL" ),
  'email' => array( 'datatype' => "varchar(128) default NULL" ),
  'fax' => array( 'datatype' => "varchar(128) default NULL" ),
  'gstexempt' => array( 'datatype' => "int(11) default '0'" ),
  'rate' => array( 'datatype' => "float default NULL" ),
  'months' => array( 'datatype' => "int(11) default '0'" ),
  'all_months' => array( 'datatype' => "int(11) default '0'" ),
  'gst_number' => array( 'datatype' => "varchar(128) default NULL" ),
  'credit_limit' => array( 'datatype' => "float default NULL" ),
  'status' => array( 'datatype' => "varchar(32) default ''" ),
  'notes' => array( 'datatype' => "text" ),
);
