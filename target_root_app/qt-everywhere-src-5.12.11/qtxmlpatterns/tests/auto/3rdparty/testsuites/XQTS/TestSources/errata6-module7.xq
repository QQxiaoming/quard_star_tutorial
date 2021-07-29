(:*******************************************************:)
(: Test: errata6-module7.xq                              :)
(: Written By: John Snelson                              :)
(: Date: 2009/10/01                                      :)
(: Purpose: Module that imports a schema and uses types  :)
(: from it                                               :)
(:*******************************************************:)

module namespace errata6_7="http://www.w3.org/TestModules/errata6-7";

import schema namespace a = "http://www.w3.org/XQueryTest/someExamples";

declare function errata6_7:fun($a as element(*, a:weather))
{
  $a
};
