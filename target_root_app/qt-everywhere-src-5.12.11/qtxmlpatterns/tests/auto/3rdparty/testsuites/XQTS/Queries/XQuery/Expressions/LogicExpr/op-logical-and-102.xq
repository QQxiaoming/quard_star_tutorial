(:*******************************************************:)
(: Test: op-logical-and-102.xq                           :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:50:40 2005                        :)
(: Purpose: Logical 'and' using long values              :)
(:*******************************************************:)

   <return>
     { xs:long(-99999999999999999) and xs:long(0) }
   </return>
