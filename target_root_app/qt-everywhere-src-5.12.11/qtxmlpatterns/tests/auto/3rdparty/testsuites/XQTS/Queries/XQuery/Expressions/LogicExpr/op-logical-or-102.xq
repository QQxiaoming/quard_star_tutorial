(:*******************************************************:)
(: Test: op-logical-or-102.xq                            :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:53:51 2005                        :)
(: Purpose: Logical 'or'  using long values              :)
(:*******************************************************:)

   <return>
     { xs:long(-99999999999999999) or xs:long(0) }
   </return>
