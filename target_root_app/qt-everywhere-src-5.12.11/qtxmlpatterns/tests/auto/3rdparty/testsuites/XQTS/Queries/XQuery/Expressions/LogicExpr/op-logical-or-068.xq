(:*******************************************************:)
(: Test: op-logical-or-068.xq                            :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:53:51 2005                        :)
(: Purpose: Logical 'or'  using decimal values           :)
(:*******************************************************:)

   <return>
     { xs:decimal(9.99999999999999999999999999)
         or xs:decimal(-123456789.123456789123456789) }
   </return>
