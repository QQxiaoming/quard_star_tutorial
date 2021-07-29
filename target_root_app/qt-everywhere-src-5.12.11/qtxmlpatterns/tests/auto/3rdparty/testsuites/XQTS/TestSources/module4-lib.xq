(:*******************************************************:)
(: Test: modules4-lib.xq                                 :)
(: Written By: Nicolae Brinza                            :)
(: Purpose: Some Module Definitions                      :)
(:*******************************************************:)

module namespace defs = "http://www.w3.org/TestModules/module4";

import schema namespace sample = "http://www.w3.org/XQueryTest/sample"; 


declare variable $defs:var as schema-attribute(sample:attrib)? := (); 


declare function defs:function1() as schema-attribute(sample:attrib)?
{
  ()
};


declare function defs:function2($param as schema-attribute(sample:attrib)?) as xs:integer?
{
  $param cast as xs:integer? + 1 
};

