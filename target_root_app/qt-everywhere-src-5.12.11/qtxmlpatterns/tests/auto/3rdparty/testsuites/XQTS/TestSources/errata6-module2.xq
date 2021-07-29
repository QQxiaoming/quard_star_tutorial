(:*******************************************************:)
(: Test: errata6-module2.xq                              :)
(: Written By: John Snelson                              :)
(: Date: 2009/10/01                                      :)
(: Purpose: Module that imports a schema and uses types  :)
(: from it                                               :)
(:*******************************************************:)

module namespace errata6_2="http://www.w3.org/TestModules/errata6-2";

import schema namespace a = "http://www.w3.org/XQueryTest/userDefinedTypes";

declare function errata6_2:hatsize-add($a, $b) as a:hatsize
{
  a:hatsize($a + $b)
};
