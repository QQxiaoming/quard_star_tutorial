(:*******************************************************:)
(: Test: errata6-module4.xq                              :)
(: Written By: John Snelson                              :)
(: Date: 2009/10/01                                      :)
(: Purpose: Module that imports a schema and uses types  :)
(: from it                                               :)
(:*******************************************************:)

module namespace errata6_4="http://www.w3.org/TestModules/errata6-4";

import schema namespace a = "http://www.w3.org/XQueryTest/userDefinedTypes";

declare variable $errata6_4:var as a:hatsize := a:hatsize(3);
