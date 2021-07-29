(:*******************************************************:)
(: Test: op-logical-and-094.xq                           :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:50:40 2005                        :)
(: Purpose: Logical 'and' using positiveInteger values   :)
(:*******************************************************:)

   <return>
     { xs:positiveInteger(99999999999999999)
         and xs:positiveInteger(1) }
   </return>
