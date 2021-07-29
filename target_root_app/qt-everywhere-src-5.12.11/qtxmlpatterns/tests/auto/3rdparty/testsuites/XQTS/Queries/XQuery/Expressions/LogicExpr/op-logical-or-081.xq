(:*******************************************************:)
(: Test: op-logical-or-081.xq                            :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:53:51 2005                        :)
(: Purpose: Logical 'or'  using nonPositiveInteger values:)
(:*******************************************************:)

   <return>
     { xs:nonPositiveInteger(-1)
         or xs:nonPositiveInteger(-99999999999999999) }
   </return>
