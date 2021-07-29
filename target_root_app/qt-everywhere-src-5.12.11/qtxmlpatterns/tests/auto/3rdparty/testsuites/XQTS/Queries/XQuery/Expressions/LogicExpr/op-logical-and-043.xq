(:*******************************************************:)
(: Test: op-logical-and-043.xq                           :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:50:40 2005                        :)
(: Purpose: Logical 'and' using float values             :)
(:*******************************************************:)

   <return>
     { xs:float('INF') and xs:float('NaN') }
   </return>
