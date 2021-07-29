(:*******************************************************:)
(: Test: errata6-module5.xq                              :)
(: Written By: John Snelson                              :)
(: Date: 2009/10/01                                      :)
(: Purpose: Module that imports a schema and uses types  :)
(: from it                                               :)
(:*******************************************************:)

module namespace errata6_5="http://www.w3.org/TestModules/errata6-5";

import schema namespace a = "http://www.w3.org/XQueryTest/someExamples";

declare function errata6_5:fun($a as schema-element(a:E6-Root))
{
  $a
};
