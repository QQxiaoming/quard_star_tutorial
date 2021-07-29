(:*******************************************************:)
(: Test: op-logical-or-109.xq                            :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:53:51 2005                        :)
(: Purpose: Logical 'or' using unsignedLong values       :)
(:*******************************************************:)

   <return>
     { xs:unsignedLong(9223372036854775807) or xs:unsignedLong(0) }
   </return>
