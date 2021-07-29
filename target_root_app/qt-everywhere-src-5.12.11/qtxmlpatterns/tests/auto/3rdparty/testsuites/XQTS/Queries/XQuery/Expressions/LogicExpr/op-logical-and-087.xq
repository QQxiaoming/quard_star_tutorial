(:*******************************************************:)
(: Test: op-logical-and-087.xq                           :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:50:40 2005                        :)
(: Purpose: Logical 'and' using nonNegativeInteger values:)
(:*******************************************************:)

   <return>
     { xs:nonNegativeInteger(9999999999)
         and xs:nonNegativeInteger(0) }
   </return>
