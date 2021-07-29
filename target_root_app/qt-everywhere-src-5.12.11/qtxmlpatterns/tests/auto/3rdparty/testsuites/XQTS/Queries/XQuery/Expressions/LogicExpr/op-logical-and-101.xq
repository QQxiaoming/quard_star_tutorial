(:*******************************************************:)
(: Test: op-logical-and-101.xq                           :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:50:40 2005                        :)
(: Purpose: Logical 'and' using long values              :)
(:*******************************************************:)

   <return>
     { xs:long(9223372036854775807) and xs:long(-1) }
   </return>
