(:*******************************************************:)
(: Test: modules-24.xq                                   :)
(: Written By: Nicolae Brinza                            :)
(: Purpose: Load a library but do not import the schema  :)
(:*******************************************************:)

import module namespace mod3 = "http://www.w3.org/TestModules/module3";


mod3:function4(<simple:integer>1</simple:integer>)
