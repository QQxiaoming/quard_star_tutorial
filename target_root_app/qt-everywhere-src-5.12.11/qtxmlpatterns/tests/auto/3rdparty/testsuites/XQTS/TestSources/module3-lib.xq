(:*******************************************************:)
(: Test: modules3-lib.xq                                 :)
(: Written By: Nicolae Brinza                            :)
(: Purpose: Some Module Definitions                      :)
(:*******************************************************:)

module namespace mod3="http://www.w3.org/TestModules/module3";

import schema namespace simple="http://www.w3.org/XQueryTest/simple";


declare variable $mod3:var1 as simple:myType := 1 cast as simple:myType;
declare variable $mod3:var2 as schema-element(simple:integer) := <simple:integer>1</simple:integer>;


declare function mod3:function1() as simple:myType
{
  1 cast as simple:myType
};

declare function mod3:function2($param as simple:myType) as xs:integer
{
  $param + 1
};


declare function mod3:function3() as schema-element(simple:integer)
{
  <simple:integer>2</simple:integer>
};


declare function mod3:function4($param as schema-element(simple:integer)) as xs:integer
{
  $param cast as xs:integer + 1
};

