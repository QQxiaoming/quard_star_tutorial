(:*******************************************************:)
(: Test: op-logical-and-110.xq                           :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:50:40 2005                        :)
(: Purpose: Logical 'and' using unsignedLong values      :)
(:*******************************************************:)

   <return>
     { xs:unsignedLong(9223372036854775807) and xs:unsignedLong(1) }
   </return>
