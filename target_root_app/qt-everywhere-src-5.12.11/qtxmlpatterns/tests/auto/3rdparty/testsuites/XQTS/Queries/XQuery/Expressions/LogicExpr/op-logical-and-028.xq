(:*******************************************************:)
(: Test: op-logical-and-028.xq                           :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:50:40 2005                        :)
(: Purpose: Logical 'and' using untypedAtomic values     :)
(:*******************************************************:)

   <return>
     { xs:untypedAtomic('abc') and xs:untypedAtomic("0") }
   </return>
