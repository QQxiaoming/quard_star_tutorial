(:*******************************************************:)
(: Test: op-logical-or-077.xq                            :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:53:51 2005                        :)
(: Purpose: Logical 'or'  using nonPositiveInteger values:)
(:*******************************************************:)

   <return>
     { xs:nonPositiveInteger(0) or xs:nonPositiveInteger(-1) }
   </return>
