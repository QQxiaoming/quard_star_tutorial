(:*******************************************************:)
(: Test: op-logical-and-082.xq                           :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:50:40 2005                        :)
(: Purpose: Logical 'and' using nonPositiveInteger values:)
(:*******************************************************:)

   <return>
     { xs:nonPositiveInteger(-99999999999999999)
         and xs:nonPositiveInteger(-99999999999999999) }
   </return>
