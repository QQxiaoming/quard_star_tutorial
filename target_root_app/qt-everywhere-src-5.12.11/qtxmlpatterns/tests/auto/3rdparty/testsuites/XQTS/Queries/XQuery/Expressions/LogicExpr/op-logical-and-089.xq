(:*******************************************************:)
(: Test: op-logical-and-089.xq                           :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:50:40 2005                        :)
(: Purpose: Logical 'and' using nonNegativeInteger values:)
(:*******************************************************:)

   <return>
     { xs:nonNegativeInteger(99999999999999999)
         and xs:nonNegativeInteger(99999999999999999) }
   </return>
