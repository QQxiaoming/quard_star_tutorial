(:*******************************************************:)
(: Test: errata6-module6.xq                              :)
(: Written By: John Snelson                              :)
(: Date: 2009/10/01                                      :)
(: Purpose: Module that imports a schema and uses types  :)
(: from it                                               :)
(:*******************************************************:)

module namespace errata6_6="http://www.w3.org/TestModules/errata6-6";

import schema namespace a = "http://www.w3.org/XQueryTest/someExamples2";

declare function errata6_6:fun($a as schema-attribute(a:attr))
{
  $a
};
