(:*******************************************************:)
(: Test: op-logical-and-078.xq                           :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:50:40 2005                        :)
(: Purpose: Logical 'and' using nonPositiveInteger values:)
(:*******************************************************:)

   <return>
     { xs:nonPositiveInteger(-1) and xs:nonPositiveInteger(0) }
   </return>
