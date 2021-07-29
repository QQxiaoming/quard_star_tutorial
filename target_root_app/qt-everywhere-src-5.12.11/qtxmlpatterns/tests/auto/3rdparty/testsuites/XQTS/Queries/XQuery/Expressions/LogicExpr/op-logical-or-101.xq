(:*******************************************************:)
(: Test: op-logical-or-101.xq                            :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:53:51 2005                        :)
(: Purpose: Logical 'or'  using long values              :)
(:*******************************************************:)

   <return>
     { xs:long(9223372036854775807) or xs:long(-1) }
   </return>
