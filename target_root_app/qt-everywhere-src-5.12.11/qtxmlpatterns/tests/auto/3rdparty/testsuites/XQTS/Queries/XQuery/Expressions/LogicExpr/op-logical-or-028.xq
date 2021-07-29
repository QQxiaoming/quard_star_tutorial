(:*******************************************************:)
(: Test: op-logical-or-028.xq                            :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:53:51 2005                        :)
(: Purpose: Logical 'or'  using untypedAtomic values     :)
(:*******************************************************:)

   <return>
     { xs:untypedAtomic('abc') or xs:untypedAtomic("0") }
   </return>
