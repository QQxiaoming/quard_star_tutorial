(:*******************************************************:)
(: Test: op-logical-and-068.xq                           :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:50:40 2005                        :)
(: Purpose: Logical 'and' using decimal values           :)
(:*******************************************************:)

   <return>
     { xs:decimal(9.99999999999999999999999999)
         and xs:decimal(-123456789.123456789123456789) }
   </return>
