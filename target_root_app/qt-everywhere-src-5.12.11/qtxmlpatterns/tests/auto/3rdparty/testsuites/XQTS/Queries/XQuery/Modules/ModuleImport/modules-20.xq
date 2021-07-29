(:*******************************************************:)
(: Test: modules-20.xq                                   :)
(: Written By: Nicolae Brinza                            :)
(: Purpose: Load a library but do not import the schema  :)
(:*******************************************************:)

import module namespace mod3 = "http://www.w3.org/TestModules/module3";


mod3:function1()
