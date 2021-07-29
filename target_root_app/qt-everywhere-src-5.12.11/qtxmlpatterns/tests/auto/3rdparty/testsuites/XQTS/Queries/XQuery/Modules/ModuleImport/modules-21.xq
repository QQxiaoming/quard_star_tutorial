(:*******************************************************:)
(: Test: modules-21.xq                                   :)
(: Written By: Nicolae Brinza                            :)
(: Purpose: Load a library but do not import the schema  :)
(:*******************************************************:)

import module namespace mod3 = "http://www.w3.org/TestModules/module3";


mod3:function2(1 cast as simple:myType)
